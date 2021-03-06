/*
SQL ZONAXTREMA 				  
Desarrollado por:
Fecha: 5/1/2020
### Scripts Trabajados por ZONAXTREMA  ########
### Datos SQL ###########################################*/
SET @CREATURE  	:= 'Saronite Deposit';
SET @ENTRY 		:= '189980';
/*SET @MAP   		:= 530;       *//*(Outland)*/
/*SET @ZONE  		:= 6455;      *//*(Sunstrider Isle)*/
/*SET @AREA  		:= 3431;      *//*(Sunstrider Isle)*/
/*#######################################################*/


/* Step 4.1 Table  `gameobject_template */
DELETE FROM `gameobject_template` WHERE `entry` = @ENTRY;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `faction`, `flags`, `size`, `questItem1`, `questItem2`, `questItem3`,  `questItem4`, `questItem5`, `questItem6`, `data0`, `data1`, `data2`, `data3`, `data4`, `data5`, `data6`, `data7`, `data8`, `data9`, `data10`, `data11`, `data12`, `data13`, `data14`, `data15`, `data16`, `data17`, `data18`, `data19`, `data20`, `data21`, `data22`, `data23`, `data24`, `data25`, `data26`, `data27`, `data28`, `data29`, `data30`, `data31`, `unkInt32`, `AIName`, `ScriptName`, `WDBVerified`) VALUES 
(@ENTRY, '3', '7804', 'Saronite Deposit', '', '', '', '0', '0', '0.5', '0', '0', '0', '0', '0', '0', '1783', '189980', '0', '1', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '75', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '65', '', '', '15595'); 


