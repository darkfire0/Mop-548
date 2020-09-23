/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Common.h"
#include "SharedPtrs.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Player.h"
#include "Unit.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "DynamicObject.h"
#include "ObjectAccessor.h"
#include "Util.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "Vehicle.h"

AuraApplication::AuraApplication(Unit* target, Unit* caster, AuraPtr aura, uint32 effMask):
_target(target), _base(aura), _removeMode(AURA_REMOVE_NONE), _slot(MAX_AURAS),
_flags(AFLAG_NONE), _effectsToApply(effMask), _needClientUpdate(false), _effectMask(0)
{
    ASSERT(GetTarget() && GetBase());

    if (GetBase()->CanBeSentToClient())
    {
        // Try find slot for aura
        uint8 slot = MAX_AURAS;
        // Lookup for auras already applied from spell
        if (AuraApplication * foundAura = GetTarget()->GetAuraApplication(GetBase()->GetId(), GetBase()->GetCasterGUID(), GetBase()->GetCastItemGUID()))
        {
            // allow use single slot only by auras from same caster
            slot = foundAura->GetSlot();
        }
        else
        {
            Unit::VisibleAuraMap const* visibleAuras = GetTarget()->GetVisibleAuras();
            // lookup for free slots in units visibleAuras
            Unit::VisibleAuraMap::const_iterator itr = visibleAuras->find(0);
            for (uint32 freeSlot = 0; freeSlot < MAX_AURAS; ++itr, ++freeSlot)
            {
                if (itr == visibleAuras->end() || itr->first != freeSlot)
                {
                    slot = freeSlot;
                    break;
                }
            }
        }

        // Register Visible Aura
        if (slot < MAX_AURAS)
        {
            _slot = slot;
            GetTarget()->SetVisibleAura(slot, this);
            SetNeedClientUpdate();
        }
    }

    _InitFlags(caster, effMask);
}

void AuraApplication::_Remove()
{
    uint8 slot = GetSlot();

    if (slot >= MAX_AURAS)
        return;

    if (AuraApplication * foundAura = _target->GetAuraApplication(GetBase()->GetId(), GetBase()->GetCasterGUID(), GetBase()->GetCastItemGUID()))
    {
        // Reuse visible aura slot by aura which is still applied - prevent storing dead pointers
        if (slot == foundAura->GetSlot())
        {
            if (GetTarget()->GetVisibleAura(slot) == this)
            {
                GetTarget()->SetVisibleAura(slot, foundAura);
                foundAura->SetNeedClientUpdate();
            }
            // set not valid slot for aura - prevent removing other visible aura
            slot = MAX_AURAS;
        }
    }

    // update for out of range group members
    if (slot < MAX_AURAS)
    {
        GetTarget()->RemoveVisibleAura(slot);
        ClientUpdate(true);
    }
}

void AuraApplication::_InitFlags(Unit* caster, uint32 effMask)
{
    // mark as selfcasted if needed
    _flags |= (GetBase()->GetCasterGUID() == GetTarget()->GetGUID()) ? AFLAG_CASTER : AFLAG_NONE;

    // aura is casted by self or an enemy
    // one negative effect and we know aura is negative
    if (IsSelfcasted() || !caster || !caster->IsFriendlyTo(GetTarget()))
    {
        bool negativeFound = false;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (((1<<i) & effMask) && !GetBase()->GetSpellInfo()->IsPositiveEffect(i))
            {
                negativeFound = true;
                break;
            }
        }
        _flags |= negativeFound ? AFLAG_NEGATIVE : AFLAG_POSITIVE;
    }
    // aura is casted by friend
    // one positive effect and we know aura is positive
    else
    {
        bool positiveFound = false;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (((1<<i) & effMask) && GetBase()->GetSpellInfo()->IsPositiveEffect(i))
            {
                positiveFound = true;
                break;
            }
        }
        _flags |= positiveFound ? AFLAG_POSITIVE : AFLAG_NEGATIVE;
    }

    if (GetBase()->GetSpellInfo()->AttributesEx8 & SPELL_ATTR8_AURA_SEND_AMOUNT)
        _flags |= AFLAG_ANY_EFFECT_AMOUNT_SENT;

    // there are more auras that require this flag, this is just the beginning
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (((1 << i) & effMask))
        {
            if (GetBase()->GetEffect(i) && GetBase()->GetEffect(i)->GetAmount())
            {
                _flags |= AFLAG_ANY_EFFECT_AMOUNT_SENT;
                break;
            }
        }
    }
}

void AuraApplication::_HandleEffect(uint8 effIndex, bool apply)
{
    AuraEffectPtr aurEff = GetBase()->GetEffect(effIndex);
    ASSERT(aurEff);
    ASSERT(HasEffect(effIndex) == (!apply));
    ASSERT((1<<effIndex) & _effectsToApply);

    if (apply)
    {
        ASSERT(!(_effectMask & (1<<effIndex)));
        _effectMask |= 1<<effIndex;
        aurEff->HandleEffect(this, AURA_EFFECT_HANDLE_REAL, true);
    }
    else
    {
        ASSERT(_effectMask & (1<<effIndex));
        _effectMask &= ~(1<<effIndex);
        aurEff->HandleEffect(this, AURA_EFFECT_HANDLE_REAL, false);

        // Remove all triggered by aura spells vs unlimited duration
        aurEff->CleanupTriggeredSpells(GetTarget());
    }
    SetNeedClientUpdate();
}

void AuraApplication::BuildBitsUpdatePacket(ByteBuffer& data, bool remove) const
{
    data.WriteBit(!remove);

    if (remove)
        return;

    constAuraPtr aura = GetBase();
    uint32 flags = _flags;
    if (aura->GetMaxDuration() > 0 && !(aura->GetSpellInfo()->AttributesEx5 & SPELL_ATTR5_HIDE_DURATION))
        flags |= AFLAG_DURATION;

    uint8 count = 0;
    if (flags & AFLAG_ANY_EFFECT_AMOUNT_SENT)
    {
        for (uint8 i  = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (aura->GetEffect(i))
                ++count;
        }
    }

    data.WriteBit(flags & AFLAG_DURATION); // duration
    data.WriteBits(0, 22); // second effect count
    data.WriteBit(flags & AFLAG_DURATION); // max duration
    data.WriteBit((flags & AFLAG_CASTER) == 0);

    data.WriteBits(count, 22);

    if (!(flags & AFLAG_CASTER))
    {
        ObjectGuid casterGuid = aura->GetCasterGUID();
        uint8 order[8] = {1, 6, 0, 7, 5, 3, 2, 4};

        data.WriteBitInOrder(casterGuid, order);
    }
}

void AuraApplication::BuildBytesUpdatePacket(ByteBuffer& data, bool remove, uint32 overrideSpell) const
{
    if (remove)
    {
        data << uint8(_slot);
        return;
    }

    constAuraPtr aura = GetBase();
    uint32 flags = _flags;
    if (aura->GetMaxDuration() > 0 && !(aura->GetSpellInfo()->AttributesEx5 & SPELL_ATTR5_HIDE_DURATION))
        flags |= AFLAG_DURATION;

    uint32 mask = 0;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (constAuraEffectPtr eff = aura->GetEffect(i)) // NULL if effect flag not set
        {
            mask |= 1 << i;
        }
    }

    if (!(flags & AFLAG_CASTER))
    {
        ObjectGuid casterGuid = aura->GetCasterGUID();
        uint8 order[8] = {2, 5, 6, 7, 0, 1, 4, 3};

        data.WriteBytesSeq(casterGuid, order);
    }
    
    data << uint32(mask);

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (constAuraEffectPtr eff = aura->GetEffect(i)) // NULL if effect flag not set
        {
            if (flags & AFLAG_ANY_EFFECT_AMOUNT_SENT)
                data << float(eff->GetAmount());
        }
    }

    data << uint8(flags);
    if (overrideSpell != 0)
        data << uint32(overrideSpell);
    else
        data << uint32(aura->GetId());

    if (!aura->GetSpellInfo()->HasAttribute(SPELL_ATTR11_CAST_WITH_ITEM))
        data << uint16(aura->GetCasterLevel());
    else
        data << int16(aura->GetCastItemLevel());

    data << uint8(aura->GetSpellInfo()->StackAmount ? aura->GetStackAmount() : aura->GetCharges());

    if (flags & AFLAG_DURATION)
        data << uint32(aura->GetMaxDuration());

    if (flags & AFLAG_DURATION)
        data << uint32(aura->GetDuration());

    // effect value 2 for

    data << uint8(_slot);
}

void AuraApplication::ClientUpdate(bool remove)
{
    _needClientUpdate = false;

    bool powerData = false;
    ObjectGuid targetGuid = GetTarget()->GetGUID();
    WorldPacket data(SMSG_AURA_UPDATE, 32);

    data.WriteBit(targetGuid[3]);
    data.WriteBit(false); // full update bit
    data.WriteBit(targetGuid[4]);
    data.WriteBit(targetGuid[5]);
    data.WriteBits(1, 24); // aura counter
    data.WriteBit(targetGuid[7]);
    data.WriteBit(targetGuid[6]);
    //data.WriteBit(powerData); // has power data, don't care about it ?

    if (powerData)
    {
        //packet.StartBitStream(guid2, 7, 0, 6);
        //powerCounter = packet.ReadBits(21);
        //packet.StartBitStream(guid2, 3, 1, 2, 4, 5);
    }

    BuildBitsUpdatePacket(data, remove);

    data.WriteBit(targetGuid[2]);
    data.WriteBit(targetGuid[0]);
    data.WriteBit(targetGuid[1]);
    data.FlushBits();
    
    BuildBytesUpdatePacket(data, remove);

	uint8 orderGuid[8] = { 0, 1, 3, 4, 2, 6, 7, 5 };
	data.WriteBytesSeq(targetGuid, orderGuid);

	_target->SendMessageToSet(&data, true);

    if (powerData)
    {
        //packet.ReadXORBytes(guid2, 7, 4, 5, 1, 6);

        //for (var i = 0; i < powerCounter; ++i)
        //{
            //packet.ReadInt32("Power Value", i);
            //packet.ReadEnum<PowerType>("Power Type", TypeCode.UInt32, i);
        //}

        //packet.ReadInt32("Attack power");
        //packet.ReadInt32("Spell power");
        //packet.ReadXORBytes(guid2, 3);
        //packet.ReadInt32("Current Health");
        //packet.ReadXORBytes(guid2, 0, 2);
        //packet.WriteGuid("PowerUnitGUID", guid2);
    }

	AuraPtr l_AuraBase = GetBase();
	if (!l_AuraBase || remove)
		return;

	if (l_AuraBase->GetSpellInfo()->Attributes & SPELL_ATTR0_HIDDEN_CLIENTSIDE)
		return;

	if (l_AuraBase->GetCaster() == _target)
		return;

	Mechanics l_Mechanic = Mechanics::MECHANIC_NONE;
	SpellEffIndex l_EffectIndex = SpellEffIndex::EFFECT_0;
	LossOfControlType l_Type = LossOfControlType::TypeNone;
	l_AuraBase->FillMechanicAndControlTypes(l_Mechanic, l_Type, l_EffectIndex);

	if (l_Type == LossOfControlType::TypeNone || l_Mechanic == Mechanics::MECHANIC_NONE)
		return;
	_target->SendLossOfControlAuraUpdate(this, l_Mechanic, l_EffectIndex, l_Type);
}

void AuraApplication::SendFakeAuraUpdate(uint32 auraId, bool remove)
{
    bool powerData = false;
    ObjectGuid targetGuid = GetTarget()->GetGUID();
    WorldPacket data(SMSG_AURA_UPDATE, 32);

    data.WriteBit(targetGuid[3]);
    data.WriteBit(false); // full update bit
    data.WriteBit(targetGuid[4]);
    data.WriteBit(targetGuid[5]);
    data.WriteBits(1, 24); // aura counter
    data.WriteBit(targetGuid[7]);
    data.WriteBit(targetGuid[6]);
    //data.WriteBit(powerData); // has power data, don't care about it ?

    if (powerData)
    {
        //packet.StartBitStream(guid2, 7, 0, 6);
        //powerCounter = packet.ReadBits(21);
        //packet.StartBitStream(guid2, 3, 1, 2, 4, 5);
    }

    BuildBitsUpdatePacket(data, remove);

    data.WriteBit(targetGuid[2]);
    data.WriteBit(targetGuid[0]);
    data.WriteBit(targetGuid[1]);
    data.FlushBits();

    BuildBytesUpdatePacket(data, remove, auraId);

    if (powerData)
    {
        //packet.ReadXORBytes(guid2, 7, 4, 5, 1, 6);

        //for (var i = 0; i < powerCounter; ++i)
        //{
            //packet.ReadInt32("Power Value", i);
            //packet.ReadEnum<PowerType>("Power Type", TypeCode.UInt32, i);
        //}

        //packet.ReadInt32("Attack power");
        //packet.ReadInt32("Spell power");
        //packet.ReadXORBytes(guid2, 3);
        //packet.ReadInt32("Current Health");

        //packet.ReadXORBytes(guid2, 0, 2);
        //packet.WriteGuid("PowerUnitGUID", guid2);
    }

    uint8 orderGuid[8] = {0, 1, 3, 4, 2, 6, 7, 5};
    data.WriteBytesSeq(targetGuid, orderGuid);

    _target->SendMessageToSet(&data, true);
}

uint32 Aura::BuildEffectMaskForOwner(SpellInfo const* spellProto, uint32 avalibleEffectMask, WorldObject* owner)
{
    ASSERT(spellProto);
    ASSERT(owner);
    uint32 effMask = 0;
    switch (owner->GetTypeId())
    {
        case TYPEID_UNIT:
        case TYPEID_PLAYER:
            for (uint8 i = 0; i< MAX_SPELL_EFFECTS; ++i)
            {
                if (spellProto->Effects[i].IsUnitOwnedAuraEffect())
                    effMask |= 1 << i;
            }
            break;
        case TYPEID_DYNAMICOBJECT:
            for (uint8 i = 0; i< MAX_SPELL_EFFECTS; ++i)
            {
                if (spellProto->Effects[i].Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA || spellProto->Effects[i].IsPeriodicEffect())
                    effMask |= 1 << i;
                else if (spellProto->Effects[i].Effect == SPELL_EFFECT_CREATE_AREATRIGGER)
                    effMask |= 1 << i;
            }
            break;
        default:
            break;
    }
    return effMask & avalibleEffectMask;
}

AuraPtr Aura::TryRefreshStackOrCreate(SpellInfo const* spellproto, uint32 tryEffMask, WorldObject* owner, Unit* caster, SpellPowerEntry const* spellPowerData, int32* baseAmount /*= NULL*/, Item* castItem /*= NULL*/, uint64 casterGUID /*= 0*/, bool* refresh /*= NULL*/, int32 castItemLevel /*= -1*/)
{
    ASSERT(spellproto);
    ASSERT(owner);
    ASSERT(caster || casterGUID);
    ASSERT(tryEffMask <= MAX_EFFECT_MASK);
    if (refresh)
        *refresh = false;
    uint32 effMask = Aura::BuildEffectMaskForOwner(spellproto, tryEffMask, owner);
    if (!effMask)
        return NULLAURA;

    AuraPtr foundAura = owner->ToUnit()->_TryStackingOrRefreshingExistingAura(spellproto, effMask, caster, baseAmount, castItem, casterGUID, castItemLevel);
    if (foundAura != NULLAURA)
    {
        // we've here aura, which script triggered removal after modding stack amount
        // check the state here, so we won't create new Aura object
        if (foundAura->IsRemoved())
            return NULLAURA;

        // Earthgrab Totem : Don't refresh root
        if (foundAura->GetId() == 64695)
            return NULLAURA;

        if (refresh)
            *refresh = true;
        return foundAura;
    }
    else
        return Create(spellproto, effMask, owner, caster, spellPowerData, baseAmount, castItem, casterGUID, castItemLevel);
}

