#include "ScriptPCH.h"
#include "Containers.h"
#include "dragon_soul.h" 

// 109192 105211

enum ScriptedTexts
{
    SAY_AGGRO       = 0,
    SAY_BERSERK     = 1,
    SAY_DEATH       = 2,
    SAY_INTRO_1     = 3,
    SAY_INTRO_2     = 4,
    SAY_KILL        = 5,
    SAY_TWILIGHT    = 6,
    SAY_UNSTABLE    = 7,
};

enum Spells
{
    SPELL_UNSTABLE_MONSTROSITY_1            = 106372, // 6 secs
    SPELL_UNSTABLE_MONSTROSITY_2            = 106376, // 5 secs
    SPELL_UNSTABLE_MONSTROSITY_3            = 106377, // 4 secs
    SPELL_UNSTABLE_MONSTROSITY_4            = 106378, // 3 secs
    SPELL_UNSTABLE_MONSTROSITY_5            = 106379, // 2 secs
    SPELL_UNSTABLE_MONSTROSITY_6            = 106380, // 1 secs
    SPELL_TWILIGHT_INSTABILITY_AOE_1        = 109176,
    SPELL_TWILIGHT_INSTABILITY_AOE_2        = 106374,
    SPELL_TWILIGHT_INSTABILITY_DMG          = 106375,
    SPELL_UNSTABLE_MONSTROSITY_DUMMY_1      = 106390, // prevents from casting those auras for 4 secs
    SPELL_UNSTABLE_MONSTROSITY_DUMMY_2      = 106381, // ? 8/6 secs in normal/heroic
    
    SPELL_TWILIGHT_SHIFT_AOE                = 106369,
    SPELL_TWILIGHT_SHIFT                    = 106368,
    SPELL_HEROIC_WILL_AOE                   = 105554,
    SPELL_HEROIC_WILL                       = 106108,
    SPELL_FADED_INTO_TWILIGHT               = 105927,
    SPELL_ULTRAXION_NORMAL_REALM_COSMETIC   = 105929,
    SPELL_COSMETIC_INTRO_TRIGGER            = 109397,
    SPELL_COSMETIC_LIGHT_0                  = 105592, // Lightning Cosmetic High
    SPELL_COSMETIC_LIGHT_1                  = 109405, // Lightning Cosmetic
    SPELL_COSMETIC_LIGHT_2                  = 109406, // Lightning Cosmetic Temblor of the Screen

    SPELL_HOUR_OF_TWILIGHT                  = 106371,
    SPELL_HOUR_OF_TWILIGHT_DMG              = 103327, // dmg + forse cast 109231, force cast 106370
    SPELL_HOUR_OF_TWILIGHT_1                = 106174, // remove heroic will
    SPELL_HOUR_OF_TWILIGHT_2                = 106370, // from player, force cast achievement

    SPELL_LOOMING_DARKNESS_DUMMY            = 106498,
    SPELL_LOOMING_DARKNESS_DMG              = 109231,

    SPELL_FADING_LIGHT_1                    = 105925, // from boss to player, triggered by hour of twilight, tank only
    SPELL_FADING_LIGHT_KILL                 = 105926, // kill player
    SPELL_FADING_LIGHT_AOE_1                = 109075, // from boss, triggered by 105925, dps
    SPELL_FADING_LIGHT_DUMMY                = 109200, // ?

    SPELL_TWILIGHT_BURST                    = 106415,
    SPELL_TWILIGHT_ERUPTION                 = 106388,
    
    SPELL_LAST_DEFENDER_OF_AZEROTH          = 106182, // scale + force cast 110327
    SPELL_LAST_DEFENDER_OF_AZEROTH_DUMMY    = 110327,
    SPELL_LAST_DEFENDER_OF_AZEROTH_SCRIPT   = 106218, // ?
    SPELL_LAST_DEFENDER_OF_AZEROTH_DRUID    = 106224,
    SPELL_LAST_DEFENDER_OF_AZEROTH_PALADIN  = 106226,
    SPELL_LAST_DEFENDER_OF_AZEROTH_DK       = 106227,
    SPELL_LAST_DEFENDER_OF_AZEROTH_WARRIOR  = 106080,
    SPELL_LAST_DEFENDER_OF_AZEROTH_MONK     = 129873,

    SPELL_GIFT_OF_LIVE_AURA                 = 105896,
    SPELL_GIFT_OF_LIVE_1                    = 106042,
    SPELL_GIFT_OF_LIVE_SUMMON_1             = 106044,
    SPELL_GIFT_OF_LIVE_2                    = 109345, // triggered by 106042 in 25 ppl

    SPELL_ESSENCE_OF_DREAMS_AURA            = 105900,
    SPELL_ESSENCE_OF_DREAMS_HEAL            = 105996,
    SPELL_ESSENCE_OF_DREAMS_1               = 106049,
    SPELL_ESSENCE_OF_DREAMS_SUMMON_1        = 106047,
    SPELL_ESSENCE_OF_DREAMS_2               = 109344, // triggered by 106049 in 25 ppl
    SPELL_ESSENCE_OF_DREAMS_SUMMON_2        = 109342,

    SPELL_SOURCE_OF_MAGIC_AURA              = 105903,
    SPELL_SOURCE_OF_MAGIC_1                 = 106050,
    SPELL_SOURCE_OF_MAGIC_SUMMON_1          = 106048,
    SPELL_SOURCE_OF_MAGIC_2                 = 109347, // triggered by 106050 in 25 ppl
    SPELL_SOURCE_OF_MAGIC_SUMMON_2          = 109346,

    SPELL_TIMELOOP                          = 105984,
    SPELL_TIMELOOP_HEAL                     = 105992,