/* Step 4.1 Table  `gameobject */
DELETE FROM `gameobject` WHERE `id` = @ENTRY;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `phaseId`, `position_x`, `position_y`, `position_z`, `orientation`,`rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `isActive`, `protect_anti_doublet`) VALUES 
(NULL, @ENTRY, '571', '210', '4519', '1', '1', '0', '6522.39', '2173.17', '509.07', '-3.10665', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '4531', '1', '1', '0', '6497.03', '3510.88', '540.232', '-2.70526', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '4505', '1', '1', '0', '6484.37', '294.937', '399.821', '-1.15192', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '4507', '1', '1', '0', '6483.74', '1237.93', '286.932', '1.37881', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '4506', '1', '1', '0', '6481.97', '540.381', '438.703', '0.47124', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '4518', '1', '1', '0', '6477.44', '2487.12', '475.981', '-0.27925', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '4506', '1', '1', '0', '6466.4', '648.508', '428.804', '1.79769', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '4520', '1', '1', '0', '6448.21', '1897.02', '515.36', '0.36652', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '6277.39', '4221.15', '-40.3804', '0.59341', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '66', '66', '1', '1', '0', '6265.88', '-2931.17', '307.012', '0.76794', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '2817', '2817', '1', '1', '0', '6185.18', '718.735', '196', '2.09439', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '6173.29', '4609.01', '-92.2689', '-1.90241', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '2817', '2817', '1', '1', '0', '6125.19', '761.549', '183.29', '0.331611', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '66', '4371', '1', '1', '0', '6055.43', '-4338.67', '378.82', '2.60054', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '6053.3', '4795.46', '-92.8918', '0.8203', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '6037.1', '4847.04', '-93.403', '-1.25664', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '2817', '2817', '1', '1', '0', '6035.78', '631.157', '201.116', '0.994837', '0', '0', '0', '1', '1200', '100', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '4283', '1', '1', '0', '6001.92', '4160.64', '-71.2961', '0.83776', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '2817', '2817', '1', '1', '0', '5978.92', '559.39', '212.441', '-2.80997', '0', '0', '0', '1', '1200', '100', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '4522', '1', '1', '0', '5966.52', '1908.87', '580.817', '0.71559', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '2817', '2817', '1', '1', '0', '5903.81', '380.04', '220.506', '-2.72271', '0', '0', '0', '1', '1200', '100', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '4615', '1', '1', '0', '5894.7', '1913.14', '-345.937', '-2.07694', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '5882.23', '5916.37', '-25.0905', '-1.6057', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '5861.32', '4419.11', '-89.7436', '-3.07177', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '4391', '1', '1', '0', '5834', '5370.13', '-87.9156', '-0.06981', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '4615', '1', '1', '0', '5826.98', '1863.07', '-345.809', '1.90241', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '4522', '1', '1', '0', '5790.74', '2214.07', '515.154', '1.309', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '4615', '1', '1', '0', '5775.48', '2081.83', '-342.742', '0.71559', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '5762.14', '3502.19', '8.5711', '-0.5236', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '4522', '1', '1', '0', '5753.73', '2282.75', '533.972', '-1.53589', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '4615', '1', '1', '0', '5749.15', '2086.66', '-343.52', '-2.09439', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '4615', '1', '1', '0', '5715.87', '2059.13', '-340.391', '-2.25147', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '4615', '1', '1', '0', '5706.65', '2046.59', '-339.659', '-2.3911', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '4283', '1', '1', '0', '5704.43', '3884.6', '-72.5278', '1.8675', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '5679.05', '5598.78', '-73.9954', '-0.97738', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '66', '4323', '1', '1', '0', '5673.48', '-3554.34', '373.482', '0.71558', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '5669.97', '5463.91', '-68.9469', '2.89724', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '4522', '1', '1', '0', '5665.19', '1897.49', '519.675', '2.93214', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '4295', '1', '1', '0', '5660.41', '3699.91', '-13.5126', '-2.37364', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '4522', '1', '1', '0', '5636.89', '1931.16', '517.353', '-1.50098', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '66', '4489', '1', '1', '0', '5566.94', '-1860.93', '235.266', '-1.79769', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '4386', '1', '1', '0', '5529.63', '4501.9', '-132.392', '-2.33874', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '4293', '1', '1', '0', '5528.73', '4681.65', '-127.088', '0.43633', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '4197', '4197', '1', '1', '0', '5467.79', '3264.6', '431.625', '-2.11185', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '4290', '1', '1', '0', '5461.79', '4916.51', '-197.193', '-0.62832', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '4387', '1', '1', '0', '5447.93', '4519.89', '-135.988', '-1.67551', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '4293', '1', '1', '0', '5446.75', '5166.05', '-133.793', '-3.07177', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '4387', '1', '1', '0', '5418.62', '4371.29', '-137.676', '1.55334', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '4197', '4197', '1', '1', '0', '5391.04', '2478.3', '413.997', '-1.55334', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '4290', '1', '1', '0', '5388.27', '4753.33', '-198.798', '2.30383', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '4290', '1', '1', '0', '5385.53', '4913.98', '-195.354', '1.83259', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '4197', '4575', '1', '1', '0', '5382.98', '2588.29', '413.491', '-0.645772', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '4293', '1', '1', '0', '5358.9', '4514.82', '-127.633', '2.07694', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '4197', '4575', '1', '1', '0', '5351.96', '3172.97', '383.045', '-1.69297', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '5333.07', '4300.51', '-148.082', '0.68068', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '5331.86', '4405.6', '-126.997', '-1.51844', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '4197', '4197', '1', '1', '0', '5296.4', '2187.28', '509.532', '-0.174532', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '66', '4279', '1', '1', '0', '5213.89', '-3377.16', '290.401', '1.81514', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '4197', '4197', '1', '1', '0', '5170.72', '2170.47', '418.683', '-0.471238', '0', '0', '0', '1', '1200', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '5093.57', '4341.18', '-82.7253', '3.08918', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '4293', '1', '1', '0', '5086.37', '5098.74', '-134.572', '2.07694', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '210', '210', '1', '1', '0', '5073.55', '1900.11', '672.424', '1.18682', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '4388', '1', '1', '0', '5062.48', '5031.56', '-131.003', '-1.22173', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '5062.38', '4526.64', '-96.4765', '0.20944', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '5056.29', '4819.88', '-132.884', '-2.19912', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '4197', '4197', '1', '1', '0', '5016.65', '2941.86', '375.215', '-2.21656', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '5015.7', '4691.84', '-91.0594', '1.43117', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '4988.2', '5373.05', '-94.5907', '1.36136', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '4197', '4197', '1', '1', '0', '4894.2', '2171.45', '362.681', '1.62316', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '4197', '4585', '1', '1', '0', '4889.06', '1872.17', '457.047', '-2.65289', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '394', '4366', '1', '1', '0', '4877.31', '-3794.38', '341.362', '1.18682', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '4866.02', '4570.89', '-60.3061', '2.00713', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '4197', '4197', '1', '1', '0', '4816.67', '3641.68', '365.962', '-0.575957', '0', '0', '0', '1', '1200', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '4724.21', '5107.49', '-46.3651', '-1.78023', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '3711', '3711', '1', '1', '0', '4709.27', '4367.73', '-31.7133', '-0.76794', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '4197', '4587', '1', '1', '0', '4560.84', '3708.44', '373.765', '1.81514', '0', '0', '0', '1', '1200', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '4197', '4197', '1', '1', '0', '4361.93', '3678.48', '393.914', '-1.39626', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '4197', '4612', '1', '1', '0', '4342.11', '2426', '350.083', '0.506145', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '571', '4197', '4584', '1', '1', '0', '4223.81', '1779.78', '350.496', '3.07541', '0', '0', '0.999452', '0.0330874', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '4197', '4197', '1', '1', '0', '3972.69', '2032.2', '354.518', '2.35619', '0', '0', '0', '1', '1200', '255', '1', '0', '0'), 
(NULL, @ENTRY, '599', '4264', '4264', '6', '1', '0', '1063.82', '714.079', '203.998', '2.25147', '0', '0', '0', '1', '1200', '0', '1', '0', '0'), 
(NULL, @ENTRY, '599', '4264', '4264', '6', '1', '0', '949.208', '845.198', '186.281', '-2.60053', '0', '0', '0', '1', '1200', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '6880.53', '-1360.63', '830.989', '3.18679', '0', '0', '0.999745', '-0.0225977', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '6859.13', '-1348.68', '832.253', '2.5035', '0', '0', '0.949534', '0.313663', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '6834.68', '-1322.29', '834.221', '2.94725', '0', '0', '0.995282', '0.0970208', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '6824.67', '-1314', '833.432', '1.14083', '0', '0', '0.539981', '0.841677', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '6853.65', '-1324.49', '835.722', '1.0191', '0', '0', '0.487783', '0.872965', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '6989.77', '-1355.05', '830.921', '5.5312', '0', '0', '0.367195', '-0.930144', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '7014.55', '-1356.21', '831.132', '6.04171', '0', '0', '0.120445', '-0.99272', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '7027.55', '-1342.42', '830.913', '0.905203', '0', '0', '0.437307', '0.899312', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '7014.56', '-1340.74', '832.836', '2.7823', '0', '0', '0.983907', '0.178679', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '7004.19', '-1344.16', '831.456', '1.53745', '0', '0', '0.695219', '0.718798', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '6990.57', '-1295.07', '819.458', '0.799163', '0', '0', '0.389033', '0.921224', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '7011.43', '-1304.3', '819.284', '0.499635', '0', '0', '0.247227', '0.968958', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '7030.98', '-1305.74', '819.697', '0.476066', '0', '0', '0.235791', '0.971804', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '7042.75', '-1310.93', '818.955', '5.64791', '0', '0', '0.312323', '-0.949976', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '7026.86', '-1317.62', '819.811', '4.12817', '0', '0', '0.880781', '-0.473524', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '7002.57', '-1329.05', '820.211', '4.30096', '0', '0', '0.836637', '-0.547758', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '6875.21', '-1262.11', '812.32', '2.09399', '0', '0', '0.865923', '0.500177', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '6854.77', '-1248.57', '807.672', '2.73016', '0', '0', '0.978915', '0.204269', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '6848.53', '-1238.85', '805.117', '1.39498', '0', '0', '0.642297', '0.766456', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '6864.54', '-1242.49', '808.315', '0.868763', '0', '0', '0.42085', '0.90713', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '6880.87', '-1237.18', '809.793', '0.986572', '0', '0', '0.473523', '0.880782', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '6959.88', '-1214.21', '808.753', '1.19862', '0', '0', '0.564072', '0.825726', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '6975.75', '-1210.58', '809.156', '0.366098', '0', '0', '0.182028', '0.983293', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '6990.59', '-1205.96', '808.817', '0.23258', '0', '0', '0.116028', '0.993246', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '6991.97', '-1214.36', '809.006', '4.33237', '0', '0', '0.827931', '-0.56083', '300', '0', '1', '0', '0'), 
(NULL, @ENTRY, '571', '67', '4425', '1', '1', '0', '6980.53', '-1220.02', '809.432', '3.99465', '0', '0', '0.910408', '-0.413711', '300', '0', '1', '0', '0'); 


/*## Step 2: GObject Loots ###########################################*/

/* Step x.x Table  `gameobject_loot_template */
DELETE FROM gameobject_loot_template WHERE  `entry` = @ENTRY;
INSERT INTO `gameobject_loot_template` (`entry`,`item`,`ChanceOrQuestChance`,`lootmode`,`groupid`,`mincountOrRef`,`maxcount`) VALUES 
(@ENTRY, '36912', '100', '1', '0', '1', '3'), 
(@ENTRY, '37701', '45', '1', '1', '1', '2'), 
(@ENTRY, '37703', '45', '1', '1', '1', '2'); 