AuraPtr Aura::TryCreate(SpellInfo const* spellproto, uint32 tryEffMask, WorldObject* owner, Unit* caster, SpellPowerEntry const* spellPowerData, int32* baseAmount /*= NULL*/, Item* castItem /*= NULL*/, uint64 casterGUID /*= 0*/, int32 castItemLevel /*= -1*/)
{
    ASSERT(spellproto);
    ASSERT(owner);
    ASSERT(caster || casterGUID);
    ASSERT(tryEffMask <= MAX_EFFECT_MASK);
    uint32 effMask = Aura::BuildEffectMaskForOwner(spellproto, tryEffMask, owner);
    if (!effMask)
        return NULLAURA;
    return Create(spellproto, effMask, owner, caster, spellPowerData, baseAmount, castItem, casterGUID, castItemLevel);
}

AuraPtr Aura::Create(SpellInfo const* spellproto, uint32 effMask, WorldObject* owner, Unit* caster, SpellPowerEntry const* spellPowerData, int32* baseAmount, Item* castItem, uint64 casterGUID, int32 castItemLevel)
{
    ASSERT(effMask);
    ASSERT(spellproto);
    ASSERT(owner);
    ASSERT(caster || casterGUID);
    ASSERT(effMask <= MAX_EFFECT_MASK);
    // try to get caster of aura
    if (casterGUID)
    {
        if (owner->GetGUID() == casterGUID)
            caster = owner->ToUnit();
        else
            caster = ObjectAccessor::GetUnit(*owner, casterGUID);
    }
    else
        casterGUID = caster->GetGUID();

    // check if aura can be owned by owner
    if (owner->isType(TYPEMASK_UNIT))
        if (!owner->IsInWorld() || ((Unit*)owner)->IsDuringRemoveFromWorld())
            // owner not in world so don't allow to own not self casted single target auras
            if (casterGUID != owner->GetGUID() && spellproto->IsSingleTarget())
                return NULLAURA;

    AuraPtr aura = NULLAURA;
    switch (owner->GetTypeId())
    {
        case TYPEID_UNIT:
        case TYPEID_PLAYER:
            aura = AuraPtr(new UnitAura(spellproto, effMask, owner, caster, spellPowerData, baseAmount, castItem, casterGUID, castItemLevel));
            aura->GetUnitOwner()->_AddAura(TO_UNITAURA(aura), caster);
            aura->LoadScripts();
            aura->_InitEffects(effMask, caster, baseAmount);
            break;
        case TYPEID_DYNAMICOBJECT:
            {
                aura = AuraPtr(new DynObjAura(spellproto, effMask, owner, caster, spellPowerData, baseAmount, castItem, casterGUID, castItemLevel));

                auto dynowner_temp = aura->GetDynobjOwner();

                ASSERT(dynowner_temp);
                ASSERT(dynowner_temp->IsInWorld());
                ASSERT(caster);
                ASSERT(caster->IsInWorld());
                ASSERT(dynowner_temp->GetMap() == caster->GetMap());

                aura->GetDynobjOwner()->SetAura(aura);
                aura->_InitEffects(effMask, caster, baseAmount);

                aura->LoadScripts();
                break;
            }
        default:
            ASSERT(false);
            return NULLAURA;
    }
    // aura can be removed in Unit::_AddAura call
    if (aura->IsRemoved())
        return NULLAURA;
    return aura;
}

Aura::Aura(SpellInfo const* spellproto, WorldObject* owner, Unit* caster, SpellPowerEntry const* spellPowerData, Item* castItem, uint64 casterGUID, int32 castItemLevel) :
m_spellInfo(spellproto), m_casterGuid(casterGUID ? casterGUID : caster->GetGUID()),
m_castItemGuid(castItem ? castItem->GetGUID() : 0), m_applyTime(time(NULL)),
m_owner(owner), m_timeCla(0), m_updateTargetMapInterval(0),
m_casterLevel(caster ? caster->getLevel() : m_spellInfo->SpellLevel), m_procCharges(0), m_stackAmount(1),
m_isRemoved(false), m_isSingleTarget(false), m_isUsingCharges(false), m_castItemLevel(castItemLevel)
{
    if (spellPowerData->manaPerSecond)
        m_timeCla = 1 * IN_MILLISECONDS;

    m_maxDuration = CalcMaxDuration(caster);
    m_duration = m_maxDuration;
    m_procCharges = CalcMaxCharges(caster);
    m_isUsingCharges = m_procCharges != 0;
    // m_casterLevel = cast item level/caster level, caster level should be saved to db, confirmed with sniffs
	if (m_spellInfo->Id == 80240)//No me queda de otra, que inicializar este valor del havoc aqui
	{
		Player* _caster = GetCaster()->ToPlayer();
		if (_caster){
			if (_caster->HasGlyph(146962)) // Glyph of havoc
			{
				m_stackAmount = 6;
			}
			else{
				m_stackAmount = 3;
			}
		}				
	}
		
}

void Aura::_InitEffects(uint32 effMask, Unit* caster, int32 *baseAmount)
{
    // shouldn't be in constructor - functions in AuraEffect::AuraEffect use polymorphism
	CallScriptOnEffectInit(effMask);
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (effMask & (uint8(1) << i))
        {
            m_effects[i] = AuraEffectPtr(new AuraEffect(shared_from_this(), i, baseAmount ? baseAmount + i : NULL, caster));

            m_effects[i]->CalculatePeriodic(caster, true, false);
            m_effects[i]->SetAmount(m_effects[i]->CalculateAmount(caster));
            m_effects[i]->CalculateSpellMod();
        }
        else
            m_effects[i] = NULLAURA_EFFECT;
    }
}

Aura::~Aura()
{
    // unload scripts
    while (!m_loadedScripts.empty())
    {
        std::list<AuraScript*>::iterator itr = m_loadedScripts.begin();
        (*itr)->_Unload();
        delete (*itr);
        m_loadedScripts.erase(itr);
    }

    ASSERT(m_applications.empty());
    _DeleteRemovedApplications();
}

void Aura::FillMechanicAndControlTypes(Mechanics& p_Mechanics, LossOfControlType& p_Type, SpellEffIndex& p_EffIndex)
{
    SpellInfo const* l_SpellInfo = GetSpellInfo();

    for (uint8 l_I = 0; l_I < MAX_EFFECTS; ++l_I)
    {
        switch (l_SpellInfo->Effects[l_I].Mechanic)
        {
            case Mechanics::MECHANIC_CHARM:
                if (l_SpellInfo->Effects[l_I].ApplyAuraName == SPELL_AURA_MOD_POSSESS)
                    p_Type = LossOfControlType::TypePossess;
                else
                    p_Type = LossOfControlType::TypeCharm;
                p_Mechanics = l_SpellInfo->Effects[l_I].Mechanic;
                p_EffIndex = SpellEffIndex(l_I);
                return;
            case Mechanics::MECHANIC_DISORIENTED:
                p_Type = LossOfControlType::TypeConfuse;
                p_Mechanics = l_SpellInfo->Effects[l_I].Mechanic;
                p_EffIndex = SpellEffIndex(l_I);
                return;
            case Mechanics::MECHANIC_DISARM:
                p_Type = LossOfControlType::TypeDisarm;
                p_Mechanics = l_SpellInfo->Effects[l_I].Mechanic;
                p_EffIndex = SpellEffIndex(l_I);
                return;
            case Mechanics::MECHANIC_FEAR:
                p_Type = LossOfControlType::TypeFear;
                p_Mechanics = l_SpellInfo->Effects[l_I].Mechanic;
                p_EffIndex = SpellEffIndex(l_I);
                return;
            case Mechanics::MECHANIC_ROOT:
                p_Type = LossOfControlType::TypeRoot;
                p_Mechanics = l_SpellInfo->Effects[l_I].Mechanic;
                p_EffIndex = SpellEffIndex(l_I);
                return;
            case Mechanics::MECHANIC_SILENCE:
            case Mechanics::MECHANIC_POLYMORPH:
            case Mechanics::MECHANIC_BANISH:
                if (l_SpellInfo->Effects[l_I].ApplyAuraName != SPELL_AURA_MOD_PACIFY_SILENCE)
                    p_Type = LossOfControlType::TypeSilence;
                else
                    p_Type = LossOfControlType::TypePacifySilence;
                p_Mechanics = l_SpellInfo->Effects[l_I].Mechanic;
                p_EffIndex = SpellEffIndex(l_I);
                return;
            case Mechanics::MECHANIC_SLEEP:
            case Mechanics::MECHANIC_STUN:
                p_Type = LossOfControlType::TypeStunMechanic;
                p_Mechanics = l_SpellInfo->Effects[l_I].Mechanic;
                p_EffIndex = SpellEffIndex(l_I);
                return;
            case Mechanics::MECHANIC_INTERRUPT:
                p_Type = LossOfControlType::TypeSchoolInterrupt;
                p_Mechanics = l_SpellInfo->Effects[l_I].Mechanic;
                p_EffIndex = SpellEffIndex(l_I);
                return;
            case Mechanics::MECHANIC_SAPPED:
            case Mechanics::MECHANIC_HORROR:
            case Mechanics::MECHANIC_FREEZE:
                p_Type = LossOfControlType::TypeStun;
                p_Mechanics = l_SpellInfo->Effects[l_I].Mechanic;
                p_EffIndex = SpellEffIndex(l_I);
                return;
            default:
                break;
        }
    }

    /// If nothing found, we must find default values depending on AuraType
    for (uint8 l_I = 0; l_I < MAX_EFFECTS; ++l_I)
    {
        switch (l_SpellInfo->Effects[l_I].ApplyAuraName)
        {
            case SPELL_AURA_MOD_ROOT:
                p_Type = LossOfControlType::TypeRoot;
                p_Mechanics = Mechanics::MECHANIC_ROOT;
                p_EffIndex = SpellEffIndex(l_I);
                return;
            case SPELL_AURA_MOD_FEAR:
            case SPELL_AURA_MOD_FEAR_2:
                p_Type = LossOfControlType::TypeFear;
                p_Mechanics = Mechanics::MECHANIC_FEAR;
                p_EffIndex = SpellEffIndex(l_I);
                return;
            case SPELL_AURA_MOD_STUN:
                p_Type = LossOfControlType::TypeStun;
                p_Mechanics = Mechanics::MECHANIC_STUN;
                p_EffIndex = SpellEffIndex(l_I);
                return;
            case SPELL_AURA_MOD_SILENCE:
                p_Type = LossOfControlType::TypeSilence;
                p_Mechanics = Mechanics::MECHANIC_SILENCE;
                p_EffIndex = SpellEffIndex(l_I);
                return;
            case SPELL_AURA_MOD_PACIFY_SILENCE:
                p_Type = LossOfControlType::TypePacifySilence;
                p_Mechanics = Mechanics::MECHANIC_POLYMORPH;
                p_EffIndex = SpellEffIndex(l_I);
                return;
            case SPELL_AURA_MOD_DISARM:
            case SPELL_AURA_MOD_DISARM_OFFHAND:
            case SPELL_AURA_MOD_DISARM_RANGED:
                p_Type = LossOfControlType::TypeDisarm;
                p_Mechanics = Mechanics::MECHANIC_DISARM;
                p_EffIndex = SpellEffIndex(l_I);
                return;
            case SPELL_AURA_MOD_POSSESS:
                p_Type = LossOfControlType::TypePossess;
                p_Mechanics = Mechanics::MECHANIC_CHARM;
                p_EffIndex = SpellEffIndex(l_I);
                return;
            case SPELL_AURA_MOD_CHARM:
                p_Type = LossOfControlType::TypeCharm;
                p_Mechanics = Mechanics::MECHANIC_CHARM;
                p_EffIndex = SpellEffIndex(l_I);
                return;
            case SPELL_AURA_MOD_CONFUSE:
                p_Type = LossOfControlType::TypeConfuse;
                p_Mechanics = Mechanics::MECHANIC_DISORIENTED;
                p_EffIndex = SpellEffIndex(l_I);
                return;
            default:
                break;
        }
    }
}

Unit* Aura::GetCaster() const
{
    if (!GetOwner())
        return NULL;

    if (GetOwner()->GetGUID() == GetCasterGUID())
        return GetUnitOwner();
    if (AuraApplication const* aurApp = GetApplicationOfTarget(GetCasterGUID()))
        return aurApp->GetTarget();

    return ObjectAccessor::GetUnit(*GetOwner(), GetCasterGUID());
}

AuraObjectType Aura::GetType() const
{
    return (m_owner && m_owner->GetTypeId() == TYPEID_DYNAMICOBJECT) ? DYNOBJ_AURA_TYPE : UNIT_AURA_TYPE;
}

void Aura::_ApplyForTarget(Unit* target, Unit* caster, AuraApplication * auraApp)
{
    m_applications[target->GetGUID()] = auraApp;

    // set infinity cooldown state for spells
    if (caster && caster->GetTypeId() == TYPEID_PLAYER)
    {
        if (m_spellInfo->Attributes & SPELL_ATTR0_DISABLED_WHILE_ACTIVE)
        {
            Item* castItem = m_castItemGuid ? caster->ToPlayer()->GetItemByGuid(m_castItemGuid) : NULL;
            caster->ToPlayer()->AddSpellAndCategoryCooldowns(m_spellInfo, castItem ? castItem->GetEntry() : 0, NULL, true);
        }
    }
}

void Aura::_UnapplyForTarget(Unit* target, Unit* caster, AuraApplication * auraApp)
{
    ASSERT(target);
    ASSERT(auraApp->GetRemoveMode());
    ASSERT(auraApp);

    ApplicationMap::iterator itr = m_applications.find(target->GetGUID());

    // TODO: Figure out why this happens
    if (itr == m_applications.end())
    {
        sLog->outError(LOG_FILTER_SPELLS_AURAS, "Aura::_UnapplyForTarget, target:%u, caster:%u, spell:%u was not found in owners application map!",
        target->GetGUIDLow(), caster ? caster->GetGUIDLow() : 0, auraApp->GetBase()->GetSpellInfo()->Id);
        ASSERT(false);
    }

    // aura has to be already applied
    ASSERT(itr->second == auraApp);
    m_applications.erase(itr);

    m_removedApplications.push_back(auraApp);

    // reset cooldown state for spells
    if (caster && caster->GetTypeId() == TYPEID_PLAYER)
    {
        if (GetSpellInfo()->Attributes & SPELL_ATTR0_DISABLED_WHILE_ACTIVE && !(GetSpellInfo()->Id == 34477 && caster->HasAura(56829) && (caster->GetPetGUID() == target->GetGUID())))
            // note: item based cooldowns and cooldown spell mods with charges ignored (unknown existed cases)
            caster->ToPlayer()->SendCooldownEvent(GetSpellInfo());
    }
}

// removes aura from all targets
// and marks aura as removed
void Aura::_Remove(AuraRemoveMode removeMode)
{
    ASSERT (!m_isRemoved);
    m_isRemoved = true;
    ApplicationMap::iterator appItr = m_applications.begin();
    for (appItr = m_applications.begin(); appItr != m_applications.end();)
    {
        AuraApplication * aurApp = appItr->second;
        Unit* target = aurApp->GetTarget();
        target->_UnapplyAura(aurApp, removeMode);
        appItr = m_applications.begin();
    }
}