    SPELL_ULTRAXION_ACHIEVEMENT_AURA        = 109188,
    SPELL_ULTRAXION_AHCIEVEMENT_FAILED      = 109194,
};

enum Events
{
    EVENT_CHARGING_UP           = 1,
    EVENT_YSERA_TALK            = 2,
    EVENT_MOVE                  = 3,
    EVENT_TALK_1                = 4,
    EVENT_UNSTABLE_MONSTROSITY  = 5,
    EVENT_HOUR_OF_TWILIGHT      = 6,
    EVENT_CHECK_TARGET          = 7,
    EVENT_THRALL                = 8,
    EVENT_ALEXTRASZA            = 9,
    EVENT_YSERA                 = 10,
    EVENT_KALECGOS              = 11,
    EVENT_NOZDORMU              = 12,
    EVENT_END_TALK              = 13,
};

enum Adds
{
    NPC_ULTRAXION_GAUNTLET  = 56305,
    NPC_TWILIGHT_ASSAULTER  = 57281,
    GO_GIFT_OF_LIFE         = 209873,
    GO_ESSENCE_OF_DREAMS    = 209874,
    GO_SOURCE_OF_MAGIC      = 209875,
};

enum Actions
{
    ACTION_TWILIGHT_ERUPTION    = 1,
};

const Position gameobjectPos[2] =
{
    { -1756.24f, -2385.01f, 340.807f, 0.0f },
    { -1755.99f, -2395.31f, 340.803f, 0.0f }
};