/* Step 3.1 Table  `item_template */
DELETE FROM item_template WHERE entry = 36912;
INSERT INTO `item_template` (`entry`, `class`, `subclass`, `SoundOverrideSubclass`, `name`, `displayid`, `Quality`, `Flags`, `FlagsExtra`, `Unk430_1`, `Unk430_2`, `BuyCount`, `BuyPrice`, `SellPrice`, `InventoryType`, `AllowableClass`, `AllowableRace`, `ItemLevel`, `RequiredLevel`, `RequiredSkill`, `RequiredSkillRank`, `requiredspell`, `requiredhonorrank`, `RequiredCityRank`, `RequiredReputationFaction`, `RequiredReputationRank`, `maxcount`, `stackable`, `ContainerSlots`, `stat_type1`, `stat_value1`, `stat_unk1_1`, `stat_unk2_1`, `stat_type2`, `stat_value2`, `stat_unk1_2`, `stat_unk2_2`, `stat_type3`, `stat_value3`, `stat_unk1_3`, `stat_unk2_3`, `stat_type4`, `stat_value4`, `stat_unk1_4`, `stat_unk2_4`, `stat_type5`, `stat_value5`, `stat_unk1_5`, `stat_unk2_5`, `stat_type6`, `stat_value6`, `stat_unk1_6`, `stat_unk2_6`, `stat_type7`, `stat_value7`, `stat_unk1_7`, `stat_unk2_7`, `stat_type8`, `stat_value8`, `stat_unk1_8`, `stat_unk2_8`, `stat_type9`, `stat_value9`, `stat_unk1_9`, `stat_unk2_9`, `stat_type10`, `stat_value10`, `stat_unk1_10`, `stat_unk2_10`, `ScalingStatDistribution`, `DamageType`, `delay`, `RangedModRange`, `spellid_1`, `spelltrigger_1`, `spellcharges_1`, `spellcooldown_1`, `spellcategory_1`, `spellcategorycooldown_1`, `spellid_2`, `spelltrigger_2`, `spellcharges_2`, `spellcooldown_2`, `spellcategory_2`, `spellcategorycooldown_2`, `spellid_3`, `spelltrigger_3`, `spellcharges_3`, `spellcooldown_3`, `spellcategory_3`, `spellcategorycooldown_3`, `spellid_4`, `spelltrigger_4`, `spellcharges_4`, `spellcooldown_4`, `spellcategory_4`, `spellcategorycooldown_4`, `spellid_5`, `spelltrigger_5`, `spellcharges_5`, `spellcooldown_5`, `spellcategory_5`, `spellcategorycooldown_5`, `bonding`, `description`, `PageText`, `LanguageID`, `PageMaterial`, `startquest`, `lockid`, `Material`, `sheath`, `RandomProperty`, `RandomSuffix`, `itemset`, `MaxDurability`, `area`, `Map`, `BagFamily`, `TotemCategory`, `socketColor_1`, `socketContent_1`, `socketColor_2`, `socketContent_2`, `socketColor_3`, `socketContent_3`, `socketBonus`, `GemProperties`, `ArmorDamageModifier`, `duration`, `ItemLimitCategory`, `HolidayId`, `StatScalingFactor`, `CurrencySubstitutionId`, `CurrencySubstitutionCount`, `flagsCustom`, `WDBVerified`) VALUES 
('36912', '7', '7', '-1', 'Saronite Ore', '57335', '1', '262144', '8192', '0.9957', '1', '1', '6000', '1500', '0', '-1', '-1', '75', '0', '755', '400', '0', '0', '0', '0', '0', '0', '20', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '-1', '0', '-1', '0', '0', '0', '-1', '0', '-1', '0', '0', '0', '-1', '0', '-1', '0', '0', '0', '-1', '0', '-1', '0', '0', '0', '-1', '0', '-1', '0', '', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '1024', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '16135'); 