void Aura::UpdateTargetMap(Unit* caster, bool apply)
{
    if (IsRemoved())
        return;

    m_updateTargetMapInterval = UPDATE_TARGET_MAP_INTERVAL;

    // fill up to date target list
    //       target, effMask
    std::map<Unit*, uint32> targets;

    FillTargetMap(targets, caster);

    UnitList targetsToRemove;

    // mark all auras as ready to remove
    for (ApplicationMap::iterator appIter = m_applications.begin(); appIter != m_applications.end();++appIter)
    {
        std::map<Unit*, uint32>::iterator existing = targets.find(appIter->second->GetTarget());
        // not found in current area - remove the aura
        if (existing == targets.end())
            targetsToRemove.push_back(appIter->second->GetTarget());
        else
        {
            // needs readding - remove now, will be applied in next update cycle
            // (dbcs do not have auras which apply on same type of targets but have different radius, so this is not really needed)
            if (appIter->second->GetEffectMask() != existing->second || !CanBeAppliedOn(existing->first))
                targetsToRemove.push_back(appIter->second->GetTarget());
            // nothing todo - aura already applied
            // remove from auras to register list
            targets.erase(existing);
        }
    }

    // register auras for units
    for (std::map<Unit*, uint32>::iterator itr = targets.begin(); itr!= targets.end();)
    {
        // aura mustn't be already applied on target
        if (AuraApplication * aurApp = GetApplicationOfTarget(itr->first->GetGUID()))
        {
            // the core created 2 different units with same guid
            // this is a major failue, which i can't fix right now
            // let's remove one unit from aura list
            // this may cause area aura "bouncing" between 2 units after each update
            // but because we know the reason of a crash we can remove the assertion for now
            if (aurApp->GetTarget() != itr->first)
            {
                // remove from auras to register list
                targets.erase(itr++);
                continue;
            }
            else
            {
                // ok, we have one unit twice in target map (impossible, but...)
                ASSERT(false);
            }
        }

        bool addUnit = true;
        // check target immunities
        for (uint8 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
        {
            if (itr->first->IsImmunedToSpellEffect(GetSpellInfo(), effIndex))
                itr->second &= ~(1 << effIndex);
        }
        if (!itr->second
            || itr->first->IsImmunedToSpell(GetSpellInfo())
            || !CanBeAppliedOn(itr->first))
            addUnit = false;

        if (addUnit)
        {
            // persistent area aura does not hit flying targets
            if (GetType() == DYNOBJ_AURA_TYPE)
            {
                if (itr->first->isInFlight())
                    addUnit = false;
            }
            // unit auras can not stack with each other
            else // (GetType() == UNIT_AURA_TYPE)
            {
                // Allow to remove by stack when aura is going to be applied on owner
                if (itr->first != GetOwner())
                {
                    // check if not stacking aura already on target
                    // this one prevents unwanted usefull buff loss because of stacking and prevents overriding auras periodicaly by 2 near area aura owners
                    for (Unit::AuraApplicationMap::iterator iter = itr->first->GetAppliedAuras().begin(); iter != itr->first->GetAppliedAuras().end(); ++iter)
                    {
                        constAuraPtr aura = iter->second->GetBase();
                        if (!CanStackWith(aura))
                        {
                            addUnit = false;
                            break;
                        }
                    }
                }
            }
        }
        if (!addUnit)
            targets.erase(itr++);
        else
        {
            // owner has to be in world, or effect has to be applied to self
            if (!GetOwner()->IsSelfOrInSameMap(itr->first))
            {
                //TODO: There is a crash caused by shadowfiend load addon
                sLog->outFatal(LOG_FILTER_SPELLS_AURAS, "Aura %u: Owner %s (map %u) is not in the same map as target %s (map %u).", GetSpellInfo()->Id,
                    GetOwner()->GetName(), GetOwner()->IsInWorld() ? GetOwner()->GetMap()->GetId() : uint32(-1),
                    itr->first->GetName(), itr->first->IsInWorld() ? itr->first->GetMap()->GetId() : uint32(-1));
                ASSERT(false);
            }
            itr->first->_CreateAuraApplication(shared_from_this(), itr->second);
            ++itr;
        }
    }

    // remove auras from units no longer needing them
    for (UnitList::iterator itr = targetsToRemove.begin(); itr != targetsToRemove.end();++itr)
        if (AuraApplication * aurApp = GetApplicationOfTarget((*itr)->GetGUID()))
            (*itr)->_UnapplyAura(aurApp, AURA_REMOVE_BY_DEFAULT);

    if (!apply)
        return;

    // apply aura effects for units
    for (std::map<Unit*, uint32>::iterator itr = targets.begin(); itr!= targets.end();++itr)
    {
        if (AuraApplication * aurApp = GetApplicationOfTarget(itr->first->GetGUID()))
        {
            // owner has to be in world, or effect has to be applied to self
            ASSERT((!GetOwner()->IsInWorld() && GetOwner() == itr->first) || GetOwner()->IsInMap(itr->first));
            itr->first->_ApplyAura(aurApp, itr->second);
        }
    }
}

// targets have to be registered and not have effect applied yet to use this function
void Aura::_ApplyEffectForTargets(uint8 effIndex)
{
    // prepare list of aura targets
    UnitList targetList;
    for (ApplicationMap::iterator appIter = m_applications.begin(); appIter != m_applications.end(); ++appIter)
    {
        if ((appIter->second->GetEffectsToApply() & (1<<effIndex)) && !appIter->second->HasEffect(effIndex))
            targetList.push_back(appIter->second->GetTarget());
    }

    // apply effect to targets
    for (UnitList::iterator itr = targetList.begin(); itr != targetList.end(); ++itr)
    {
        if (GetApplicationOfTarget((*itr)->GetGUID()))
        {
            // owner has to be in world, or effect has to be applied to self
            ASSERT((!GetOwner()->IsInWorld() && GetOwner() == *itr) || GetOwner()->IsInMap(*itr));
            (*itr)->_ApplyAuraEffect(shared_from_this(), effIndex);
        }
    }
}
void Aura::UpdateOwner(uint32 diff, WorldObject* owner)
{
    ASSERT(owner == m_owner);

    Unit* caster = GetCaster();
    // Apply spellmods for channeled auras
    // used for example when triggered spell of spell:10 is modded
    Spell* modSpell = NULL;
    Player* modOwner = NULL;
    if (caster)
    {
        modOwner = caster->GetSpellModOwner();
        if (modOwner)
        {
            modSpell = modOwner->FindCurrentSpellBySpellId(GetId());
            if (modSpell)
                modOwner->SetSpellModTakingSpell(modSpell, true);
        }
    }

    Update(diff, caster);

    if (m_updateTargetMapInterval <= int32(diff))
        UpdateTargetMap(caster);
    else
        m_updateTargetMapInterval -= diff;

    // update aura effects
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (m_effects[i])
            m_effects[i]->Update(diff, caster);

    // remove spellmods after effects update
    if (modSpell)
        modOwner->SetSpellModTakingSpell(modSpell, false);

    _DeleteRemovedApplications();
}

void Aura::Update(uint32 diff, Unit* caster)
{
    if (m_duration > 0)
    {
        m_duration -= diff;
        if (m_duration < 0)
            m_duration = 0;

        CallScriptAuraUpdateHandlers(diff);

        if (!caster &&  m_spellPowerData &&
           (m_spellPowerData->SpellId == 128432 || m_spellPowerData->SpellId == 90309 ||
            m_spellPowerData->SpellId == 24604 || m_spellPowerData->SpellId == 93435 ||
            m_spellPowerData->SpellId == 128433 || m_spellPowerData->SpellId == 126373 ||
            m_spellPowerData->SpellId == 128997 || m_spellPowerData->SpellId == 97229))
        {
            Remove();
            return;
        }

        // handle manaPerSecond/manaPerSecondPerLevel
        if (m_timeCla)
        {
            if (m_timeCla > int32(diff))
                m_timeCla -= diff;
            else if (caster)
            {
                if (!m_spellPowerData)
                    return;

                if (int32 manaPerSecond = m_spellPowerData->manaPerSecond)
                {
                    m_timeCla += 1000 - diff;

                    Powers powertype = Powers(m_spellPowerData->powerType);
                    if (powertype == POWER_HEALTH)
                    {
                        if (int32(caster->CountPctFromMaxHealth(manaPerSecond)) < caster->GetHealth())
                            caster->ModifyHealth(-1 * caster->CountPctFromMaxHealth(manaPerSecond));
                        else
                        {
                            Remove();
                            return;
                        }
                    }
                    else
                    {
                        if (int32(caster->CountPctFromMaxPower(manaPerSecond, powertype)) <= caster->GetPower(powertype))
                            caster->ModifyPower(powertype, -1 * int32(caster->CountPctFromMaxPower(manaPerSecond, powertype)));
                        else
                        {
                            Remove();
                            return;
                        }
                    }
                }
            }
        }
    }
}

int32 Aura::CalcMaxDuration(Unit* caster) const
{
    Player* modOwner = NULL;
    int32 maxDuration;

    if (caster)
    {
        modOwner = caster->GetSpellModOwner();
        maxDuration = caster->CalcSpellDuration(m_spellInfo);
    }
    else
        maxDuration = m_spellInfo->GetDuration();

    if (IsPassive() && !m_spellInfo->DurationEntry)
        maxDuration = -1;

    // Hack fix Duration for Tigereye Brew
    if (m_spellInfo->Id == 116740)
        maxDuration = 15000;
    if (m_spellInfo->Id == 108212)
        maxDuration = 4000;

    // IsPermanent() checks max duration (which we are supposed to calculate here)
    if (maxDuration != -1 && modOwner)
        modOwner->ApplySpellMod(GetId(), SPELLMOD_DURATION, maxDuration);
    return maxDuration;
}

void Aura::SetDuration(int32 duration, bool withMods)
{
    if (withMods)
    {
        if (Unit* caster = GetCaster())
            if (Player* modOwner = caster->GetSpellModOwner())
                modOwner->ApplySpellMod(GetId(), SPELLMOD_DURATION, duration);
    }

    m_duration = duration;
    SetNeedClientUpdateForTargets();
}

void Aura::SetAuraTimer(int32 newTime, uint64 guid)
{
    if (GetDuration() == -1 || GetDuration() > newTime)
    {
        SetDuration(newTime);
        SetMaxDuration(newTime);
        if (AuraApplication *timeAura = GetApplicationOfTarget(guid ? guid : m_casterGuid))
            timeAura->ClientUpdate();
    }
}

void Aura::RefreshDuration(bool recalculate)
{
    SetDuration(GetMaxDuration());

    if (m_spellPowerData->manaPerSecond)
        m_timeCla = 1 * IN_MILLISECONDS;
}

void Aura::RefreshTimers()
{
    m_maxDuration = CalcMaxDuration();
    bool resetPeriodic = true;
    if (m_spellInfo->AttributesCu & SPELL_ATTR0_CU_UPDATE_LAST_TICK)
    {
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (constAuraEffectPtr eff = GetEffect(i))
                if (int32 tick = eff->GetPeriodicTimer())
                {
                    m_maxDuration += tick;
                    break;
                }
    }

    if (m_spellInfo->AttributesCu & SPELL_ATTR0_CU_DONT_RESET_PERIODIC_TIMER)
        resetPeriodic = false;

    RefreshDuration();
    Unit* caster = GetCaster();
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (HasEffect(i))
            GetEffect(i)->CalculatePeriodic(caster, resetPeriodic, false);
}

void Aura::SetCharges(uint8 charges)
{
    if (m_procCharges == charges)
        return;

    m_procCharges = charges;
    m_isUsingCharges = m_procCharges != 0;

    SetNeedClientUpdateForTargets();
}

uint8 Aura::CalcMaxCharges(Unit* caster) const
{
    uint32 maxProcCharges = m_spellInfo->ProcCharges;
    if (SpellProcEntry const* procEntry = sSpellMgr->GetSpellProcEntry(GetId()))
        maxProcCharges = procEntry->charges;

    if (caster)
        if (Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(GetId(), SPELLMOD_CHARGES, maxProcCharges);
    return uint8(maxProcCharges);
}

bool Aura::ModCharges(int32 num, AuraRemoveMode removeMode)
{
    if (IsUsingCharges())
    {
        int32 charges = m_procCharges + num;
        int32 maxCharges = CalcMaxCharges();

        // Hack Fix - Arcane Missiles !
        if (GetId() == 79683)
            maxCharges = 2;

        // limit charges (only on charges increase, charges may be changed manually)
        if ((num > 0) && (charges > int32(maxCharges)))
            charges = maxCharges;
        // we're out of charges, remove
        else if (charges <= 0)
        {
            Remove(removeMode);
            return true;
        }

        SetCharges(charges);

        if (GetId() == 49222)
        {
            if (GetCaster()->HasAura(138197))
            {
                GetCaster()->CastSpell(GetCaster(), 138214, true);
            }
        }
        // Molten Core and Arcane Missiles ! : charges = stackAmount
        if (GetId() == 122355 || GetId() == 79683)
            SetStackAmount(charges);
    }
    return false;
}

void Aura::SetStackAmount(uint8 stackAmount)
{
    m_stackAmount = stackAmount;
    Unit* caster = GetCaster();

    std::list<AuraApplication*> applications;
    GetApplicationList(applications);

    for (std::list<AuraApplication*>::const_iterator apptItr = applications.begin(); apptItr != applications.end(); ++apptItr)
        if (!(*apptItr)->GetRemoveMode())
            HandleAuraSpecificMods(*apptItr, caster, false, true);

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (HasEffect(i))
            m_effects[i]->ChangeAmount(m_effects[i]->CalculateAmount(caster), false, true);

    for (std::list<AuraApplication*>::const_iterator apptItr = applications.begin(); apptItr != applications.end(); ++apptItr)
        if (!(*apptItr)->GetRemoveMode())
            HandleAuraSpecificMods(*apptItr, caster, true, true);

    SetNeedClientUpdateForTargets();
}

bool Aura::ModStackAmount(int32 num, AuraRemoveMode removeMode)
{	
    int32 stackAmount = m_stackAmount + num;

    // limit the stack amount (only on stack increase, stack amount may be changed manually)
    if ((num > 0) && (stackAmount > int32(m_spellInfo->StackAmount)))
    {
        // not stackable aura - set stack amount to 1
        if (!m_spellInfo->StackAmount)
            stackAmount = 1;
        else
            stackAmount = m_spellInfo->StackAmount;
    }
    // we're out of stacks, remove
    else if (stackAmount <= 0)
    {
        Remove(removeMode);
        return true;
    }

    bool refresh = stackAmount >= GetStackAmount();

    // Agony doesn't refresh itself every tick
    if (m_spellInfo->Id == 980)
        refresh = false;

    // Update stack amount
    SetStackAmount(stackAmount);

    if (refresh)
    {
        RefreshSpellMods();
        RefreshTimers();

        // Fix Backdraft can stack up to 6 charges max
        if (m_spellInfo->Id == 117828)
            SetCharges((GetCharges() + 3) > 6 ? 6 : GetCharges() + 3);
        // Molten Core - Just apply one charge by one charge
        else if (m_spellInfo->Id == 122355)
           SetCharges(GetCharges() + 1);
        // reset charges
        else
            SetCharges(CalcMaxCharges());

        // FIXME: not a best way to synchronize charges, but works
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (AuraEffectPtr aurEff = GetEffect(i))
                if (aurEff->GetAuraType() == SPELL_AURA_ADD_FLAT_MODIFIER || aurEff->GetAuraType() == SPELL_AURA_ADD_PCT_MODIFIER)
                    if (SpellModifier* mod = aurEff->GetSpellModifier())
                        mod->charges = GetCharges();
    }

    SetNeedClientUpdateForTargets();
    return false;
}

void Aura::RefreshSpellMods()
{
    for (Aura::ApplicationMap::const_iterator appIter = m_applications.begin(); appIter != m_applications.end(); ++appIter)
        if (Player* player = appIter->second->GetTarget()->ToPlayer())
            player->RestoreAllSpellMods(0, shared_from_this());
}

bool Aura::IsArea() const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (HasEffect(i) && GetSpellInfo()->Effects[i].IsAreaAuraEffect())
            return true;
    }
    return false;
}