class boss_ultraxion: public CreatureScript
{
    public:
        boss_ultraxion() : CreatureScript("boss_ultraxion") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new boss_ultraxionAI(pCreature);
        }

        struct boss_ultraxionAI : public BossAI
        {
            boss_ultraxionAI(Creature* pCreature) : BossAI(pCreature, DATA_ULTRAXION)
            {             
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
                me->SetReactState(REACT_DEFENSIVE);                
                unstableCount = 0;
            }

            void InitializeAI()
            {
                if (!instance || static_cast<InstanceMap*>(me->GetMap())->GetScriptId() != sObjectMgr->GetScriptId(DSScriptName))
                    me->IsAIEnabled = false;
                else if (!me->isDead())
                    Reset();
            }

            void Reset()
            {
                _Reset();
                me->SetVisible(false);
                me->GetMotionMaster()->MovePoint(1, ultraxionPos[0]);
                checkAuraTimer = 1000;

                RemoveEncounterAuras();
                DeleteGameObjects(GO_GIFT_OF_LIFE);
                DeleteGameObjects(GO_ESSENCE_OF_DREAMS);
                DeleteGameObjects(GO_SOURCE_OF_MAGIC);           
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);                
                if (Creature* pThrall = me->FindNearestCreature(NPC_THRALL_1, 300.0f))
                {
                    pThrall->CastStop();
                    pThrall->RemoveAllAuras();
                    pThrall->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    me->SetFacingToObject(pThrall);
                }                   
                if (Creature* pYsera = me->FindNearestCreature(NPC_YSERA_THE_AWAKENED, 300.0f))
                {
                    pYsera->CastStop();
                    pYsera->RemoveAllAuras();
                }
                if (Creature* pAlextrasza = me->FindNearestCreature(NPC_ALEXTRASZA_THE_LIFE_BINDER, 300.0f))
                {
                    pAlextrasza->CastStop();
                    pAlextrasza->RemoveAllAuras();
                }
                if (Creature* pNozdormu = me->FindNearestCreature(NPC_NOZDORMU_THE_TIMELESS_ONE, 300.0f))
                {
                    pNozdormu->CastStop();
                    pNozdormu->RemoveAllAuras();
                }
                if (Creature* pKalecgos = me->FindNearestCreature(NPC_KALECGOS, 300.0f))
                {
                    pKalecgos->CastStop();
                    pKalecgos->RemoveAllAuras();
                }

                unstableCount = 0;
                instance->SetBossState(DATA_ULTRAXION, NOT_STARTED);
                instance->SetBossState(BOSS_ULTRAXION, NOT_STARTED);
            }

            void AttackStart(Unit* victim)
            {
                if (!victim)
                    return;

                if (me->Attack(victim, true))
                    DoStartNoMovement(victim);
            }

            void IsSummonedBy(Unit* /*owner*/)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                unstableCount = 0;
                phase = 1;
                checkAuraTimer = 1000;                
                RemoveEncounterAuras();
                DeleteGameObjects(GO_GIFT_OF_LIFE);
                DeleteGameObjects(GO_ESSENCE_OF_DREAMS);
                DeleteGameObjects(GO_SOURCE_OF_MAGIC);                
                events.ScheduleEvent(EVENT_CHARGING_UP, 1000);
            }

            void EnterCombat(Unit* who)
            {
                Talk(SAY_AGGRO);

                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ULTRAXION_ACHIEVEMENT_AURA);
                RemoveEncounterAuras();
                DeleteGameObjects(GO_GIFT_OF_LIFE);
                DeleteGameObjects(GO_ESSENCE_OF_DREAMS);
                DeleteGameObjects(GO_SOURCE_OF_MAGIC);

                unstableCount = 0;
                phase = 0;

                DoCastAOE(SPELL_TWILIGHT_SHIFT_AOE, true);
                DoCastAOE(SPELL_HEROIC_WILL_AOE, true);
                DoCastAOE(SPELL_LAST_DEFENDER_OF_AZEROTH_DUMMY, true);

                events.ScheduleEvent(EVENT_UNSTABLE_MONSTROSITY, 1000);
                events.ScheduleEvent(EVENT_HOUR_OF_TWILIGHT, 45000);
                events.ScheduleEvent(EVENT_CHECK_TARGET, 4000);
                events.ScheduleEvent(EVENT_THRALL, 5000);
                events.ScheduleEvent(EVENT_ALEXTRASZA, 80000);
                events.ScheduleEvent(EVENT_YSERA, 155000);
                events.ScheduleEvent(EVENT_KALECGOS, 215000);
                events.ScheduleEvent(EVENT_NOZDORMU, 5 * MINUTE * IN_MILLISECONDS);

                DoZoneInCombat();
                instance->SetBossState(DATA_ULTRAXION, IN_PROGRESS);
                instance->SetBossState(BOSS_ULTRAXION, IN_PROGRESS);
            }

            bool AllowAchieve()
            {
                Map::PlayerList const &playerList = instance->instance->GetPlayers();
                if (!playerList.isEmpty())
                {
                    for (Map::PlayerList::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                        if (Player* pPlayer = itr->getSource())
                        {
                            if (constAuraPtr aur = pPlayer->GetAura(SPELL_ULTRAXION_ACHIEVEMENT_AURA))
                                if (aur->GetStackAmount() > 1)
                                    return false;
                        }
                }
                return true;                
            }

            void DoAction(const int32 action)
            {
                if (action == ACTION_TWILIGHT_ERUPTION)
                {
                    unstableCount = 7;
                    events.RescheduleEvent(EVENT_UNSTABLE_MONSTROSITY, 1000);
                }
            }

            void JustDied(Unit* killer)
            {
                _JustDied();

                Talk(SAY_DEATH);

                RemoveEncounterAuras();
                DeleteGameObjects(GO_GIFT_OF_LIFE);
                DeleteGameObjects(GO_ESSENCE_OF_DREAMS);
                DeleteGameObjects(GO_SOURCE_OF_MAGIC);
                me->SetCanFly(false);
                me->RemoveUnitMovementFlag(MOVEMENTFLAG_FLYING);
                killer->SummonGameObject(RAID_MODE(GO_ULTRAXION_LOOT_10N, GO_ULTRAXION_LOOT_25N, GO_ULTRAXION_LOOT_10H, GO_ULTRAXION_LOOT_25H), -1753.63f, -2370.01f, 340.845f, 0.0f, 0.0f, 0.0f, -0.66913f, 0.743145f, 100000);
                instance->SetBossState(DATA_ULTRAXION, DONE);
                instance->SetBossState(BOSS_ULTRAXION, DONE);

				DespawnCreatures(NPC_HARBRINGER_OF_TWILIGHT);
				DespawnCreatures(NPC_HARBRINGER_OF_DESTRUCTION);
				DespawnCreatures(NPC_FORCE_OF_DESTRUCTION);
				DespawnCreatures(NPC_PORTENT_OF_TWILIGHT);

                if (Creature* morchok = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_MORCHOK)))
                if (GameObject * ship = morchok->FindNearestGameObject(GO_HORDE_SHIP, 1000.0f))
                    ship->SetDestructibleState(GO_DESTRUCTIBLE_DESTROYED);

                if (me->FindNearestGameObject(GO_ALIANCE_SHIP_1, 500.0f))
                    DeleteGameObjects(GO_ALIANCE_SHIP_1);

                if (Creature* deathwing = me->FindNearestCreature(NPC_DEATHWING_TOWER, 500.0f, true))
                    deathwing->DespawnOrUnsummon();

                if (Creature* pThrall = me->FindNearestCreature(NPC_THRALL_1, 300.0f))
                {
                    pThrall->InterruptNonMeleeSpells(true);
                    pThrall->RemoveAllAuras();
                    pThrall->AI()->DoAction(1); // ACTION_ULTRAXION_WIN
                }
                if (Creature* pYsera = me->FindNearestCreature(NPC_YSERA_THE_AWAKENED, 300.0f))
                {
                    pYsera->InterruptNonMeleeSpells(true);
                    pYsera->RemoveAllAuras();
                }
                if (Creature* pAlextrasza = me->FindNearestCreature(NPC_ALEXTRASZA_THE_LIFE_BINDER, 300.0f))
                {
                    pAlextrasza->InterruptNonMeleeSpells(true);
                    pAlextrasza->RemoveAllAuras();
                }
                if (Creature* pNozdormu = me->FindNearestCreature(NPC_NOZDORMU_THE_TIMELESS_ONE, 300.0f))
                {
                    pNozdormu->InterruptNonMeleeSpells(true);
                    pNozdormu->RemoveAllAuras();
                }
                if (Creature* pKalecgos = me->FindNearestCreature(NPC_KALECGOS, 300.0f))
                {
                    pKalecgos->InterruptNonMeleeSpells(true);
                    pKalecgos->RemoveAllAuras();
                }
                
                me->DespawnOrUnsummon(10000);

				if (instance)
				{
					instance->DoModifyPlayerCurrenciesIfLevel(395, 7500, 85);
					if (!IsHeroic())
					{
						instance->DoModifyPlayerCurrencies(614, 1);
						instance->DoModifyPlayerCurrencies(615, 1);
					}
					else
					{
						instance->DoModifyPlayerCurrencies(614, 2);
						instance->DoModifyPlayerCurrencies(615, 2);
					}
                    instance->DoStartMovie(73);
				}
            }

            void KilledUnit(Unit* victim)
            {
                if (victim && victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_KILL);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim() && !phase)
                    return;

                if (me->isInCombat())
                if (me->getVictim()->HasAura(SPELL_HEROIC_WILL))
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                if (!target->HasAura(SPELL_HEROIC_WILL))
                    AttackStart(target);

                if (checkAuraTimer <= diff)
                {
                    Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers();
                    if (!PlayerList.isEmpty())
                    {
                        for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                        {
                            Player *player = i->getSource();
                            if (player->isAlive() && !player->HasAura(SPELL_TWILIGHT_SHIFT_AOE) && !player->HasAura(SPELL_HEROIC_WILL) && me->isInCombat())
                            {
                                DoCastAOE(SPELL_TWILIGHT_SHIFT_AOE, true);
                                DoCastAOE(SPELL_HEROIC_WILL_AOE, true);
                            }
                        }
                    }

                    checkAuraTimer = 2000;
                }
                else
                    checkAuraTimer -= diff;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CHARGING_UP:
                            if (Creature* pThrall = me->FindNearestCreature(NPC_THRALL_1, 300.0f))
                                pThrall->CastSpell(pThrall, SPELL_WARD_OF_EARTH);
                            if (Creature* pYsera = me->FindNearestCreature(NPC_YSERA_THE_AWAKENED, 300.0f))
                                pYsera->CastSpell(pYsera, SPELL_WARD_OF_DREAMS);
                            if (Creature* pAlextrasza = me->FindNearestCreature(NPC_ALEXTRASZA_THE_LIFE_BINDER, 300.0f))
                                pAlextrasza->CastSpell(pAlextrasza, SPELL_WARD_OF_LIFE);
                            if (Creature* pNozdormu = me->FindNearestCreature(NPC_NOZDORMU_THE_TIMELESS_ONE, 300.0f))
                                pNozdormu->CastSpell(pNozdormu, SPELL_WARD_OF_TIME);
                            if (Creature* pKalecgos = me->FindNearestCreature(NPC_KALECGOS, 300.0f))
                                pKalecgos->CastSpell(pKalecgos, SPELL_WARD_OF_MAGIC);
                            events.ScheduleEvent(EVENT_YSERA_TALK, 5000);
                            break;
                        case EVENT_YSERA_TALK:
                            if (Creature* pYsera = me->FindNearestCreature(NPC_YSERA_THE_AWAKENED, 300.0f))
                                pYsera->AI()->Talk(0); // SAY_PRE_ULTRAXION                            
                            events.ScheduleEvent(EVENT_MOVE, 7000);
                            break;
                        case EVENT_MOVE:
                        {
                            me->SetVisible(true);
                            me->SetSpeed(MOVE_FLIGHT, 1.0f);                            
                            me->GetMotionMaster()->MovePoint(1, ultraxionPos[1]);
                            DoCastAOE(SPELL_TWILIGHT_SHIFT_AOE, true);    
                            me->AddAura(SPELL_COSMETIC_INTRO_TRIGGER, me);
                            Talk(SAY_INTRO_1);
                            events.ScheduleEvent(EVENT_TALK_1, 13000);
                            break;
                        }
                        case EVENT_TALK_1:
                            Talk(SAY_INTRO_2);                           
                            events.ScheduleEvent(EVENT_END_TALK, 10000);
                            break;
                        case EVENT_END_TALK:
                            phase = 0;
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_SELECTABLE);
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                            if (me->HasAura(SPELL_COSMETIC_INTRO_TRIGGER))
                                me->RemoveAura(SPELL_COSMETIC_INTRO_TRIGGER);
                            me->SetInCombatWithZone();
                            break;
                        case EVENT_CHECK_TARGET:
                            if (!me->IsWithinMeleeRange(me->getVictim()))
                                DoCastAOE(SPELL_TWILIGHT_BURST);
                            events.ScheduleEvent(EVENT_CHECK_TARGET, 4000);
                            break;
                        case EVENT_THRALL:
                            if (Creature* pThrall = me->FindNearestCreature(NPC_THRALL_1, 300.0f))
                            {
                                pThrall->AI()->Talk(0); // SAY_ULTRAXION
                                pThrall->CastSpell(pThrall, SPELL_LAST_DEFENDER_OF_AZEROTH, true);
                            }                            
                            break;
                        case EVENT_ALEXTRASZA:
                            if (Creature* pAlextrasza = me->FindNearestCreature(NPC_ALEXTRASZA_THE_LIFE_BINDER, 300.0f))
                            {
                                pAlextrasza->AI()->Talk(1); // SAY_ULTRAXION
                                uint8 i = urand(0, 1);
                                if (map->GetDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL || map->GetDifficulty() == RAID_DIFFICULTY_25MAN_HEROIC)
                                {
                                    pAlextrasza->CastSpell(gameobjectPos[0].GetPositionX(), gameobjectPos[0].GetPositionY(), gameobjectPos[0].GetPositionZ(), SPELL_GIFT_OF_LIVE_1, true);
                                    pAlextrasza->CastSpell(gameobjectPos[1].GetPositionX(), gameobjectPos[1].GetPositionY(), gameobjectPos[1].GetPositionZ(), SPELL_GIFT_OF_LIVE_1, true);
                                }
                                else
                                    pAlextrasza->CastSpell(gameobjectPos[i].GetPositionX(), gameobjectPos[i].GetPositionY(), gameobjectPos[i].GetPositionZ(), SPELL_GIFT_OF_LIVE_1, true);                                
                            }
                            break;
                        case EVENT_YSERA:
                            if (Creature* pYsera = me->FindNearestCreature(NPC_YSERA_THE_AWAKENED, 300.0f))
                            {
                                pYsera->AI()->Talk(1); // SAY_ULTRAXION
                                uint8 i = urand(0, 1);
                                if (map->GetDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL || map->GetDifficulty() == RAID_DIFFICULTY_25MAN_HEROIC)
                                {
                                    pYsera->CastSpell(gameobjectPos[0].GetPositionX(), gameobjectPos[0].GetPositionY(), gameobjectPos[0].GetPositionZ(), SPELL_ESSENCE_OF_DREAMS_1, true);
                                    pYsera->CastSpell(gameobjectPos[1].GetPositionX(), gameobjectPos[1].GetPositionY(), gameobjectPos[1].GetPositionZ(), SPELL_ESSENCE_OF_DREAMS_1, true);
                                }
                                else
                                    pYsera->CastSpell(gameobjectPos[i].GetPositionX(), gameobjectPos[i].GetPositionY(), gameobjectPos[i].GetPositionZ(), SPELL_ESSENCE_OF_DREAMS_1, true);
                            }
                            break;
                        case EVENT_KALECGOS:
                            if (Creature* pKalecgos = me->FindNearestCreature(NPC_KALECGOS, 300.0f))
                            {
                                pKalecgos->AI()->Talk(0); // SAY_ULTRAXION
                                uint8 i = urand(0, 1);
                                if (map->GetDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL || map->GetDifficulty() == RAID_DIFFICULTY_25MAN_HEROIC)
                                {
                                    pKalecgos->CastSpell(gameobjectPos[0].GetPositionX(), gameobjectPos[0].GetPositionY(), gameobjectPos[0].GetPositionZ(), SPELL_SOURCE_OF_MAGIC_1, true);
                                    pKalecgos->CastSpell(gameobjectPos[1].GetPositionX(), gameobjectPos[1].GetPositionY(), gameobjectPos[1].GetPositionZ(), SPELL_SOURCE_OF_MAGIC_1, true);
                                }
                                else
                                    pKalecgos->CastSpell(gameobjectPos[i].GetPositionX(), gameobjectPos[i].GetPositionY(), gameobjectPos[i].GetPositionZ(), SPELL_SOURCE_OF_MAGIC_1, true);
                            }
                            break;
                        case EVENT_NOZDORMU:
                            if (Creature* pNozdormu = me->FindNearestCreature(NPC_NOZDORMU_THE_TIMELESS_ONE, 300.0f))
                            {
                                pNozdormu->AI()->Talk(0); // SAY_ULTRAXION
                                pNozdormu->CastSpell(pNozdormu, SPELL_TIMELOOP, true);
                            }
                            break;
                        case EVENT_UNSTABLE_MONSTROSITY:
                            unstableCount++;
                            switch (unstableCount)
                            {
                                case 1:
                                    DoCast(me, SPELL_UNSTABLE_MONSTROSITY_1, true);
                                    break;
                                case 2:
                                    me->RemoveAura(SPELL_UNSTABLE_MONSTROSITY_1);
                                    DoCast(me, SPELL_UNSTABLE_MONSTROSITY_2, true);
                                    break;
                                case 3:
                                    me->RemoveAura(SPELL_UNSTABLE_MONSTROSITY_2);
                                    DoCast(me, SPELL_UNSTABLE_MONSTROSITY_3, true);
                                    break;
                                case 4:
                                    Talk(SAY_UNSTABLE);
                                    me->RemoveAura(SPELL_UNSTABLE_MONSTROSITY_3);
                                    DoCast(me, SPELL_UNSTABLE_MONSTROSITY_4, true);
                                    break;
                                case 5:
                                    me->RemoveAura(SPELL_UNSTABLE_MONSTROSITY_4);
                                    DoCast(me, SPELL_UNSTABLE_MONSTROSITY_5, true);
                                    break;
                                case 6:
                                    me->RemoveAura(SPELL_UNSTABLE_MONSTROSITY_5);
                                    DoCast(me, SPELL_UNSTABLE_MONSTROSITY_6, true);
                                    break;
                                default:
                                    break;
                            }
                            if (unstableCount >= 7)
                            {
                                Talk(SAY_BERSERK);
                                DoCast(SPELL_TWILIGHT_ERUPTION);
                            }
                            else
                                events.ScheduleEvent(EVENT_UNSTABLE_MONSTROSITY, MINUTE * IN_MILLISECONDS);
                            break;
                        case EVENT_HOUR_OF_TWILIGHT:
                            Talk(SAY_TWILIGHT);
                            DoCast(SPELL_HOUR_OF_TWILIGHT);
                            events.ScheduleEvent(EVENT_HOUR_OF_TWILIGHT, 45000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:

            uint8 unstableCount;
            uint8 phase;
            uint32 checkAuraTimer;
            Map* map = me->GetMap();

            void DespawnCreatures(uint32 entry)
            {
                std::list<Creature*> creatures;
                GetCreatureListWithEntryInGrid(creatures, me, entry, 1000.0f);

                if (creatures.empty())
                   return;

                for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                     (*iter)->DespawnOrUnsummon();
            }

            void DeleteGameObjects(uint32 entry)
            {
                std::list<GameObject*> gameobjects;
                me->GetGameObjectListWithEntryInGrid(gameobjects, entry, 300.0f);
                if (!gameobjects.empty())
                    for (std::list<GameObject*>::iterator itr = gameobjects.begin(); itr != gameobjects.end(); ++itr)
                        (*itr)->Delete();
            }

            void RemoveEncounterAuras()
            {
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TWILIGHT_SHIFT);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_HEROIC_WILL_AOE);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_HEROIC_WILL);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GIFT_OF_LIVE_AURA);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ESSENCE_OF_DREAMS_AURA);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SOURCE_OF_MAGIC_AURA);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TIMELOOP);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_LAST_DEFENDER_OF_AZEROTH_DK);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_LAST_DEFENDER_OF_AZEROTH_PALADIN);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_LAST_DEFENDER_OF_AZEROTH_DRUID);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_LAST_DEFENDER_OF_AZEROTH_WARRIOR);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_LAST_DEFENDER_OF_AZEROTH_MONK);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_LOOMING_DARKNESS_DUMMY);
            }
        };
};

