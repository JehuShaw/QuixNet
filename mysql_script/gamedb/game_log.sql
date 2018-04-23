/*
MySQL Data Transfer
Source Host: localhost
Source Database: game_log
Target Host: localhost
Target Database: game_log
Date: 2015/5/29 13:25:36
*/

SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Procedure structure for SP_CREATE_LOG_TABLE
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_CREATE_LOG_TABLE`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `SP_CREATE_LOG_TABLE`(IN nIndex INT)
BEGIN
	DECLARE sql1 VARCHAR(1124);
        SET sql1 = concat("CREATE TABLE `event_log",nIndex,
	"` (`id`bigint(20) unsigned zerofill NOT NULL,
  	`account` int(11) unsigned zerofill NOT NULL,
  	`log_type` int(11) unsigned zerofill NOT NULL,
  	`action_type` int(11) unsigned zerofill NOT NULL,
  	`time` varchar(21) NOT NULL,
	`sub_type`  int(11) unsigned zerofill NOT NULL,
	`server_id` int(11) unsigned zerofill NOT NULL,
	`player_id` bigint(20) unsigned zerofill NOT NULL,
	`iparam0` bigint(20) zerofill NOT NULL,
	`iparam1` bigint(20) zerofill NOT NULL,
	`iparam2` bigint(20) zerofill NOT NULL,
	`iparam3` bigint(20) zerofill NOT NULL,
	`iparam4` bigint(20) zerofill NOT NULL,
	`iparam5` bigint(20) zerofill NOT NULL,
	`iparam6` bigint(20) zerofill NOT NULL,
	`iparam7` bigint(20) zerofill NOT NULL,
	`iparam8` bigint(20) zerofill NOT NULL,
	`iparam9` bigint(20) zerofill NOT NULL,
	`strparam0` text NOT NULL,
	`strparam1` text NOT NULL,
	`strparam2` text NOT NULL,
	`strparam3` text NOT NULL,
  	PRIMARY KEY (`id`)
	) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;");

	SET @SQUERY = sql1;
   	PREPARE STMT FROM @SQUERY;
	EXECUTE STMT;
END;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for SP_INSERT_LOG
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_INSERT_LOG`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `SP_INSERT_LOG`(IN nIndex INT,IN nId BIGINT UNSIGNED, IN nAccount  INT UNSIGNED, IN nLogType  INT UNSIGNED, IN nActionType INT UNSIGNED,IN nSubType INT UNSIGNED,IN nServerId INT UNSIGNED,IN nPlayerId BIGINT UNSIGNED,IN iParam0 BIGINT,IN iParam1 BIGINT,IN iParam2 BIGINT,IN iParam3 BIGINT,IN iParam4 BIGINT,IN iParam5 BIGINT,IN iParam6 BIGINT,IN iParam7 BIGINT,IN iParam8 BIGINT,IN iParam9 BIGINT,IN strParam0 TEXT,IN strParam1 TEXT,IN strParam2 TEXT,IN strParam3 TEXT)
BEGIN
	DECLARE sql1 text;
        SET sql1 = concat("INSERT INTO `event_log", nIndex, "` (`id`,`account`,`log_type`,`action_type`,`time`"
	",`sub_type`,`server_id`,`player_id`,`iparam0`,`iparam1`,`iparam2`,`iparam3`,`iparam4`,`iparam5`,"
	"`iparam6`,`iparam7`,`iparam8`,`iparam9`,`strparam0`,`strparam1`,`strparam2`,`strparam3`) VALUES (",nId,",",
	nAccount ,",",nLogType ,",",nActionType,",'",NOW(),"',",nSubType,",",nServerId,",",nPlayerId,",",iParam0,",",
	iParam1,",",iParam2,",",iParam3,",",iParam4,",",iParam5,",",iParam6,",",iParam7,",",iParam8,",",iParam9,",'",
	strParam0,"','",strParam1,"','",strParam2,"','",strParam3,"');");
	SET @SQUERY = sql1;
   	PREPARE STMT FROM @SQUERY;
	EXECUTE STMT;
END;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for SP_MAX_LOG_TABLE_NUM
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_MAX_LOG_TABLE_NUM`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `SP_MAX_LOG_TABLE_NUM`()
BEGIN
SELECT 10;
END;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for SP_SHOW_LOG_TABLES
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_SHOW_LOG_TABLES`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `SP_SHOW_LOG_TABLES`()
BEGIN
show tables like 'event_log%';
END;;
DELIMITER ;

-- ----------------------------
-- Records 
-- ----------------------------