bool Aura::IsPassive() const
{
    return GetSpellInfo()->IsPassive();
}

bool Aura::IsDeathPersistent() const
{
    return GetSpellInfo()->IsDeathPersistent();
}

bool Aura::CanBeSaved() const
{
    if (GetId() == 81744 || GetId() == 81748)
        return false;

    if (GetId() == 54637)
        return true;

    if (IsPassive())
        return false;

    if (GetCasterGUID() != GetOwner()->GetGUID())
        if (GetSpellInfo()->IsSingleTarget())
            return false;

    // Can't be saved - aura handler relies on calculated amount and changes it
    if (HasEffectType(SPELL_AURA_CONVERT_RUNE))
        return false;

    // No point in saving this, since the stable dialog can't be open on aura load anyway.
    if (HasEffectType(SPELL_AURA_OPEN_STABLE))
        return false;

    // Can't save vehicle auras, it requires both caster & target to be in world
    if (HasEffectType(SPELL_AURA_CONTROL_VEHICLE))
        return false;

    // Incanter's Absorbtion - considering the minimal duration and problems with aura stacking
    // we skip saving this aura
    // Also for some reason other auras put as MultiSlot crash core on keeping them after restart, 
    // so put here only these for which you are sure they get removed
    switch (GetId())
    {
        // Silithyst
        case 29519:
        // Incanter's Absorbtion - considering the minimal duration and problems with aura stacking
        // we skip saving this aura
        case 44413:
        case 40075: // Fel Flak Fire 
        case 55849: // Power Spark
        case 61669: // Aspect of the Beast
        // When a druid logins, he doesnt have either eclipse power, nor the marker auras, nor the eclipse buffs. Dont save them.
        case 48517:
        case 48518:
        case 67483:
        case 67484:
        // Pyromaniac
        case 83582:
        // Nature's Bounty
        case 96206:
        // Dark Flames
        case 99158:
        // Don't save special liquid auras
        case 101619: // Magma, Fall of Azeroth
        case 97151: // Magma, Firelands
        case 81114: // Magma, Blackwing Descent
        case 57634: // Magma, CoA Black / Chamber
        case 42201: // Water, Hyjal Past
        case 37025: // Water, Coilfang Raid
        case 36444: // Water, Lake Wintergrasp
        case 28801: // Slime, Naxxramas
            return false;
        default:
            break;
    }

    // don't save auras removed by proc system
    if (IsUsingCharges() && !GetCharges())
        return false;

    return true;
}

bool Aura::CanBeSentToClient() const
{
    if (GetId() == 115098)
        return false;

    if (!IsPassive())
        return true;

    if (GetSpellInfo()->HasAreaAuraEffect())
        return true;

    if (HasEffectType(SPELL_AURA_ABILITY_IGNORE_AURASTATE)      ||
        HasEffectType(SPELL_AURA_CAST_WHILE_WALKING)            ||
        HasEffectType(SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS)     ||
        HasEffectType(SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS_2)   ||
        HasEffectType(SPELL_AURA_MOD_IGNORE_SHAPESHIFT)         ||
        HasEffectType(SPELL_AURA_MOD_CHARGES))
        return true;

    return false;
}

void Aura::UnregisterSingleTarget()
{
    ASSERT(m_isSingleTarget);
    Unit* caster = GetCaster();

    // TODO: find a better way to do this.
    if (!caster)
        caster = ObjectAccessor::GetObjectInOrOutOfWorld(GetCasterGUID(), (Unit*)NULL);

    if (caster)
        caster->GetSingleCastAuras().remove(shared_from_this());

    SetIsSingleTarget(false);
}

int32 Aura::CalcDispelChance(Unit* auraTarget, bool offensive) const
{
    // we assume that aura dispel chance is 100% on start
    // need formula for level difference based chance
    int32 resistChance = 0;

    // Apply dispel mod from aura caster
    if (Unit* caster = GetCaster())
        if (Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(GetId(), SPELLMOD_RESIST_DISPEL_CHANCE, resistChance);

    // Dispel resistance from target SPELL_AURA_MOD_DISPEL_RESIST
    // Only affects offensive dispels
    if (offensive && auraTarget)
        resistChance += auraTarget->GetTotalAuraModifier(SPELL_AURA_MOD_DISPEL_RESIST);

    resistChance = resistChance < 0 ? 0 : resistChance;
    resistChance = resistChance > 100 ? 100 : resistChance;
    return 100 - resistChance;
}

void Aura::SetLoadedState(int32 maxduration, int32 duration, int32 charges, uint8 stackamount, uint32 recalculateMask, int32 * amount)
{
    m_maxDuration = maxduration;
    m_duration = duration;
    m_procCharges = charges;
    m_isUsingCharges = m_procCharges != 0;
    m_stackAmount = stackamount;
    Unit* caster = GetCaster();
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (m_effects[i])
        {
            m_effects[i]->SetAmount(amount[i]);
            m_effects[i]->SetCanBeRecalculated(recalculateMask & (1<<i));
            m_effects[i]->CalculatePeriodic(caster, false, true);
            m_effects[i]->CalculateSpellMod();
            m_effects[i]->RecalculateAmount(caster);
        }
}

bool Aura::HasEffectType(AuraType type) const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (HasEffect(i) && m_effects[i]->GetAuraType() == type)
            return true;
    }
    return false;
}

void Aura::RecalculateAmountOfEffects()
{
	if (IsRemoved())
		sLog->outError(LOG_FILTER_GENERAL, "Trying to recalculate amount of aura %u, but aura is removed", GetId());

    Unit* caster = GetCaster();
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (HasEffect(i))
            m_effects[i]->RecalculateAmount(caster);
}

void Aura::HandleAllEffects(AuraApplication * aurApp, uint8 mode, bool apply)
{
    ASSERT (!IsRemoved());
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (m_effects[i] && !IsRemoved())
            m_effects[i]->HandleEffect(aurApp, mode, apply);
}

void Aura::GetApplicationList(std::list<AuraApplication*> & applicationList) const
{
    for (Aura::ApplicationMap::const_iterator appIter = m_applications.begin(); appIter != m_applications.end(); ++appIter)
    {
        if (appIter->second->GetEffectMask())
            applicationList.push_back(appIter->second);
    }
}

void Aura::SetNeedClientUpdateForTargets() const
{
    for (ApplicationMap::const_iterator appIter = m_applications.begin(); appIter != m_applications.end(); ++appIter)
        appIter->second->SetNeedClientUpdate();
}