class spell_ultraxion_twilight_instability : public SpellScriptLoader
{
    public:
        spell_ultraxion_twilight_instability() : SpellScriptLoader("spell_ultraxion_twilight_instability") {}

        class spell_ultraxion_twilight_instability_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_ultraxion_twilight_instability_SpellScript);
            
            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster())
                    return;

                targets.remove_if(AuraCheck(SPELL_HEROIC_WILL, true));
            }

            void HandleScript()
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                if (GetCaster()->HasUnitState(UNIT_STATE_CASTING))
                    return;

                GetCaster()->CastSpell(GetHitUnit(), SPELL_TWILIGHT_INSTABILITY_DMG, true);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ultraxion_twilight_instability_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                AfterHit += SpellHitFn(spell_ultraxion_twilight_instability_SpellScript::HandleScript);
            }

        private:
            class AuraCheck
            {
                public:
                    AuraCheck(uint32 spellId, bool present) : _spellId(spellId), _present(present) {}
            
                    bool operator()(WorldObject* unit)
                    {
                        if (!unit->ToUnit())
                            return true;

                        if (_present)
                            return unit->ToUnit()->HasAura(_spellId);
                        else
                            return !unit->ToUnit()->HasAura(_spellId);
                    }

                private:
                    uint32 _spellId;
                    bool _present;
            };
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_ultraxion_twilight_instability_SpellScript();
        }
};