/* Step 3.1 Table  `item_template */
DELETE FROM item_template WHERE entry = 37701;
INSERT INTO `item_template` (`entry`, `class`, `subclass`, `SoundOverrideSubclass`, `name`, `displayid`, `Quality`, `Flags`, `FlagsExtra`, `Unk430_1`, `Unk430_2`, `BuyCount`, `BuyPrice`, `SellPrice`, `InventoryType`, `AllowableClass`, `AllowableRace`, `ItemLevel`, `RequiredLevel`, `RequiredSkill`, `RequiredSkillRank`, `requiredspell`, `requiredhonorrank`, `RequiredCityRank`, `RequiredReputationFaction`, `RequiredReputationRank`, `maxcount`, `stackable`, `ContainerSlots`, `stat_type1`, `stat_value1`, `stat_unk1_1`, `stat_unk2_1`, `stat_type2`, `stat_value2`, `stat_unk1_2`, `stat_unk2_2`, `stat_type3`, `stat_value3`, `stat_unk1_3`, `stat_unk2_3`, `stat_type4`, `stat_value4`, `stat_unk1_4`, `stat_unk2_4`, `stat_type5`, `stat_value5`, `stat_unk1_5`, `stat_unk2_5`, `stat_type6`, `stat_value6`, `stat_unk1_6`, `stat_unk2_6`, `stat_type7`, `stat_value7`, `stat_unk1_7`, `stat_unk2_7`, `stat_type8`, `stat_value8`, `stat_unk1_8`, `stat_unk2_8`, `stat_type9`, `stat_value9`, `stat_unk1_9`, `stat_unk2_9`, `stat_type10`, `stat_value10`, `stat_unk1_10`, `stat_unk2_10`, `ScalingStatDistribution`, `DamageType`, `delay`, `RangedModRange`, `spellid_1`, `spelltrigger_1`, `spellcharges_1`, `spellcooldown_1`, `spellcategory_1`, `spellcategorycooldown_1`, `spellid_2`, `spelltrigger_2`, `spellcharges_2`, `spellcooldown_2`, `spellcategory_2`, `spellcategorycooldown_2`, `spellid_3`, `spelltrigger_3`, `spellcharges_3`, `spellcooldown_3`, `spellcategory_3`, `spellcategorycooldown_3`, `spellid_4`, `spelltrigger_4`, `spellcharges_4`, `spellcooldown_4`, `spellcategory_4`, `spellcategorycooldown_4`, `spellid_5`, `spelltrigger_5`, `spellcharges_5`, `spellcooldown_5`, `spellcategory_5`, `spellcategorycooldown_5`, `bonding`, `description`, `PageText`, `LanguageID`, `PageMaterial`, `startquest`, `lockid`, `Material`, `sheath`, `RandomProperty`, `RandomSuffix`, `itemset`, `MaxDurability`, `area`, `Map`, `BagFamily`, `TotemCategory`, `socketColor_1`, `socketContent_1`, `socketColor_2`, `socketContent_2`, `socketColor_3`, `socketContent_3`, `socketBonus`, `GemProperties`, `ArmorDamageModifier`, `duration`, `ItemLimitCategory`, `HolidayId`, `StatScalingFactor`, `CurrencySubstitutionId`, `CurrencySubstitutionCount`, `flagsCustom`, `WDBVerified`) VALUES 
('37701', '7', '10', '-1', 'Crystallized Earth', '60020', '1', '1024', '8192', '0.9884', '1', '1', '2400', '600', '0', '-1', '-1', '75', '0', '0', '0', '0', '0', '0', '0', '0', '0', '10', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '49248', '0', '-1', '500', '0', '-1', '0', '0', '0', '-1', '0', '-1', '0', '0', '0', '-1', '0', '-1', '0', '0', '0', '-1', '0', '-1', '0', '0', '0', '-1', '0', '-1', '0', '', '0', '0', '0', '0', '0', '4', '0', '0', '0', '0', '0', '0', '0', '1224', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '16135'); 