// trigger effects on real aura apply/remove
void Aura::HandleAuraSpecificMods(AuraApplication const* aurApp, Unit* caster, bool apply, bool onReapply)
{
    Unit* target = aurApp->GetTarget();
    AuraRemoveMode removeMode = aurApp->GetRemoveMode();
    // handle spell_area table
    SpellAreaForAreaMapBounds saBounds = sSpellMgr->GetSpellAreaForAuraMapBounds(GetId());
    if (saBounds.first != saBounds.second)
    {
        uint32 zone, area;
        target->GetZoneAndAreaId(zone, area);

        for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        {
            // some auras remove at aura remove
            if (!itr->second->IsFitToRequirements((Player*)target, zone, area))
                target->RemoveAurasDueToSpell(itr->second->spellId);
            // some auras applied at aura apply
            else if (itr->second->autocast)
            {
                if (!target->HasAura(itr->second->spellId))
                    target->CastSpell(target, itr->second->spellId, true);
            }
        }
    }

    // handle spell_linked_spell table
    if (!onReapply)
    {
        // apply linked auras
        if (apply)
        {
            if (std::vector<int32> const* spellTriggered = sSpellMgr->GetSpellLinked(GetId() + SPELL_LINK_AURA))
            {
                for (std::vector<int32>::const_iterator itr = spellTriggered->begin(); itr != spellTriggered->end(); ++itr)
                {
                    if (*itr < 0)
                        target->ApplySpellImmune(GetId(), IMMUNITY_ID, -(*itr), true);
                    else if (caster)
                        caster->AddAura(*itr, target);
                }
            }
        }
        else
        {
            // remove linked auras
            if (std::vector<int32> const* spellTriggered = sSpellMgr->GetSpellLinked(-(int32)GetId()))
            {
                for (std::vector<int32>::const_iterator itr = spellTriggered->begin(); itr != spellTriggered->end(); ++itr)
                {
                    if (*itr < 0)
                    {
                        if (!(aurApp->GetBase()->GetId() == 5143 && (-(*itr)) == 36032))
                            target->RemoveAurasDueToSpell(-(*itr));
                    }
                    else if (removeMode != AURA_REMOVE_BY_DEATH)
                        target->CastSpell(target, *itr, true, NULL, NULLAURA_EFFECT, GetCasterGUID());
                }
            }
            if (std::vector<int32> const* spellTriggered = sSpellMgr->GetSpellLinked(GetId() + SPELL_LINK_AURA))
            {
                for (std::vector<int32>::const_iterator itr = spellTriggered->begin(); itr != spellTriggered->end(); ++itr)
                {
                    if (*itr < 0)
                        target->ApplySpellImmune(GetId(), IMMUNITY_ID, -(*itr), false);
                    else
                        target->RemoveAura(*itr, GetCasterGUID(), 0, removeMode);
                }
            }
        }
    }
    else if (apply)
    {
        // modify stack amount of linked auras
        if (std::vector<int32> const* spellTriggered = sSpellMgr->GetSpellLinked(GetId() + SPELL_LINK_AURA))
        {
            for (std::vector<int32>::const_iterator itr = spellTriggered->begin(); itr != spellTriggered->end(); ++itr)
                if (*itr > 0)
                {
                    AuraPtr triggeredAura = target->GetAura(*itr, GetCasterGUID());
                    if (triggeredAura != NULLAURA)
                        triggeredAura->ModStackAmount(GetStackAmount() - triggeredAura->GetStackAmount());
                }
        }
    }    

    // Soul burn: Unending Breath
    if (GetId() == 5697 && target)
    {
        if (apply)
       {
            if (target->HasAura(74434))
            {
                target->RemoveAurasDueToSpell(74434);
                target->AddAura(104242,target);
            }
        }
        else
            target->RemoveAurasDueToSpell(104242);
    }

    // mods at aura apply
    if (apply)
    {
        switch (GetSpellInfo()->SpellFamilyName)
        {
            case SPELLFAMILY_POTION:
                if (GetId() == 6615) // Free Action
                    if (GetUnitOwner() && GetUnitOwner()->getLevel() >= 77)
                    {
                        int32 duration = (30 - ((GetUnitOwner()->getLevel() - 77) * 3)) * IN_MILLISECONDS;
                        SetDuration(duration);
                        SetMaxDuration(duration);
                    }
                break;
            case SPELLFAMILY_GENERIC:
                switch (GetId())
                {                    
                    case 114206:
                    {
                        if (target->HasAura(138126))
                            target->CastSpell(target, 138127, true);
                        break;
                    }
                    // Spark of Zandalar
                    case 138958:
                        if (target && target->GetTypeId() == TYPEID_PLAYER && target->HasAura(138958))
                        {
                            if (target->GetAura(138958)->GetStackAmount() == 10)
                            {
                                target->RemoveAurasDueToSpell(138958);
                                target->CastSpell(target, 138960, true, target->ToPlayer()->GetItemByGuid(GetCastItemGUID()));
                            }
                        }
                        break;
                    // PvP Trinket
                    case 42292:
                        if (target && target->GetTypeId() == TYPEID_PLAYER)
                            target->CastSpell(target, (target->ToPlayer()->GetTeam() == ALLIANCE ? 97403 : 97404), true);
                        break;
                    // Magma, Echo of Baine
                    case 101619:
                        if (target && target->GetTypeId() == TYPEID_PLAYER && !target->HasAura(101866))
                            target->CastSpell(target, 101866, true);
                        break;
                    // Burning Rage, Item - Warrior T12 DPS 2P Bonus
                    case 99233:
                    {
                        if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                            break;

                        uint32 max_duration = 12000;

                        if (constAuraEffectPtr aurEff = caster->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_WARRIOR, 47, EFFECT_1))
                            max_duration -= 6000 * (0.01f * aurEff->GetAmount());

                        SetDuration(max_duration);
                        break;
                    }
                    // Blazing Power, Alysrazor
                    case 99461:
                        if (AuraPtr aur = target->GetAura(98619))
                            aur->RefreshDuration();

                        if (!target->HasAura(100029) && aurApp->GetBase()->GetStackAmount() >= 25)
                            target->CastSpell(target, 100029, true);    

                        break;
                    // Arion - Swirling Winds
                    case 83500:
                        target->RemoveAurasDueToSpell(83581);
                        break;
                    // Terrastra - Grounded
                    case 83581:
                        target->RemoveAurasDueToSpell(83500);
                        break;
                    case 32474: // Buffeting Winds of Susurrus
                        if (target->GetTypeId() == TYPEID_PLAYER)
                            target->ToPlayer()->ActivateTaxiPathTo(506, GetId());
                        break;
                    case 33572: // Gronn Lord's Grasp, becomes stoned
                        if (GetStackAmount() >= 5 && !target->HasAura(33652))
                            target->CastSpell(target, 33652, true);
                        break;
                    case 50836: //Petrifying Grip, becomes stoned
                        if (GetStackAmount() >= 5 && !target->HasAura(50812))
                            target->CastSpell(target, 50812, true);
                        break;
                    case 60970: // Heroic Fury (remove Intercept cooldown)
                        if (target->GetTypeId() == TYPEID_PLAYER)
                            target->ToPlayer()->RemoveSpellCooldown(20252, true);
                        break;
                    case 105785:    // Stolen Time mage T13 set bonus
                    {
                        if (target->HasAura(105790))
                            target->CastSpell(target, 105791, true); // cooldown bonus
                        break;
                    }
                    // Smoke Bomb, Rogue
                    case 88611:
                        if (caster)
                            if (!target->IsFriendlyTo(caster))
                                target->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);
                        break;
                }
                break;
            case SPELLFAMILY_DRUID:
                if (!caster)
                    break;
                switch (GetId())
                {
                    case 5217:
                    {
                        if (target->HasAura(144841))
                            target->CastSpell(target, 146874, true);
                    }break;
                    case 16870:
                    {
                        if (target->HasAura(144864))
                            target->CastSpell(target, 144865, true);
                    }break;
                    case 48438:
                    {
                        if (caster->HasAura(144875))
                            caster->CastSpell(target, 144876, true);
                    }break;
                    case 16886:
                    {
                        if (caster->HasAura(138350))
                        {
                            caster->CastSpell(caster, 138351, true);
                        }
                    }break;
                }
                // Rejuvenation
                if (GetSpellInfo()->SpellFamilyFlags[0] & 0x10 && GetEffect(EFFECT_0))
                {
                    // Druid T8 Restoration 4P Bonus
                    if (caster->HasAura(64760))
                    {
                        int32 heal = GetEffect(EFFECT_0)->GetAmount();
                        caster->CastCustomSpell(target, 64801, &heal, NULL, NULL, true, NULL, GetEffect(EFFECT_0));
                    }
                }
                break;
            case SPELLFAMILY_MAGE:
            {
                if (!caster)
                    break;

                switch (GetId())
                {
                    case 10: // Blizzard
                        target->UpdateSpeed(MOVE_RUN, true);
                        break;
                    case 118:   // Polymorph
                    case 61305: // Polymorph (Black Cat)
                    {
                        // Polymorph Sound - Sheep && Penguin
                        // Glyph of the Penguin
                        if (caster->HasAura(52648))
                            caster->CastSpell(target, 61635, true);
                        // Glyph of the Monkey
                        else if (caster->HasAura(57927))
                            caster->CastSpell(target, 89729, true);
                        // Glyph of the Porcupine
                        else if (caster->HasAura(57924))
                            caster->CastSpell(target, 126834, true);
                        else
                            caster->CastSpell(target, 61634, true);

                        // Glyph of Polymorph
                        if (caster && caster->HasAura(56375))
                        {
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE, 0, target->GetAura(32409)); // Shawow Word : Death Self Damage not be removed.
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_LEECH);
                        }

                        break;
                    }
                    case 12536: // Clearcasting
                    case 12043: // Presence of Mind
                    {
                        // Arcane Potency
                        if (constAuraEffectPtr aurEff = caster->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_MAGE, 2120, 0))
                        {
                            uint32 spellId = 0;

                            switch (aurEff->GetId())
                            {
                                case 31571: spellId = 57529; break;
                                case 31572: spellId = 57531; break;
                                default:
                                    sLog->outError(LOG_FILTER_SPELLS_AURAS, "Aura::HandleAuraSpecificMods: Unknown rank of Arcane Potency (%d) found", aurEff->GetId());
                            }
                            if (spellId)
                                caster->CastSpell(caster, spellId, true);
                        }
                        break;
                    }
                    case 44457: // Living Bomb
                    {
                        UnitList targets;
                        WoWSource::AnyUnitHavingBuffInObjectRangeCheck u_check(caster, caster, 300.0f, 44457, false);
                        WoWSource::UnitListSearcher<WoWSource::AnyUnitHavingBuffInObjectRangeCheck> searcher(caster, targets, u_check);
                        caster->VisitNearbyObject(300.0f, searcher);
                        if (targets.size() >= 4)
                        {
                            std::list<AuraPtr> auras;
                            for (UnitList::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                                if (AuraPtr aur = (*itr)->GetAura(44457, caster->GetGUID()))
                                    auras.push_back(aur);

                            if (auras.size() >= 4)
                            {
                                auras.sort(WoWSource::DurationOrderPred(false));
                                auras.pop_front();
                                auras.pop_front();
                                auras.pop_front();
                                for (std::list<AuraPtr>::iterator itr = auras.begin(); itr != auras.end();)
                                {
                                    (*itr)->Remove();
                                    itr = auras.erase(itr);
                                }
                            }
                        }

                        break;
                    }

                    default: break;
                }

                break;
            }
            case SPELLFAMILY_PRIEST:
                if (!caster)
                    break;
                switch (GetId())
                {
                    case 63735:
                    {
                        if (target->HasAura(145306))
                            target->CastSpell(target, 145327, true);
                    }break;
                    case 109964:
                    {
                        if (caster->HasAura(145334))
                            caster->CastSpell(target, 145374, true);
                    }break;
					//Psyfiend - SPELL_PSYCHIC_HORROR    = 113792  2.5 sec immune
					case 113792:
						target->AddAura(91264, target);
						break;
                }
                // Devouring Plague
                if (GetSpellInfo()->SpellFamilyFlags[0] & 0x02000000 && GetEffect(0))
                {
                    // Improved Devouring Plague
                    if (constAuraEffectPtr aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 3790, 0))
                    {
                        uint32 damage = target->SpellDamageBonusTaken(caster, GetSpellInfo(), GetEffect(0)->GetAmount(), DOT);
                        int32 basepoints0 = aurEff->GetAmount() * GetEffect(0)->GetTotalTicks() * int32(damage) / 100;
                        int32 heal = int32(CalculatePct(basepoints0, 15));

                        caster->CastCustomSpell(target, 63675, &basepoints0, NULL, NULL, true, NULL, GetEffect(0));
                        caster->CastCustomSpell(caster, 75999, &heal, NULL, NULL, true, NULL, GetEffect(0));
                    }
                }
                // Power Word: Shield
                else if (m_spellInfo->Id == 17 || m_spellInfo->Id == 123258 && GetEffect(0))
                {
                    // Glyph of Power Word: Shield
                    if (AuraEffectPtr glyph = caster->GetAuraEffect(55672, 0))
                    {
                        // instantly heal m_amount% of the absorb-value
                        int32 heal = glyph->GetAmount() * GetEffect(0)->GetAmount()/100;
                        caster->CastCustomSpell(GetUnitOwner(), 56160, &heal, NULL, NULL, true, 0, GetEffect(0));
                    }
                }
                break;
            case SPELLFAMILY_PALADIN:
            {
                if (!caster)
                    break;

                switch (m_spellInfo->Id)
                {
                    // Grand Crusader
                    case 85416:
                        caster->ToPlayer()->RemoveSpellCooldown(31935, true);
                        break;
                    case 114637:
                        if (caster->HasAura(144566))
                        {
                            if (GetStackAmount() >= 3)
                                caster->CastSpell(caster, 144569, true);
                        }
                        break;
                    case 54149:
                        if (caster->HasAura(144625))
                            caster->CastSpell(caster, 144624, true);
                        break;
                    default:
                        break;
                }

                break;
            }
            case SPELLFAMILY_ROGUE:
            {
                // Sprint (skip non player casted spells by category)
                if (GetSpellInfo()->SpellFamilyFlags[0] & 0x40 && GetSpellInfo()->Category == 44)
                {
                    // in official maybe there is only one icon?
                    if (target->HasAura(58039)) // Glyph of Blurred Speed
                        target->CastSpell(target, 61922, true); // Sprint (waterwalk)
                }

                switch (GetId())
                {
                    case 121471:
                        if (target->HasAura(138150))
                            target->CastSpell(target, 138151, true);
                    break;
                    // Blind
                    case 2094:
                    {
                        // Glyph of Blind
                        if (caster && caster->HasAura(91299))
                        {
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE, 0, target->GetAura(32409)); // Shawow Word : Death Self Damage not be removed.
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_LEECH);
                        }
                        break;
                    }
                    // Sap
                    case 6770:
                    {
                        // Remove stealth from target
                        target->RemoveAurasByType(SPELL_AURA_MOD_STEALTH, 0, 0, 11327);
                        break;
                    }
                    case 108212:// Burst of Speed
                    {
                        target->RemoveAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case SPELLFAMILY_SHAMAN:
                // Maelstorm Weapon
                if (GetId() == 53817)
                {
                    // Item - Shaman T13 Enhancement 2P Bonus
                    if (target->HasAura(105866))
                        target->CastSpell(target, 105869, true);
                }
                // Spiritwalker's Grace
                else if (GetId() == 79206)
                {
                    // Item - Shaman T13 Restoration 4P Bonus (Spiritwalker's Grace)
                    if (target->HasAura(105876))
                        target->CastSpell(target, 105877, true);
                    if (target->HasAura(131557))
                        target->CastSpell(target, 131558, true);
                }
                
                break;
            case SPELLFAMILY_WARLOCK:               
                switch (GetId())
                {
                case 113858:
                case  113860:
                case  113861:
                    if (caster && target)
                        if (caster == target && caster->HasAura(138129))
                            caster->CastSpell(caster, 138483, true);
                    break;
                }
                break;
            case SPELLFAMILY_WARRIOR:
                // Heroic Fury
                if (m_spellInfo->Id == 60970)
                {
                    if (target->GetTypeId() == TYPEID_PLAYER)
                        target->ToPlayer()->RemoveSpellCooldown(20252, true);
                }
                // Battle Shout && Commander Shout
                else if (m_spellInfo->Id == 469 || m_spellInfo->Id == 6673)
                {
                    // Item - Warrior T12 DPS 2P Bonus
                    if (caster)
                        if (caster == target && caster->HasAura(99234))
                            caster->CastSpell(caster, 99233, true);
                }
                break;
            case SPELLFAMILY_HUNTER:
                if (!caster)
                    break;

                switch(GetId())
                {
                    case 55041: // Freezing Trap Effect
                    case 19503: // Scatter Shot
                    case 3355:  // Freezing Trap
                    {
                        // Freezing Trap removes stealth
                        if (m_spellInfo->Id == 55041)
                            target->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);

                        // Glyph of Solace
                        if (caster->HasAura(119407))
                        {
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE, 0, target->GetAura(32409)); // Shawow Word : Death Self Damage not be removed.
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
                            target->RemoveAurasByType(SPELL_AURA_PERIODIC_LEECH);
                        }
                        break;
                    }
                }
                break;
            case SPELLFAMILY_DEATHKNIGHT:
            {
                switch (GetId())
                {
                    // Vampiric Blood
                    case 55233:
                        // Item - Death Knight T13 Blood 4P Bonus
                        if (caster->HasAura(105587))
                            caster->CastSpell((Unit*)NULL, 105588, true);
                        break;
                    case 81162: // Will of the Necropolis
                    {
                        if (caster)
                            caster->CastSpell(caster, 96171, true);

                        break;
                    }
                    default:
                        break;
                }
                if (GetSpellInfo()->GetSpellSpecific() == SPELL_SPECIFIC_PRESENCE)
                {
                    if (!caster)
                        return;

                    int runicPowerSwitch = 0;
                    if (caster->HasAura(58647)) // You retain 70% of your Runic Power when switching Presences.
                        runicPowerSwitch = int(caster->GetPower(POWER_RUNIC_POWER) * 0.7f);

                    caster->SetPower(POWER_RUNIC_POWER, runicPowerSwitch);
                }

                break;
            }
            default:
                break;
        }
    }
    // mods at aura remove
    else
    {
        switch (GetSpellInfo()->SpellFamilyName)
        {
            case SPELLFAMILY_GENERIC:
                switch (GetId())
                {
                    case 69674: // Mutated Infection (Rotface)
                    case 71224: // Mutated Infection (Rotface)
                    case 73022: // Mutated Infection (Rotface)
                    case 73023: // Mutated Infection (Rotface)
                    {
                        AuraRemoveMode removeMode = aurApp->GetRemoveMode();
                        if (removeMode == AURA_REMOVE_BY_EXPIRE || removeMode == AURA_REMOVE_BY_ENEMY_SPELL)
                            target->CastSpell(target, 69706, true);
                        break;
                    }
                    // Torment, Baleroc
                    case 99256:
                    case 100230:
                    case 100231:
                    case 100232:
                        if (removeMode == AURA_REMOVE_BY_EXPIRE)
                            target->CastSpell(target, 99257, true);
                        break;
                    // Vital Flame, Baleroc
                    case 99263:
                    {//if (InstanceScript* pInstance = target->GetInstanceScript())
                            //if (pInstance->GetBossState(4) == IN_PROGRESS)
                                uint8 stacks = GetEffect(EFFECT_0)->GetAmount() / 5;
                                target->CastCustomSpell(99262, SPELLVALUE_AURA_STACK, stacks? stacks: 1, target, true);
                        break;
                    }
                    // Finkle's Mixture, Chimaeron
                    case 82705:
                        target->RemoveAurasDueToSpell(89084);                            
                        break;
                    case 69483: // Dark Reckoning (ICC)
                        if (caster)
                            caster->CastSpell(target, 69482, false);
                        break;
                    case 91296: // Egg Shell, Corrupted Egg Shell
                        if (caster)
                            caster->CastSpell(caster, 91305, true);
                        break;
                    case 91308: // Egg Shell, Corrupted Egg Shell (H)
                        if (caster)
                            caster->CastSpell(caster, 91310, true);
                        break;
                    case 37896: // To Infinity And Above
                        target->CastSpell(target, 55001, true);
                        break;
                    case 49440: // Racer Slam, Slamming
                        if (Creature* racerBunny = target->FindNearestCreature(27674, 25.0f))
                            target->CastSpell(racerBunny, 49302, false);
                        break;
                    case 52418: // Carrying Seaforium
                        if (removeMode == AURA_REMOVE_BY_CANCEL)
                            target->CastSpell(target, 52417, true);
                        break;
                    case 72368: // Shared Suffering
                    case 72369:
                        if (caster)
                        {
                            if (AuraEffectPtr aurEff = GetEffect(0))
                            {
                                int32 remainingDamage = aurEff->GetAmount() * (aurEff->GetTotalTicks() - aurEff->GetTickNumber());
                                if (remainingDamage > 0)
                                    caster->CastCustomSpell(caster, 72373, NULL, &remainingDamage, NULL, true);
                            }
                        }
                        break;
                }
                break;
            case SPELLFAMILY_MAGE:
            {
                switch (GetId())
                {
                case 110909:
                    if (caster->HasAura(138316))
                    {
                        caster->CastSpell(caster, 138317, true);
                    }
                    break;
                    // Blizzard
                    case 10:
                        target->UpdateSpeed(MOVE_RUN, true);
                        break;
                    case 66: // Invisibility
                    {
                        // Glyph of Invisibility
                        if (caster)
                            caster->RemoveAurasDueToSpell(87833);

                        if (removeMode != AURA_REMOVE_BY_EXPIRE)
                            break;

                        // Mage water elemental gains invisibility as mage
                        if (target->GetTypeId() == TYPEID_PLAYER)
                            if (Pet* pet = target->ToPlayer()->GetPet())
                                pet->CastSpell(pet, 32612, true, NULL, GetEffect(1));

                        target->CastSpell(target, 32612, true, NULL, GetEffect(1));
                        target->CombatStop();

                        // Glyph of Invisibility
                        if (target->HasAura(56366))
                            target->CastSpell(target, 87833, true);

                        break;
                    }
                    case 32612: // Invisibility (triggered)
                    {
                        if (target->GetTypeId() == TYPEID_PLAYER)
                            if (Pet* pet = target->ToPlayer()->GetPet())
                                if (pet->HasAura(32612))
                                    pet->RemoveAurasDueToSpell(32612);

                        break;
                    }
                    // Ring of Frost - 2.5 sec immune
                    case 82691:
                        target->AddAura(91264, target);
                        break;
                    default:
                        break;
                }

                break;
            }
            case SPELLFAMILY_WARLOCK:
                if (!caster)
                    break;

                // Glyph of Verdant Spheres
                if (GetSpellInfo()->Id == SPELL_GLYPH_VERDANT_SPHERES)
                {
                    caster->RemoveAurasDueToSpell(SPELL_VERDANT_SPHERE_DESTRO);
                    caster->RemoveAurasDueToSpell(SPELL_VERDANT_SPHERE_AFFL);
                    caster->RemoveAurasDueToSpell(SPELL_VERDANT_SPHERE_AFFL_2);
                }

                // Soul burn: health funnel
                if (GetSpellInfo()->Id == 755)
                    caster->RemoveAurasDueToSpell(74434);

                // Improved Fear
                if (GetSpellInfo()->SpellFamilyFlags[1] & 0x00000400)
                {
                    if (AuraEffectPtr aurEff = caster->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_WARLOCK, 98, 0))
                    {
                        uint32 spellId = 0;
                        switch (aurEff->GetId())
                        {
                            case 53759: spellId = 60947; break;
                            case 53754: spellId = 60946; break;
                            default:
                                sLog->outError(LOG_FILTER_SPELLS_AURAS, "Aura::HandleAuraSpecificMods: Unknown rank of Improved Fear (%d) found", aurEff->GetId());
                        }
                        if (spellId)
                            caster->CastSpell(target, spellId, true);
                    }
                }
                break;
            case SPELLFAMILY_PRIEST:
                if (!caster)
                    break;
                break;
            case SPELLFAMILY_ROGUE:
                // Remove Vanish on stealth remove
                if (GetId() == 1784 || GetId() == 115191)
                    target->RemoveAurasDueToSpell(131369, target->GetGUID());
                break;
            case SPELLFAMILY_DEATHKNIGHT:
            {
                switch (GetSpellInfo()->Id)
                {
                    case 56835: // Reaping
                    {
                        if (!GetEffect(0) || GetEffect(0)->GetAuraType() != SPELL_AURA_PERIODIC_DUMMY)
                            break;
                        if (target->GetTypeId() != TYPEID_PLAYER)
                            break;
                        if (target->ToPlayer()->getClass() != CLASS_DEATH_KNIGHT)
                            break;

                        // aura removed - remove death runes
                        target->ToPlayer()->RemoveRunesBySpell(GetId());
                        break;
                    }
                    case 81256: // Dancing Rune Weapon
                    {
                        // Item - Death Knight T12 Tank 4P Bonus
                        if (target->HasAura(98966) && (removeMode == AURA_REMOVE_BY_EXPIRE))
                            target->CastSpell(target, 101162, true); // +15% parry
                        break;
                    }
                    default:
                        break;
                }

                break;
            }
            case SPELLFAMILY_HUNTER:
                // Glyph of Freezing Trap
                if (GetSpellInfo()->SpellFamilyFlags[0] & 0x00000008)
                    if (caster && caster->HasAura(56845))
                        target->CastSpell(target, 61394, true);
                break;
            case SPELLFAMILY_SHAMAN:
            {
                switch (GetSpellInfo()->Id)
                {
                    // Grownding Totem effect
                    case 89523:
                    case 8178:
                        //if (caster != target && removeMode != AURA_REMOVE_NONE)
                            //caster->setDeathState(JUST_DIED); El server esta crashando aki revisar luego
                        break;
                    default:
                        break;
                }
                break;
            }
            case SPELLFAMILY_DRUID:
            {
                switch (GetSpellInfo()->Id)
                {
                    case 22812: // Barkskin
                    {
                        // Item - Druid T12 Feral 4P Bonus
                        if (caster && caster->HasAura(99009) && (removeMode == AURA_REMOVE_BY_EXPIRE))
                            caster->CastSpell(caster, 99011, true);

                        if (caster&& caster->HasAura(144879))
                        {
                            caster->CastSpell(caster, 62606, true);
                            caster->CastSpell(caster, 22842, true);
                        }
                        break;
                    }
                    default:
                        break;
                }

                break;
            }
            case SPELLFAMILY_PALADIN:
            {
                if (!caster)
                    break;

                switch (GetSpellInfo()->Id)
                {
                    case 498:
                    {
                        if (!caster || !caster->HasAura(144580))
                            break;
                        
                        if (auto aura_ptr = caster->GetAuraEffect(144580, EFFECT_0))
                        {
                            int32 aura_amount = aura_ptr->GetAmount();
                            if (aura_amount)
                            {
                                caster->CastCustomSpell(caster, 144581, &aura_amount, NULL, NULL, true);
                                aura_ptr->SetAmount(0);
                            }
                        }
                        break;
                    }

                    default:
                        break;
                }

                // Avenging Wrath
                if (m_spellInfo->Id == 31884)
                    target->RemoveAura(57318);
                // Communion
                else if (m_spellInfo->SpellFamilyFlags[2] & 0x20 && target == caster)
                    caster->RemoveAurasDueToSpell(63531);
                // Divine Protection
                else if (m_spellInfo->Id == 498)
                    // Item - Paladin T12 Protection 4P Bonus
                    if (caster->HasAura(99091) && (removeMode == AURA_REMOVE_BY_EXPIRE))
                        caster->CastSpell(caster, 99090, true);

                break;
            }
            case SPELLFAMILY_WARRIOR:
                switch (GetId())
                {
                    case 125565:
                    {
                        if (target->HasAura(144502))
                            target->CastSpell(target, 144500, true);
                    }break;
                }

                // Shield Block
                if (GetId() == 2565)
                    // Item - Item - Warrior T12 Protection 4P Bonus
                    if (caster && caster->HasAura(99242) && (removeMode == AURA_REMOVE_BY_EXPIRE))
                        caster->CastSpell(caster, 99243, true);
                break;
            default:
                break;
        }
    }

    // mods at aura apply or remove
    switch (GetSpellInfo()->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
            switch (GetId())
            {
                case 7215:  // Mithril Spurs
                    if (apply)
                    {
                        if (target->getLevel() <= 70)
                            target->CastSpell(target, 59916, true);
                    }
                    else
                        target->RemoveAurasDueToSpell(59916);
                    break;
                case 50720: // Vigilance
                    if (apply)
                        target->CastSpell(caster, 59665, true, 0, NULLAURA_EFFECT, caster->GetGUID());
                    else
                        target->SetReducedThreatPercent(0, 0);
                    break;
                case 71289: // Mind Control (Lady Deathwisper)
                    target->ApplyPercentModFloatValue(OBJECT_FIELD_SCALE_X, 100.0f, apply);
                    break;
            }
            break;
        case SPELLFAMILY_DRUID:
        {
            switch (GetSpellInfo()->Id)
            {
                // Tree of Life
                case 33891:
                {
                    if (apply)
                    {
                        caster->CastSpell(caster, 5420, true);
                        caster->CastSpell(caster, 81097, true);
                        caster->CastSpell(caster, 81098, true);
                        if (!caster->HasAura(117679))
							caster->CastSpell(caster, 117679, true);
                    }
                    else
                    {
                        caster->RemoveAurasDueToSpell(5420);
                        caster->RemoveAurasDueToSpell(81097);
                        caster->RemoveAurasDueToSpell(81098);
                    }
                    break;
                }
                case 5229:  // Enrage
                    if (target->HasAura(70726)) // Item - Druid T10 Feral 4P Bonus
                        if (apply)
                            target->CastSpell(target, 70725, true);
                    break;
                case 22812: // Glyph of Barkskin
                {
                    if (apply)
                    {
                        if (caster->HasAura(63057))
                            caster->AddAura(63058, caster);
                    }
                    else
                        caster->RemoveAura(63058);
                    break;
                }
                default:
                    break;
            }

            break;
        }
        case SPELLFAMILY_ROGUE:
            // Stealth / Vanish
            if (GetSpellInfo()->SpellFamilyFlags[0] & 0x00400800)
            {
                // Master of subtlety
                if (constAuraEffectPtr aurEff = target->GetAuraEffectOfRankedSpell(31223, 0))
                {
                    if (apply)
                    {
                        int32 basepoints0 = aurEff->GetAmount();
                        target->CastCustomSpell(target, 31665, &basepoints0, NULL, NULL , true);
                    }
                    else if (!target->GetAuraEffect(SPELL_AURA_MOD_SHAPESHIFT, SPELLFAMILY_ROGUE, 0x400800, 0, 0))
                        if (AuraPtr aur=target->GetAura(31665))
                            aur->SetAuraTimer(6000);
                }
                // Overkill
                if (target->HasAura(58426))
                {
                    if (apply)
                    {
                        target->CastSpell(target,58427,true);
                    }
                    else if (!target->GetAuraEffect(SPELL_AURA_MOD_SHAPESHIFT, SPELLFAMILY_ROGUE, 0x400800, 0, 0))
                        if (AuraPtr aur=target->GetAura(58427))
                            aur->SetAuraTimer(20000);
                }
                break;
            }
            break;
        case SPELLFAMILY_HUNTER:
            switch (GetId())
            {
                case 19574: // Bestial Wrath
                    // The Beast Within cast on owner if talent present
                    if (Unit* owner = target->GetOwner())
                    {
                        // Search talent
                        if (owner->HasAura(34692))
                        {
                            if (apply)
                                owner->CastSpell(owner, 34471, true, 0, GetEffect(0));
                            else
                                owner->RemoveAurasDueToSpell(34471);
                        }
                    }
                    break;
            }
            break;
        case SPELLFAMILY_PALADIN:
        {
            switch (GetId())
            {
                case 31821:
                    // Aura Mastery Triggered Spell Handler
                    // If apply Concentration Aura -> trigger -> apply Aura Mastery Immunity
                    // If remove Concentration Aura -> trigger -> remove Aura Mastery Immunity
                    // If remove Aura Mastery -> trigger -> remove Aura Mastery Immunity
                    // Do effects only on aura owner
                    if (GetCasterGUID() != target->GetGUID())
                        break;

                    if (apply)
                    {
                        if ((GetSpellInfo()->Id == 31821 && target->HasAura(19746, GetCasterGUID())) || (GetSpellInfo()->Id == 19746 && target->HasAura(31821)))
                            target->CastSpell(target, 64364, true);
                    }
                    else
                    {
                        target->RemoveAurasDueToSpell(64364, GetCasterGUID());
                        target->RemoveOwnedAura(64364, GetCasterGUID());
                    }
                    break;
                case 31842: // Divine Favor
                    // Item - Paladin T10 Holy 2P Bonus
                    if (target->HasAura(70755))
                    {
                        if (apply)
                            target->CastSpell(target, 71166, true);
                        else
                            target->RemoveAurasDueToSpell(71166);
                    }
                    break;
            }
            break;
        }
        case SPELLFAMILY_MAGE:
        {
            switch (GetId())
            {
                case 66:    // Invisibility
                    if (apply && caster && caster->HasAura(56366))
                        caster->CastSpell(caster, 87833, true);
                    break;
                case 32612: // Invisibility (triggered)
                    if (!apply && caster)
                        caster->RemoveAurasDueToSpell(87833);
                    break;
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_PRIEST:
            if (!caster)
                break;

            switch(GetSpellInfo()->Id)
            {
                // Devouring Plague
                case 2944:
                {
                    // Item - Priest T12 Shadow 4P Bonus
                    if (caster->HasAura(99157))
                    {
                        if (target->HasAura(589) && target->HasAura(2944) && target->HasAura(34914))
                            caster->CastSpell(caster, 99158, true);
                        else
                            caster->RemoveAurasDueToSpell(99158);
                    }
                    break;
                }
            }
        default:
            break;
    }
}

bool Aura::CanBeAppliedOn(Unit* target)
{
    // unit not in world or during remove from world
    if (!target->IsInWorld() || target->IsDuringRemoveFromWorld())
    {
        // area auras mustn't be applied
        if (GetOwner() != target)
            return false;
        // not selfcasted single target auras mustn't be applied
        if (GetCasterGUID() != GetOwner()->GetGUID() && GetSpellInfo()->IsSingleTarget())
            return false;
        return true;
    }
    else
        return CheckAreaTarget(target);
}

bool Aura::CheckAreaTarget(Unit* target)
{
    return CallScriptCheckAreaTargetHandlers(target);
}

bool Aura::CanStackWith(constAuraPtr existingAura) const
{
    // Can stack with self
    if (this == existingAura.get())
        return true;

    // Dynobj auras always stack - Same for Swiftmend
    if (existingAura->GetType() == DYNOBJ_AURA_TYPE || existingAura->GetId() == 81262)
        return true;

    SpellInfo const* existingSpellInfo = existingAura->GetSpellInfo();
    bool sameCaster = GetCasterGUID() == existingAura->GetCasterGUID();

    // passive auras don't stack with another rank of the spell cast by same caster
    if (IsPassive() && sameCaster && m_spellInfo->IsDifferentRankOf(existingSpellInfo))
        return false;

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        // prevent remove triggering aura by triggered aura
        if (existingSpellInfo->Effects[i].TriggerSpell == GetId()
            // prevent remove triggered aura by triggering aura refresh
            || m_spellInfo->Effects[i].TriggerSpell == existingAura->GetId())
            return true;
    }

    // check spell specific stack rules
    if (m_spellInfo->IsAuraExclusiveBySpecificWith(existingSpellInfo)
        || (sameCaster && m_spellInfo->IsAuraExclusiveBySpecificPerCasterWith(existingSpellInfo)))
        return false;

    // check spell group stack rules
    SpellGroupStackRule stackRule = sSpellMgr->CheckSpellGroupStackRules(m_spellInfo, existingSpellInfo);
    if (stackRule)
    {
        if (stackRule == SPELL_GROUP_STACK_RULE_EXCLUSIVE)
            return false;
        if (sameCaster && stackRule == SPELL_GROUP_STACK_RULE_EXCLUSIVE_FROM_SAME_CASTER)
            return false;
    }

    bool casterIsPet = false;
    if (GetCaster() && existingAura->GetCaster())
    {
        if (GetCaster()->isPet() || existingAura->GetCaster()->isPet())
            casterIsPet = true;
    }

    if (!sameCaster)
    {
        if (m_spellInfo->AttributesEx3 & SPELL_ATTR3_STACK_FOR_DIFF_CASTERS)
            return true;

        // check same periodic auras
        for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            // area auras should not stack (shaman totem)
            if (m_spellInfo->Effects[i].Effect != SPELL_EFFECT_APPLY_AURA && m_spellInfo->Effects[i].Effect != SPELL_EFFECT_APPLY_AURA_ON_PET
                && m_spellInfo->Effects[i].Effect != SPELL_EFFECT_PERSISTENT_AREA_AURA)
                continue;

            // not channeled AOE effects should not stack (blizzard should, but Consecration should not)
            if (m_spellInfo->Effects[i].IsTargetingArea() && !m_spellInfo->IsChanneled())
                continue;
                
            switch (m_spellInfo->Effects[i].ApplyAuraName)
            {
                // DOT or HOT from different casters will stack
                case SPELL_AURA_PERIODIC_DAMAGE:
                case SPELL_AURA_PERIODIC_DUMMY:
                case SPELL_AURA_PERIODIC_HEAL:
                case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                case SPELL_AURA_PERIODIC_ENERGIZE:
                case SPELL_AURA_PERIODIC_MANA_LEECH:
                case SPELL_AURA_PERIODIC_LEECH:
                case SPELL_AURA_POWER_BURN:
                case SPELL_AURA_OBS_MOD_POWER:
                case SPELL_AURA_OBS_MOD_HEALTH:
                case SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE:
                    // periodic auras which target areas are not allowed to stack this way (replenishment for example)
                    if (m_spellInfo->Effects[i].IsTargetingArea() || existingSpellInfo->Effects[i].IsTargetingArea())
                        break;
                    // Curse of Elements
                    if (m_spellInfo->Id == 1490)
                        break;
                    return true;
                case SPELL_AURA_MOD_DAMAGE_FROM_CASTER:                // Vendetta-like auras
                case SPELL_AURA_BYPASS_ARMOR_FOR_CASTER:               // Find Weakness-like auras
                case SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS:    // Hunter's Mark-like auras
                    return true;
                default:
                    break;
            }
        }
    }

    // negative and positive spells
    if (m_spellInfo->IsPositive() && !existingSpellInfo->IsPositive() ||
        !m_spellInfo->IsPositive() && existingSpellInfo->IsPositive())
        return true;

    // same spell
    if (m_spellInfo->Id == existingSpellInfo->Id)
    {
        // Hack for Incanter's Absorption
        if (m_spellInfo->Id == 44413)
            return true;

        // Bandit's Guile
        if (m_spellInfo->Id == 84748)
            return true;

        // Revealing Strike
        if (m_spellInfo->Id == 84617)
            return true;

        // Sha Residue (Immerseus)
        if (m_spellInfo->Id == 143459)
            return true;

        // Bubble Shield (Tormented Initiate)
        if (m_spellInfo->Id == 147333)
            return true;

        if (GetCastItemGUID() && existingAura->GetCastItemGUID())
            if (GetCastItemGUID() != existingAura->GetCastItemGUID() && m_spellInfo->HasCustomAttribute(SPELL_ATTR0_CU_ENCHANT_STACK))
                return true;

        // same spell with same caster should not stack
        if (!m_spellInfo->HasAura(SPELL_AURA_CONTROL_VEHICLE))
            return false;
    }

    SpellSpecificType specificTypes[] = {SPELL_SPECIFIC_ASPECT, SPELL_SPECIFIC_WELL_FED};
    for (uint8 i = 0; i < 2; ++i)
    {
        if (m_spellInfo->GetSpellSpecific() == specificTypes[i] || existingSpellInfo->GetSpellSpecific() == specificTypes[i])
            return true;
    }

    if (m_spellInfo->SpellIconID == 0 || existingSpellInfo->SpellIconID == 0)
    {
        bool isModifier = false;
        for (int i = 0; i < 3; ++i)
        {
            if (m_spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_ADD_FLAT_MODIFIER ||
                m_spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_ADD_PCT_MODIFIER  ||
                existingSpellInfo->Effects[i].ApplyAuraName == SPELL_AURA_ADD_FLAT_MODIFIER ||
                existingSpellInfo->Effects[i].ApplyAuraName == SPELL_AURA_ADD_PCT_MODIFIER)
                isModifier = true;
        }
        if (isModifier == true)
            return true;
    }

    if (m_spellInfo->SpellIconID != existingSpellInfo->SpellIconID &&
        ((m_spellInfo->GetMaxDuration() <= 60*IN_MILLISECONDS && m_spellInfo->GetMaxDuration() != -1) || 
        (existingSpellInfo->GetMaxDuration() <= 60*IN_MILLISECONDS && existingSpellInfo->GetMaxDuration() != -1)))
        return true;

    if (m_spellInfo->IsAllwaysStackModifers() && !existingSpellInfo->IsAllwaysStackModifers())
    {
        for (int i = 0; i < 3; ++i)
        {
            if ((m_spellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AURA || m_spellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AREA_AURA_RAID) &&
                (existingSpellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AURA || existingSpellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AREA_AURA_RAID) &&
                (m_spellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AURA_ON_PET || m_spellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AREA_AURA_RAID) &&
                (existingSpellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AURA_ON_PET || existingSpellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AREA_AURA_RAID) &&
                m_spellInfo->Effects[i].ApplyAuraName == existingSpellInfo->Effects[i].ApplyAuraName)
            {
                switch (m_spellInfo->Effects[i].ApplyAuraName)
                {
                    case SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE:
                    case SPELL_AURA_MOD_STAT:
                        if (m_spellInfo->Effects[i].MiscValue == existingSpellInfo->Effects[i].MiscValue || (m_spellInfo->Effects[i].MiscValueB != 0 && 
                            m_spellInfo->Effects[i].MiscValueB == existingSpellInfo->Effects[i].MiscValueB))
                            return false;
                        break;
                    case SPELL_AURA_MOD_RATING:
                        if (m_spellInfo->SpellIconID == existingSpellInfo->SpellIconID && m_spellInfo->Effects[i].MiscValue == existingSpellInfo->Effects[i].MiscValue)
                            return false;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    if (m_spellInfo->SpellFamilyName != existingSpellInfo->SpellFamilyName)
        return true;
        
    bool isVehicleAura1 = false;
    bool isVehicleAura2 = false;
    uint8 i = 0;
    while (i < MAX_SPELL_EFFECTS && !(isVehicleAura1 && isVehicleAura2))
    {
        if (m_spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_CONTROL_VEHICLE)
            isVehicleAura1 = true;
        if (existingSpellInfo->Effects[i].ApplyAuraName == SPELL_AURA_CONTROL_VEHICLE)
            isVehicleAura2 = true;

        ++i;
    }

    if (isVehicleAura1 && isVehicleAura2)
    {
        Vehicle* veh = NULL;
        if (GetOwner()->ToUnit())
            veh = GetOwner()->ToUnit()->GetVehicleKit();

        if (!veh)           // We should probably just let it stack. Vehicle system will prevent undefined behaviour later
            return true;

        if (!veh->GetAvailableSeatCount())
            return false;   // No empty seat available

        return true;        // Empty seat available (skip rest)
    }

    // spell of same spell rank chain
    if (m_spellInfo->IsRankOf(existingSpellInfo))
    {
        // don't allow passive area auras to stack
        if (m_spellInfo->IsMultiSlotAura() && !IsArea())
            return true;
        if (GetCastItemGUID() && existingAura->GetCastItemGUID())
            if (GetCastItemGUID() != existingAura->GetCastItemGUID() && (m_spellInfo->AttributesCu & SPELL_ATTR0_CU_ENCHANT_STACK))
                return true;
        // same spell with same caster should not stack
        return false;
    }

    return true;
}

bool Aura::IsProcOnCooldown() const
{
    /*if (m_procCooldown)
    {
        if (m_procCooldown > time(NULL))
            return true;
    }*/
    return false;
}

void Aura::AddProcCooldown(uint32 /*msec*/)
{
    //m_procCooldown = time(NULL) + msec;
}

void Aura::PrepareProcToTrigger(AuraApplication* aurApp, ProcEventInfo& eventInfo)
{
    bool prepare = CallScriptPrepareProcHandlers(aurApp, eventInfo);
    if (!prepare)
        return;

    // take one charge, aura expiration will be handled in Aura::TriggerProcOnEvent (if needed)
    if (IsUsingCharges())
    {
        --m_procCharges;
        SetNeedClientUpdateForTargets();
    }

    SpellProcEntry const* procEntry = sSpellMgr->GetSpellProcEntry(GetId());

    ASSERT(procEntry);

    // cooldowns should be added to the whole aura (see 51698 area aura)
    AddProcCooldown(procEntry->cooldown);
}

bool Aura::IsProcTriggeredOnEvent(AuraApplication* aurApp, ProcEventInfo& eventInfo) const
{
    SpellProcEntry const* procEntry = sSpellMgr->GetSpellProcEntry(GetId());
    // only auras with spell proc entry can trigger proc
    if (!procEntry)
        return false;

    // check if we have charges to proc with
    if (IsUsingCharges() && !GetCharges())
        return false;

    // check proc cooldown
    if (IsProcOnCooldown())
        return false;

    // TODO:
    // something about triggered spells triggering, and add extra attack effect

    // do checks against db data
    if (!sSpellMgr->CanSpellTriggerProcOnEvent(*procEntry, eventInfo))
        return false;

    // do checks using conditions table
    ConditionList conditions = sConditionMgr->GetConditionsForNotGroupedEntry(CONDITION_SOURCE_TYPE_SPELL_PROC, GetId());
    ConditionSourceInfo condInfo = ConditionSourceInfo(eventInfo.GetActor(), eventInfo.GetActionTarget());
    if (!sConditionMgr->IsObjectMeetToConditions(condInfo, conditions))
        return false;

    // AuraScript Hook
    bool check = const_cast<Aura*>(this)->CallScriptCheckProcHandlers(aurApp, eventInfo);
    if (!check)
        return false;

    // TODO:
    // do allow additional requirements for procs
    // this is needed because this is the last moment in which you can prevent aura charge drop on proc
    // and possibly a way to prevent default checks (if there're going to be any)

    // Check if current equipment meets aura requirements
    // do that only for passive spells
    // TODO: this needs to be unified for all kinds of auras
    Unit* target = aurApp->GetTarget();
    if (IsPassive() && target->GetTypeId() == TYPEID_PLAYER)
    {
        if (GetSpellInfo()->EquippedItemClass == ITEM_CLASS_WEAPON)
        {
            if (target->ToPlayer()->IsInFeralForm())
                return false;

            if (eventInfo.GetDamageInfo())
            {
                WeaponAttackType attType = eventInfo.GetDamageInfo()->GetAttackType();
                Item* item = NULL;
                if (attType == BASE_ATTACK)
                    item = target->ToPlayer()->GetUseableItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                else if (attType == OFF_ATTACK)
                    item = target->ToPlayer()->GetUseableItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

                if (!item || item->IsBroken() || item->GetTemplate()->Class != ITEM_CLASS_WEAPON || !((1<<item->GetTemplate()->SubClass) & GetSpellInfo()->EquippedItemSubClassMask))
                    return false;
            }
        }
        else if (GetSpellInfo()->EquippedItemClass == ITEM_CLASS_ARMOR)
        {
            // Check if player is wearing shield
            Item* item = target->ToPlayer()->GetUseableItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
            if (!item || item->IsBroken() || item->GetTemplate()->Class != ITEM_CLASS_ARMOR || !((1<<item->GetTemplate()->SubClass) & GetSpellInfo()->EquippedItemSubClassMask))
                return false;
        }
    }

    return roll_chance_f(CalcProcChance(*procEntry, eventInfo));
}

float Aura::CalcProcChance(SpellProcEntry const& procEntry, ProcEventInfo& eventInfo) const
{
    float chance = procEntry.chance;
    // calculate chances depending on unit with caster's data
    // so talents modifying chances and judgements will have properly calculated proc chance
    if (Unit* caster = GetCaster())
    {
        // calculate ppm chance if present and we're using weapon
        if (eventInfo.GetDamageInfo() && procEntry.ratePerMinute != 0)
        {
            uint32 WeaponSpeed = caster->GetAttackTime(eventInfo.GetDamageInfo()->GetAttackType());
            chance = caster->GetPPMProcChance(WeaponSpeed, procEntry.ratePerMinute, GetSpellInfo());
        }
        // apply chance modifer aura, applies also to ppm chance (see improved judgement of light spell)
        if (Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(GetId(), SPELLMOD_CHANCE_OF_SUCCESS, chance);
    }
    return chance;
}

void Aura::TriggerProcOnEvent(AuraApplication* aurApp, ProcEventInfo& eventInfo)
{
    CallScriptProcHandlers(const_cast<AuraApplication const*>(aurApp), eventInfo);

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (aurApp->HasEffect(i))
            // OnEffectProc / AfterEffectProc hooks handled in AuraEffect::HandleProc()
            GetEffect(i)->HandleProc(aurApp, eventInfo);

    CallScriptAfterProcHandlers(const_cast<AuraApplication const*>(aurApp), eventInfo);

    // Remove aura if we've used last charge to proc
    if (IsUsingCharges() && !GetCharges())
        Remove();
}

void Aura::_DeleteRemovedApplications()
{
    while (!m_removedApplications.empty())
    {
        delete m_removedApplications.front();
        m_removedApplications.pop_front();
    }
}

void Aura::LoadScripts()
{
    sScriptMgr->CreateAuraScripts(m_spellInfo->Id, m_loadedScripts);
    for (std::list<AuraScript*>::iterator itr = m_loadedScripts.begin(); itr != m_loadedScripts.end();)
    {
        if (!(*itr)->_Load(shared_from_this()))
        {
            std::list<AuraScript*>::iterator bitr = itr;
            ++itr;
            delete (*bitr);
            m_loadedScripts.erase(bitr);
            continue;
        }
        sLog->outDebug(LOG_FILTER_SPELLS_AURAS, "Aura::LoadScripts: Script `%s` for aura `%u` is loaded now", (*itr)->_GetScriptName()->c_str(), m_spellInfo->Id);
        (*itr)->Register();
        ++itr;
    }
}

bool Aura::CallScriptCheckAreaTargetHandlers(Unit* target)
{
    uint32 scriptExecuteTime = getMSTime();
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_CHECK_AREA_TARGET);
        std::list<AuraScript::CheckAreaTargetHandler>::iterator hookItrEnd = (*scritr)->DoCheckAreaTarget.end(), hookItr = (*scritr)->DoCheckAreaTarget.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            if (!(*hookItr).Call(*scritr, target))
                return false;
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);

    return true;
}

void Aura::CallScriptOnEffectInit(uint32 &effectMask)
{
	uint32 scriptExecuteTime = getMSTime();
	
	for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
	{
		(*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_ON_INIT);
		std::list<AuraScript::InitEffectsHandler>::iterator hookItrEnd = (*scritr)->OnInitEffects.end(), hookItr = (*scritr)->OnInitEffects.begin();
		for (; hookItr != hookItrEnd; ++hookItr)
		{			
			(*hookItr).Call(*scritr, effectMask);
		}
		
		(*scritr)->_FinishScriptCall();
	}

	scriptExecuteTime = getMSTime() - scriptExecuteTime;
	if (scriptExecuteTime > 15)
		sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

void Aura::CallScriptDispel(DispelInfo* dispelInfo)
{
    uint32 scriptExecuteTime = getMSTime();

    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_DISPEL);
        std::list<AuraScript::AuraDispelHandler>::iterator hookItrEnd = (*scritr)->OnDispel.end(), hookItr = (*scritr)->OnDispel.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(*scritr, dispelInfo);
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

void Aura::CallScriptAfterDispel(DispelInfo* dispelInfo)
{
    uint32 scriptExecuteTime = getMSTime();
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_AFTER_DISPEL);
        std::list<AuraScript::AuraDispelHandler>::iterator hookItrEnd = (*scritr)->AfterDispel.end(), hookItr = (*scritr)->AfterDispel.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(*scritr, dispelInfo);
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

bool Aura::CallScriptEffectApplyHandlers(constAuraEffectPtr aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode)
{
    uint32 scriptExecuteTime = getMSTime();

    bool preventDefault = false;
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_APPLY, aurApp);
        std::list<AuraScript::EffectApplyHandler>::iterator effEndItr = (*scritr)->OnEffectApply.end(), effItr = (*scritr)->OnEffectApply.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(*scritr, aurEff, mode);
        }
        if (!preventDefault)
            preventDefault = (*scritr)->_IsDefaultActionPrevented();
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);

    return preventDefault;
}