class spell_ultraxion_twilight_burst : public SpellScriptLoader
{
public:
    spell_ultraxion_twilight_burst() : SpellScriptLoader("spell_ultraxion_twilight_burst") {}

    class spell_ultraxion_twilight_burst_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ultraxion_twilight_burst_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (!GetCaster())
                return;

            targets.remove_if(AuraCheck(SPELL_HEROIC_WILL, true));
        }


        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ultraxion_twilight_burst_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY); 
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ultraxion_twilight_burst_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }

    private:
        class AuraCheck
        {
        public:
            AuraCheck(uint32 spellId, bool present) : _spellId(spellId), _present(present) {}

            bool operator()(WorldObject* unit)
            {
                if (!unit->ToUnit())
                    return true;

                if (_present)
                    return unit->ToUnit()->HasAura(_spellId);
                else
                    return !unit->ToUnit()->HasAura(_spellId);
            }

        private:
            uint32 _spellId;
            bool _present;
        };
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_ultraxion_twilight_burst_SpellScript();
    }
};

class spell_ultraxion_hour_of_twilight_dmg : public SpellScriptLoader
{
    public:
        spell_ultraxion_hour_of_twilight_dmg() : SpellScriptLoader("spell_ultraxion_hour_of_twilight_dmg") { }