/* Step 3.1 Table  `item_template */
DELETE FROM item_template WHERE entry = 37703;
INSERT INTO `item_template` (`entry`, `class`, `subclass`, `SoundOverrideSubclass`, `name`, `displayid`, `Quality`, `Flags`, `FlagsExtra`, `Unk430_1`, `Unk430_2`, `BuyCount`, `BuyPrice`, `SellPrice`, `InventoryType`, `AllowableClass`, `AllowableRace`, `ItemLevel`, `RequiredLevel`, `RequiredSkill`, `RequiredSkillRank`, `requiredspell`, `requiredhonorrank`, `RequiredCityRank`, `RequiredReputationFaction`, `RequiredReputationRank`, `maxcount`, `stackable`, `ContainerSlots`, `stat_type1`, `stat_value1`, `stat_unk1_1`, `stat_unk2_1`, `stat_type2`, `stat_value2`, `stat_unk1_2`, `stat_unk2_2`, `stat_type3`, `stat_value3`, `stat_unk1_3`, `stat_unk2_3`, `stat_type4`, `stat_value4`, `stat_unk1_4`, `stat_unk2_4`, `stat_type5`, `stat_value5`, `stat_unk1_5`, `stat_unk2_5`, `stat_type6`, `stat_value6`, `stat_unk1_6`, `stat_unk2_6`, `stat_type7`, `stat_value7`, `stat_unk1_7`, `stat_unk2_7`, `stat_type8`, `stat_value8`, `stat_unk1_8`, `stat_unk2_8`, `stat_type9`, `stat_value9`, `stat_unk1_9`, `stat_unk2_9`, `stat_type10`, `stat_value10`, `stat_unk1_10`, `stat_unk2_10`, `ScalingStatDistribution`, `DamageType`, `delay`, `RangedModRange`, `spellid_1`, `spelltrigger_1`, `spellcharges_1`, `spellcooldown_1`, `spellcategory_1`, `spellcategorycooldown_1`, `spellid_2`, `spelltrigger_2`, `spellcharges_2`, `spellcooldown_2`, `spellcategory_2`, `spellcategorycooldown_2`, `spellid_3`, `spelltrigger_3`, `spellcharges_3`, `spellcooldown_3`, `spellcategory_3`, `spellcategorycooldown_3`, `spellid_4`, `spelltrigger_4`, `spellcharges_4`, `spellcooldown_4`, `spellcategory_4`, `spellcategorycooldown_4`, `spellid_5`, `spelltrigger_5`, `spellcharges_5`, `spellcooldown_5`, `spellcategory_5`, `spellcategorycooldown_5`, `bonding`, `description`, `PageText`, `LanguageID`, `PageMaterial`, `startquest`, `lockid`, `Material`, `sheath`, `RandomProperty`, `RandomSuffix`, `itemset`, `MaxDurability`, `area`, `Map`, `BagFamily`, `TotemCategory`, `socketColor_1`, `socketContent_1`, `socketColor_2`, `socketContent_2`, `socketColor_3`, `socketContent_3`, `socketBonus`, `GemProperties`, `ArmorDamageModifier`, `duration`, `ItemLimitCategory`, `HolidayId`, `StatScalingFactor`, `CurrencySubstitutionId`, `CurrencySubstitutionCount`, `flagsCustom`, `WDBVerified`) VALUES 
('37703', '7', '10', '-1', 'Crystallized Shadow', '55243', '1', '1024', '8192', '0.9957', '1', '1', '2400', '600', '0', '-1', '-1', '75', '0', '0', '0', '0', '0', '0', '0', '0', '0', '10', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '49246', '0', '-1', '500', '0', '-1', '0', '0', '0', '-1', '0', '-1', '0', '0', '0', '-1', '0', '-1', '0', '0', '0', '-1', '0', '-1', '0', '0', '0', '-1', '0', '-1', '0', '', '0', '0', '0', '0', '0', '4', '0', '0', '0', '0', '0', '0', '0', '1224', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '16135'); 


/* Step 2.6 GObject > Quest Items Data: ItemID - Quality - SellPrice - ItemLevel - RequiredLevel - Name */
/*
36912  1  WAIHT_COMMON      0g 15s 0c  75  0  Saronite Ore
37701  1  WAIHT_COMMON      0g 6s 0c  75  0  Crystallized Earth
37703  1  WAIHT_COMMON      0g 6s 0c  75  0  Crystallized Shadow
*/



/* Step 2.2 Table  `gameobject_questrelation */
DELETE FROM  `gameobject_questrelation` WHERE `id`= @ENTRY;
/* Step 2.2 Table  `gameobject_involvedrelation */
DELETE FROM  `gameobject_involvedrelation` WHERE `id`= @ENTRY;
/*## Step 3: GObjects Conditions #####################################*/

/*## Step 4: GObjects Needs to Work ##################################*/