bool Aura::CallScriptEffectRemoveHandlers(constAuraEffectPtr aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode)
{
    uint32 scriptExecuteTime = getMSTime();
    bool preventDefault = false;
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_REMOVE, aurApp);
        std::list<AuraScript::EffectApplyHandler>::iterator effEndItr = (*scritr)->OnEffectRemove.end(), effItr = (*scritr)->OnEffectRemove.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(*scritr, aurEff, mode);
        }
        if (!preventDefault)
            preventDefault = (*scritr)->_IsDefaultActionPrevented();
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);

    return preventDefault;
}

void Aura::CallScriptAfterEffectApplyHandlers(constAuraEffectPtr aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode)
{
    uint32 scriptExecuteTime = getMSTime();
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_AFTER_APPLY, aurApp);
        std::list<AuraScript::EffectApplyHandler>::iterator effEndItr = (*scritr)->AfterEffectApply.end(), effItr = (*scritr)->AfterEffectApply.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(*scritr, aurEff, mode);
        }
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

void Aura::CallScriptAfterEffectRemoveHandlers(constAuraEffectPtr aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode)
{
    uint32 scriptExecuteTime = getMSTime();
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_AFTER_REMOVE, aurApp);
        std::list<AuraScript::EffectApplyHandler>::iterator effEndItr = (*scritr)->AfterEffectRemove.end(), effItr = (*scritr)->AfterEffectRemove.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(*scritr, aurEff, mode);
        }
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