        class spell_ultraxion_hour_of_twilight_dmg_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_ultraxion_hour_of_twilight_dmg_SpellScript);

            void FilterTargetsDamage(std::list<WorldObject*>& targets)
            {
                if (!GetCaster())
                    return;

                targets.remove_if(AuraCheck(SPELL_HEROIC_WILL, true));

                uint32 min_players = 1;

                switch (GetCaster()->GetMap()->GetDifficulty())
                {
                    case RAID_DIFFICULTY_10MAN_HEROIC: min_players = 2; break;
                    case RAID_DIFFICULTY_25MAN_NORMAL: min_players = 3; break;
                    case RAID_DIFFICULTY_25MAN_HEROIC: min_players = 5; break;

                    default:                           min_players = 1; break;
                }

                if (targets.size() < min_players)
                    if (Creature* pUltraxion = GetCaster()->ToCreature())
                        pUltraxion->AI()->DoAction(ACTION_TWILIGHT_ERUPTION);
            }

            void FilterTargetsAchievement(std::list<WorldObject*>& targets)
            {
                if (!GetCaster())
                    return;

                targets.remove_if(AuraCheck(SPELL_HEROIC_WILL, true));
            }

            void HandleDummy()
            {
                if (Unit* caster = GetCaster())
                if (Unit* target = GetHitUnit())
                {
                    if (caster->GetMap()->IsHeroic())
                    {
                        if (target->HasAura(SPELL_LOOMING_DARKNESS_DUMMY))
                            target->CastSpell(target, SPELL_LOOMING_DARKNESS_DMG, true);
                    }
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ultraxion_hour_of_twilight_dmg_SpellScript::FilterTargetsDamage, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ultraxion_hour_of_twilight_dmg_SpellScript::FilterTargetsAchievement, EFFECT_2, TARGET_UNIT_SRC_AREA_ENTRY);
                BeforeHit += SpellHitFn(spell_ultraxion_hour_of_twilight_dmg_SpellScript::HandleDummy);
            }

        private:
            class AuraCheck
            {
                public:
                    AuraCheck(uint32 spellId, bool present) : _spellId(spellId), _present(present) {}
            
                    bool operator()(WorldObject* unit)
                    {
                        if (!unit->ToUnit())
                            return true;

                        if (_present)
                            return unit->ToUnit()->HasAura(_spellId);
                        else 
                            return !unit->ToUnit()->HasAura(_spellId);
                    }

                private:
                    uint32 _spellId;
                    bool _present;
            };
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_ultraxion_hour_of_twilight_dmg_SpellScript();
        }
};

class spell_ultraxion_fading_light : public SpellScriptLoader
{
    public:
        spell_ultraxion_fading_light() : SpellScriptLoader("spell_ultraxion_fading_light") { }