bool Aura::CallScriptEffectPeriodicHandlers(constAuraEffectPtr aurEff, AuraApplication const* aurApp)
{
    uint32 scriptExecuteTime = getMSTime();
    bool preventDefault = false;
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_PERIODIC, aurApp);
        std::list<AuraScript::EffectPeriodicHandler>::iterator effEndItr = (*scritr)->OnEffectPeriodic.end(), effItr = (*scritr)->OnEffectPeriodic.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(*scritr, aurEff);
        }
        if (!preventDefault)
            preventDefault = (*scritr)->_IsDefaultActionPrevented();
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);

    return preventDefault;
}

void Aura::CallScriptAuraUpdateHandlers(uint32 diff)
{
    uint32 scriptExecuteTime = getMSTime();
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_ON_UPDATE);
        std::list<AuraScript::AuraUpdateHandler>::iterator hookItrEnd = (*scritr)->OnAuraUpdate.end(), hookItr = (*scritr)->OnAuraUpdate.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(*scritr, diff);
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

void Aura::CallScriptEffectUpdateHandlers(uint32 diff, AuraEffectPtr aurEff)
{
    uint32 scriptExecuteTime = getMSTime();
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_UPDATE);
        std::list<AuraScript::EffectUpdateHandler>::iterator effEndItr = (*scritr)->OnEffectUpdate.end(), effItr = (*scritr)->OnEffectUpdate.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(*scritr, diff, aurEff);
        }
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