        class spell_ultraxion_fading_light_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_ultraxion_fading_light_AuraScript);

            void OnApply(constAuraEffectPtr aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;

                AuraPtr aura = aurEff->GetBase();

                uint32 duration = urand((GetCaster()->GetMap()->IsHeroic() ? 3000 : 5000), 9000);
                aura->SetDuration(duration);
                aura->SetMaxDuration(duration);
            }

            void OnRemove(constAuraEffectPtr aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (GetTarget())
                {    
                    if (GetTarget()->HasAura(SPELL_TWILIGHT_SHIFT))
                        GetTarget()->CastSpell(GetTarget(), SPELL_FADING_LIGHT_KILL, true);
                    else if (GetTarget()->HasAura(SPELL_HEROIC_WILL))
                        GetTarget()->CastSpell(GetTarget(), SPELL_TWILIGHT_SHIFT, true);
                }
            }

            void Register()
            {
                AfterEffectApply += AuraEffectApplyFn(spell_ultraxion_fading_light_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_ultraxion_fading_light_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);

            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_ultraxion_fading_light_AuraScript();
        }

        class spell_ultraxion_fading_light_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_ultraxion_fading_light_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster())
                    return;

                targets.remove_if(DPSCheck());
                if (Creature* pUltraxion = GetCaster()->ToCreature())
                    if (Unit* pTarget = pUltraxion->AI()->SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 0.0f, true))
                        targets.remove(pTarget);

                uint32 min_players = 1;
                switch (GetCaster()->GetMap()->GetDifficulty())
                {
                    case RAID_DIFFICULTY_10MAN_HEROIC: min_players = 2; break;
                    case RAID_DIFFICULTY_25MAN_NORMAL: min_players = 3; break;
                    case RAID_DIFFICULTY_25MAN_HEROIC: min_players = 6; break;

                    default:                           min_players = 1; break;
                }

                if (targets.size() > min_players)
                    WoWSource::Containers::RandomResizeList(targets, min_players);
            }
           
            void Register()
            {
                if (m_scriptSpellId == SPELL_FADING_LIGHT_AOE_1)
                    OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ultraxion_fading_light_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            }

        private:
            class DPSCheck
            {
                public:
                    DPSCheck() {}
            
                    bool operator()(WorldObject* unit)
                    {
                        if (unit->GetTypeId() != TYPEID_PLAYER)
                            return true;

                        switch (unit->ToPlayer()->GetSpecializationId(unit->ToPlayer()->GetActiveSpec()))
                        {
                            case SPEC_WARRIOR_PROTECTION:
                            case SPEC_PALADIN_HOLY:
                            case SPEC_PALADIN_PROTECTION:
                            case SPEC_PRIEST_DISCIPLINE:
                            case SPEC_PRIEST_HOLY:
                            case SPEC_DK_BLOOD:
                            case SPEC_SHAMAN_RESTORATION:
                            case SPEC_DRUID_RESTORATION:
                            case SPEC_DRUID_BEAR:
                            case SPEC_MONK_BREWMASTER:
                            case SPEC_MONK_MISTWEAVER:                   
                                return true;
                            default:
                                return false;
                        }
                        return false;
                    }

                private:
                    uint32 _spellId;
            };
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_ultraxion_fading_light_SpellScript();
        }
};

class spell_ultraxion_last_defender_of_azeroth : public SpellScriptLoader
{
    public:
        spell_ultraxion_last_defender_of_azeroth() : SpellScriptLoader("spell_ultraxion_last_defender_of_azeroth") { }

        class spell_ultraxion_last_defender_of_azeroth_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_ultraxion_last_defender_of_azeroth_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster())
                    return;

                targets.remove_if(TankCheck());
            }
           
            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ultraxion_last_defender_of_azeroth_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ultraxion_last_defender_of_azeroth_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
            }

        private:
            class TankCheck
            {
                public:
                    TankCheck() {}
            
                    bool operator()(WorldObject* unit)
                    {
                        if (unit->GetTypeId() != TYPEID_PLAYER)
                            return true;                        
                            switch (unit->ToPlayer()->GetSpecializationId(unit->ToPlayer()->GetActiveSpec()))
                        {
                            case SPEC_WARRIOR_PROTECTION:
                            case SPEC_PALADIN_PROTECTION:
                            case SPEC_DK_BLOOD:
                            case SPEC_DRUID_BEAR:
                            case SPEC_MONK_BREWMASTER:
                                    return false;
                            default:
                                return true;
                        }
                        return true;
                    }

                private:
                    uint32 _spellId;
            };
        };
        SpellScript* GetSpellScript() const
        {
            return new spell_ultraxion_last_defender_of_azeroth_SpellScript();
        }
};

class spell_ultraxion_last_defender_of_azeroth_dummy : public SpellScriptLoader
{
    public:
        spell_ultraxion_last_defender_of_azeroth_dummy() : SpellScriptLoader("spell_ultraxion_last_defender_of_azeroth_dummy") { }

        class spell_ultraxion_last_defender_of_azeroth_dummy_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_ultraxion_last_defender_of_azeroth_dummy_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                if (GetHitUnit()->GetTypeId() != TYPEID_PLAYER)
                    return;

                switch (GetHitUnit()->ToPlayer()->GetSpecializationId(GetHitUnit()->ToPlayer()->GetActiveSpec()))
                {
                    case SPEC_WARRIOR_PROTECTION:
                        GetHitUnit()->CastSpell(GetHitUnit(), SPELL_LAST_DEFENDER_OF_AZEROTH_WARRIOR, true);
                        break;
                    case SPEC_PALADIN_PROTECTION:
                        GetHitUnit()->CastSpell(GetHitUnit(), SPELL_LAST_DEFENDER_OF_AZEROTH_PALADIN, true);
                        break;
                    case SPEC_DK_BLOOD:
                        GetHitUnit()->CastSpell(GetHitUnit(), SPELL_LAST_DEFENDER_OF_AZEROTH_DK, true);
                        break;
                    case SPEC_DRUID_BEAR:
                        GetHitUnit()->CastSpell(GetHitUnit(), SPELL_LAST_DEFENDER_OF_AZEROTH_DRUID, true);
                        break;
                    case SPEC_MONK_BREWMASTER:
                        GetHitUnit()->CastSpell(GetHitUnit(), SPELL_LAST_DEFENDER_OF_AZEROTH_MONK, true);
                        break;
                }
            }
           
            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_ultraxion_last_defender_of_azeroth_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_ultraxion_last_defender_of_azeroth_dummy_SpellScript();
        }
};

class spell_ultraxion_heroic_will : public SpellScriptLoader
{
    public:
        spell_ultraxion_heroic_will() : SpellScriptLoader("spell_ultraxion_heroic_will") { }

        class spell_ultraxion_heroic_will_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_ultraxion_heroic_will_AuraScript);

            void OnRemove(constAuraEffectPtr aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (GetTarget())
                    GetTarget()->CastSpell(GetTarget(), SPELL_FADED_INTO_TWILIGHT, true);
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_ultraxion_heroic_will_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_PACIFY_SILENCE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_ultraxion_heroic_will_AuraScript();
        }
};

class spell_ultraxion_cosmetic_lightning : public SpellScriptLoader
{
public:
    spell_ultraxion_cosmetic_lightning() : SpellScriptLoader("spell_ultraxion_cosmetic_lightning") { }

    class spell_ultraxion_cosmetic_lightning_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_ultraxion_cosmetic_lightning_AuraScript);

        void OnPeriodic(constAuraEffectPtr aurEff)
        {
            if (Unit* caster = GetCaster())
            {
                uint32 SpellIds[3] = { SPELL_COSMETIC_LIGHT_0, SPELL_COSMETIC_LIGHT_1, SPELL_COSMETIC_LIGHT_2 };
                uint32 i = urand(0, 2);
                caster->CastSpell(caster, SpellIds[i], true);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_ultraxion_cosmetic_lightning_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_ultraxion_cosmetic_lightning_AuraScript();
    }
};

class TriggerCheck
{
public:
    bool operator()(const WorldObject* target)
    {
        if (target->ToCreature())
        {
            if (target->GetEntry() == NPC_TWILIGHT_ASSAULTER)
                return false;
        }
        return true;
    }
};



class spell_ultraxion_cosmetic_intro : public SpellScriptLoader
{
public:
    spell_ultraxion_cosmetic_intro() : SpellScriptLoader("spell_ultraxion_cosmetic_intro") {}

    class spell_ultraxion_cosmetic_intro_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ultraxion_cosmetic_intro_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(TriggerCheck());
            if (targets.size() > 1)
            {
                WoWSource::RandomResizeList(targets, 3);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ultraxion_cosmetic_intro_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_ultraxion_cosmetic_intro_SpellScript();
    }
};

class spell_ultraxion_time_loop : public SpellScriptLoader
{
    public:
        spell_ultraxion_time_loop() : SpellScriptLoader("spell_ultraxion_time_loop") { }

        class spell_ultraxion_time_loop_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_ultraxion_time_loop_AuraScript);

            bool Load()
            {
                return GetUnitOwner()->GetTypeId() == TYPEID_PLAYER;
            }

            void CalculateAmount(constAuraEffectPtr /*aurEff*/, int32 & amount, bool& /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void Absorb(AuraEffectPtr aurEff, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                Unit* victim = GetTarget();
                int32 remainingHealth = victim->GetHealth() - dmgInfo.GetDamage();

                if (remainingHealth <= 0)
                {
                    absorbAmount = dmgInfo.GetDamage();
                    int32 healAmount = int32(victim->GetMaxHealth());
                    victim->CastCustomSpell(victim, SPELL_TIMELOOP_HEAL, &healAmount, NULL, NULL, true, NULL, aurEff);
                    aurEff->GetBase()->Remove();
                }
                else
                {
                    absorbAmount = 0;
                }
            }

            void Register()
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_ultraxion_time_loop_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_ultraxion_time_loop_AuraScript::Absorb, EFFECT_1);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_ultraxion_time_loop_AuraScript();
        }
};

typedef boss_ultraxion::boss_ultraxionAI UltraxionAI;

class achievement_minutes_to_midnight : public AchievementCriteriaScript
{
    public:
        achievement_minutes_to_midnight() : AchievementCriteriaScript("achievement_minutes_to_midnight") { }

        bool OnCheck(Player* source, Unit* target)
        {
            if (!target)
                return false;

            if (UltraxionAI* ultraxionAI = CAST_AI(UltraxionAI, target->GetAI()))
                return ultraxionAI->AllowAchieve();

            return false;
        }
};

void AddSC_boss_ultraxion()
{
    new boss_ultraxion();
    new spell_ultraxion_twilight_instability();
    new spell_ultraxion_twilight_burst();
    new spell_ultraxion_hour_of_twilight_dmg();
    new spell_ultraxion_fading_light();
    new spell_ultraxion_last_defender_of_azeroth();
    new spell_ultraxion_last_defender_of_azeroth_dummy();
    new spell_ultraxion_heroic_will();
    new spell_ultraxion_cosmetic_lightning();
    new spell_ultraxion_cosmetic_intro();
    new spell_ultraxion_time_loop();
    new achievement_minutes_to_midnight();
}