void Aura::CallScriptEffectUpdatePeriodicHandlers(AuraEffectPtr aurEff)
{
    uint32 scriptExecuteTime = getMSTime();
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_UPDATE_PERIODIC);
        std::list<AuraScript::EffectUpdatePeriodicHandler>::iterator effEndItr = (*scritr)->OnEffectUpdatePeriodic.end(), effItr = (*scritr)->OnEffectUpdatePeriodic.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(*scritr, aurEff);
        }
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

void Aura::CallScriptEffectMasteryUpdateHandlers(AuraEffectPtr aurEff)
{
    uint32 scriptExecuteTime = getMSTime();
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_MASTERY_UPDATE);
        std::list<AuraScript::EffectMasteryUpdateHandler>::iterator effEndItr = (*scritr)->OnEffectMasteryUpdate.end(), effItr = (*scritr)->OnEffectMasteryUpdate.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(*scritr, aurEff);
        }
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

void Aura::CallScriptEffectCalcAmountHandlers(constAuraEffectPtr aurEff, int32 & amount, bool & canBeRecalculated)
{
    uint32 scriptExecuteTime = getMSTime();
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_CALC_AMOUNT);
        std::list<AuraScript::EffectCalcAmountHandler>::iterator effEndItr = (*scritr)->DoEffectCalcAmount.end(), effItr = (*scritr)->DoEffectCalcAmount.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(*scritr, aurEff, amount, canBeRecalculated);
        }
        (*scritr)->_FinishScriptCall();
    }


    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

void Aura::CallScriptEffectCalcPeriodicHandlers(constAuraEffectPtr aurEff, bool & isPeriodic, int32 & amplitude)
{
    uint32 scriptExecuteTime = getMSTime();
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_CALC_PERIODIC);
        std::list<AuraScript::EffectCalcPeriodicHandler>::iterator effEndItr = (*scritr)->DoEffectCalcPeriodic.end(), effItr = (*scritr)->DoEffectCalcPeriodic.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(*scritr, aurEff, isPeriodic, amplitude);
        }
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

void Aura::CallScriptEffectCalcSpellModHandlers(constAuraEffectPtr aurEff, SpellModifier* & spellMod)
{
    uint32 scriptExecuteTime = getMSTime();
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_CALC_SPELLMOD);
        std::list<AuraScript::EffectCalcSpellModHandler>::iterator effEndItr = (*scritr)->DoEffectCalcSpellMod.end(), effItr = (*scritr)->DoEffectCalcSpellMod.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(*scritr, aurEff, spellMod);
        }
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

void Aura::CallScriptEffectAbsorbHandlers(AuraEffectPtr aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, uint32 & absorbAmount, bool& defaultPrevented)
{
    uint32 scriptExecuteTime = getMSTime();
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_ABSORB, aurApp);
        std::list<AuraScript::EffectAbsorbHandler>::iterator effEndItr = (*scritr)->OnEffectAbsorb.end(), effItr = (*scritr)->OnEffectAbsorb.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(*scritr, aurEff, dmgInfo, absorbAmount);
        }
        defaultPrevented = (*scritr)->_IsDefaultActionPrevented();
        (*scritr)->_FinishScriptCall();
    }


    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

void Aura::CallScriptEffectAfterAbsorbHandlers(AuraEffectPtr aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, uint32 & absorbAmount)
{
    uint32 scriptExecuteTime = getMSTime();

    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_AFTER_ABSORB, aurApp);
        std::list<AuraScript::EffectAbsorbHandler>::iterator effEndItr = (*scritr)->AfterEffectAbsorb.end(), effItr = (*scritr)->AfterEffectAbsorb.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(*scritr, aurEff, dmgInfo, absorbAmount);
        }
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

void Aura::CallScriptEffectManaShieldHandlers(AuraEffectPtr aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, uint32 & absorbAmount, bool & /*defaultPrevented*/)
{
    uint32 scriptExecuteTime = getMSTime();
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_MANASHIELD, aurApp);
        std::list<AuraScript::EffectManaShieldHandler>::iterator effEndItr = (*scritr)->OnEffectManaShield.end(), effItr = (*scritr)->OnEffectManaShield.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(*scritr, aurEff, dmgInfo, absorbAmount);
        }
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

void Aura::CallScriptEffectAfterManaShieldHandlers(AuraEffectPtr aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, uint32 & absorbAmount)
{
    uint32 scriptExecuteTime = getMSTime();
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_AFTER_MANASHIELD, aurApp);
        std::list<AuraScript::EffectManaShieldHandler>::iterator effEndItr = (*scritr)->AfterEffectManaShield.end(), effItr = (*scritr)->AfterEffectManaShield.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(*scritr, aurEff, dmgInfo, absorbAmount);
        }
        (*scritr)->_FinishScriptCall();
    }


    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

void Aura::SetScriptData(uint32 type, uint32 data)
{
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
        (*scritr)->SetData(type, data);
}

void Aura::SetScriptGuid(uint32 type, uint64 data)
{
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
        (*scritr)->SetGuid(type, data);
}

bool Aura::CallScriptCheckProcHandlers(AuraApplication const* aurApp, ProcEventInfo& eventInfo)
{
    uint32 scriptExecuteTime = getMSTime();
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_CHECK_PROC, aurApp);
        std::list<AuraScript::CheckProcHandler>::iterator hookItrEnd = (*scritr)->DoCheckProc.end(), hookItr = (*scritr)->DoCheckProc.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            if (!(*hookItr).Call(*scritr, eventInfo))
                return false;
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
    return true;
}

bool Aura::CallScriptPrepareProcHandlers(AuraApplication const* aurApp, ProcEventInfo& eventInfo)
{
    uint32 scriptExecuteTime = getMSTime();
    bool prepare = true;
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_PREPARE_PROC, aurApp);
        std::list<AuraScript::AuraProcHandler>::iterator effEndItr = (*scritr)->DoPrepareProc.end(), effItr = (*scritr)->DoPrepareProc.begin();
        for (; effItr != effEndItr; ++effItr)
            (*effItr).Call(*scritr, eventInfo);

        if (prepare && (*scritr)->_IsDefaultActionPrevented())
            prepare = false;
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);

    return prepare;
}

void Aura::CallScriptProcHandlers(AuraApplication const* aurApp, ProcEventInfo& eventInfo)
{
    uint32 scriptExecuteTime = getMSTime();
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_PROC, aurApp);
        std::list<AuraScript::AuraProcHandler>::iterator hookItrEnd = (*scritr)->OnProc.end(), hookItr = (*scritr)->OnProc.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(*scritr, eventInfo);
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

void Aura::CallScriptAfterProcHandlers(AuraApplication const* aurApp, ProcEventInfo& eventInfo)
{
    uint32 scriptExecuteTime = getMSTime();
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_AFTER_PROC, aurApp);
        std::list<AuraScript::AuraProcHandler>::iterator hookItrEnd = (*scritr)->AfterProc.end(), hookItr = (*scritr)->AfterProc.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(*scritr, eventInfo);
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

bool Aura::CallScriptEffectProcHandlers(constAuraEffectPtr aurEff, AuraApplication const* aurApp, ProcEventInfo& eventInfo)
{
    uint32 scriptExecuteTime = getMSTime();
    bool preventDefault = false;
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_PROC, aurApp);
        std::list<AuraScript::EffectProcHandler>::iterator effEndItr  = (*scritr)->OnEffectProc.end(), effItr  = (*scritr)->OnEffectProc.begin();
        for (; effItr  != effEndItr; ++effItr )
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(*scritr, aurEff, eventInfo);
        }
        if (!preventDefault)
            preventDefault = (*scritr)->_IsDefaultActionPrevented();
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);

    return preventDefault;
}

void Aura::CallScriptAfterEffectProcHandlers(constAuraEffectPtr aurEff, AuraApplication const* aurApp, ProcEventInfo& eventInfo)
{
    uint32 scriptExecuteTime = getMSTime();
    for (std::list<AuraScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_AFTER_PROC, aurApp);
        std::list<AuraScript::EffectProcHandler>::iterator effEndItr  = (*scritr)->AfterEffectProc.end(), effItr  = (*scritr)->AfterEffectProc.begin();
        for (; effItr  != effEndItr ; ++effItr )
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(*scritr, aurEff, eventInfo);
        }
        (*scritr)->_FinishScriptCall();
    }

    scriptExecuteTime = getMSTime() - scriptExecuteTime;
    if (scriptExecuteTime > 15)
        sLog->OutSpecialLog("AuraScript [%u] take more than 15 ms to execute (%u ms)", GetId(), scriptExecuteTime);
}

UnitAura::UnitAura(SpellInfo const* spellproto, uint32 effMask, WorldObject* owner, Unit* caster, SpellPowerEntry const* spellPowerData, int32 *baseAmount, Item* castItem, uint64 casterGUID, int32 castItemLevel)
: Aura(spellproto, owner, caster, spellPowerData, castItem, casterGUID, castItemLevel)
{
    m_spellPowerData = spellPowerData;
    m_AuraDRGroup = DIMINISHING_NONE;
}

void UnitAura::_ApplyForTarget(Unit* target, Unit* caster, AuraApplication * aurApp)
{
    Aura::_ApplyForTarget(target, caster, aurApp);

    // register aura diminishing on apply
    if (DiminishingGroup group = GetDiminishGroup())
        target->ApplyDiminishingAura(group, true);
}

void UnitAura::_UnapplyForTarget(Unit* target, Unit* caster, AuraApplication * aurApp)
{
    Aura::_UnapplyForTarget(target, caster, aurApp);

    // unregister aura diminishing (and store last time)
    if (DiminishingGroup group = GetDiminishGroup())
        target->ApplyDiminishingAura(group, false);
}

void UnitAura::Remove(AuraRemoveMode removeMode)
{
    if (IsRemoved())
        return;
    GetUnitOwner()->RemoveOwnedAura(shared_from_this(), removeMode);
}

void UnitAura::FillTargetMap(std::map<Unit*, uint32> & targets, Unit* caster)
{
    for (uint8 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
    {
        if (!HasEffect(effIndex))
            continue;
        UnitList targetList;
        // non-area aura
        if (GetSpellInfo()->Effects[effIndex].Effect == SPELL_EFFECT_APPLY_AURA ||
            GetSpellInfo()->Effects[effIndex].Effect == SPELL_EFFECT_APPLY_AURA_ON_PET)
        {
            targetList.push_back(GetUnitOwner());
        }
        else
        {
            float radius = GetSpellInfo()->Effects[effIndex].CalcRadius(caster);

            if (!GetUnitOwner()->HasUnitState(UNIT_STATE_ISOLATED))
            {
                switch (GetSpellInfo()->Effects[effIndex].Effect)
                {
                    case SPELL_EFFECT_APPLY_AREA_AURA_PARTY:
                    case SPELL_EFFECT_APPLY_AREA_AURA_RAID:
                    {
                        targetList.push_back(GetUnitOwner());
                        WoWSource::AnyGroupedUnitInObjectRangeCheck u_check(GetUnitOwner(), GetUnitOwner(), radius, GetSpellInfo()->Effects[effIndex].Effect == SPELL_EFFECT_APPLY_AREA_AURA_RAID);
                        WoWSource::UnitListSearcher<WoWSource::AnyGroupedUnitInObjectRangeCheck> searcher(GetUnitOwner(), targetList, u_check);
                        GetUnitOwner()->VisitNearbyObject(radius, searcher);
                        break;
                    }
                    case SPELL_EFFECT_APPLY_AREA_AURA_FRIEND:
                    {
                        targetList.push_back(GetUnitOwner());
                        WoWSource::AnyFriendlyUnitInObjectRangeCheck u_check(GetUnitOwner(), GetUnitOwner(), radius);
                        WoWSource::UnitListSearcher<WoWSource::AnyFriendlyUnitInObjectRangeCheck> searcher(GetUnitOwner(), targetList, u_check);
                        GetUnitOwner()->VisitNearbyObject(radius, searcher);
                        break;
                    }
                    case SPELL_EFFECT_APPLY_AREA_AURA_ENEMY:
                    {
                        if (GetCaster() && GetCaster()->IsInWorld())
                        {
                            WoWSource::AnyAoETargetUnitInObjectRangeCheck u_check(GetCaster(), GetCaster(), radius); // No GetCharmer in searcher
                            WoWSource::UnitListSearcher<WoWSource::AnyAoETargetUnitInObjectRangeCheck> searcher(GetCaster(), targetList, u_check);
                            GetCaster()->VisitNearbyObject(radius, searcher);
                        }
                        else
                            targetList.clear();
                        break;
                    }
                    case SPELL_EFFECT_APPLY_AREA_AURA_PET:
                        targetList.push_back(GetUnitOwner());
                    case SPELL_EFFECT_APPLY_AREA_AURA_OWNER:
                    {
                        if (Unit* owner = GetUnitOwner()->GetCharmerOrOwner())
                            if (GetUnitOwner()->IsWithinDistInMap(owner, radius))
                                targetList.push_back(owner);
                        break;
                    }
                }
            }
        }

        for (UnitList::iterator itr = targetList.begin(); itr!= targetList.end();++itr)
        {
            std::map<Unit*, uint32>::iterator existing = targets.find(*itr);
            if (existing != targets.end())
                existing->second |= 1<<effIndex;
            else
                targets[*itr] = 1<<effIndex;
        }
    }
}

DynObjAura::DynObjAura(SpellInfo const* spellproto, uint32 effMask, WorldObject* owner, Unit* caster, SpellPowerEntry const* spellPowerData, int32 *baseAmount, Item* castItem, uint64 casterGUID, int32 castItemLevel)
: Aura(spellproto, owner, caster, spellPowerData, castItem, casterGUID, castItemLevel)
{
    m_spellPowerData = spellPowerData;
}

void DynObjAura::Remove(AuraRemoveMode removeMode)
{
    if (IsRemoved())
        return;
    _Remove(removeMode);
}

void DynObjAura::FillTargetMap(std::map<Unit*, uint32> & targets, Unit* /*caster*/)
{
    Unit* dynObjOwnerCaster = GetDynobjOwner()->GetCaster();
    float radius = GetDynobjOwner()->GetRadius();

    for (uint8 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
    {
        if (!HasEffect(effIndex))
            continue;

        // Earthquake
        if (GetSpellInfo()->Id == 61882)
            if (effIndex != 0)
                continue;

        UnitList targetList;
        if (GetSpellInfo()->Effects[effIndex].TargetB.GetTarget() == TARGET_DEST_DYNOBJ_ALLY
            || GetSpellInfo()->Effects[effIndex].TargetB.GetTarget() == TARGET_UNIT_DEST_AREA_ALLY)
        {
            WoWSource::AnyFriendlyUnitInObjectRangeCheck u_check(GetDynobjOwner(), dynObjOwnerCaster, radius);
            WoWSource::UnitListSearcher<WoWSource::AnyFriendlyUnitInObjectRangeCheck> searcher(GetDynobjOwner(), targetList, u_check);
            GetDynobjOwner()->VisitNearbyObject(radius, searcher);
        }
        else if (GetSpellInfo()->Effects[effIndex].Effect != SPELL_EFFECT_CREATE_AREATRIGGER)
        {
            WoWSource::AnyAoETargetUnitInObjectRangeCheck u_check(GetDynobjOwner(), dynObjOwnerCaster, radius);
            WoWSource::UnitListSearcher<WoWSource::AnyAoETargetUnitInObjectRangeCheck> searcher(GetDynobjOwner(), targetList, u_check);
            GetDynobjOwner()->VisitNearbyObject(radius, searcher);
        }

        for (UnitList::iterator itr = targetList.begin(); itr!= targetList.end();++itr)
        {
            if (dynObjOwnerCaster->MagicSpellHitResult((*itr), m_spellInfo))
                continue;

            std::map<Unit*, uint32>::iterator existing = targets.find(*itr);
            if (existing != targets.end())
                existing->second |= 1<<effIndex;
            else
                targets[*itr] = 1<<effIndex;
        }
    }
}