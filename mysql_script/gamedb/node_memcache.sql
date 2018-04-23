/*
MySQL Data Transfer
Source Host: localhost
Source Database: node_memcache
Target Host: localhost
Target Database: node_memcache
Date: 2018/2/27 23:38:10
*/

SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Table structure for config_options
-- ----------------------------
DROP TABLE IF EXISTS `config_options`;
CREATE TABLE `config_options` (
  `name` varchar(50) COLLATE utf8_unicode_ci NOT NULL,
  `value` varchar(50) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Table structure for containers
-- ----------------------------
DROP TABLE IF EXISTS `containers`;
CREATE TABLE `containers` (
  `name` varchar(50) COLLATE utf8_unicode_ci NOT NULL,
  `db_schema` varchar(250) COLLATE utf8_unicode_ci NOT NULL,
  `db_table` varchar(250) COLLATE utf8_unicode_ci NOT NULL,
  `key_columns` varchar(250) COLLATE utf8_unicode_ci DEFAULT NULL,
  `value_columns` varchar(250) COLLATE utf8_unicode_ci DEFAULT NULL,
  `flags` varchar(250) COLLATE utf8_unicode_ci DEFAULT NULL,
  `cas_column` varchar(250) COLLATE utf8_unicode_ci DEFAULT NULL,
  `expire_time_column` varchar(250) COLLATE utf8_unicode_ci DEFAULT NULL,
  `unique_idx_name_on_key` varchar(250) COLLATE utf8_unicode_ci DEFAULT '',
  PRIMARY KEY (`name`),
  KEY `db_schema` (`db_schema`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Table structure for db_server
-- ----------------------------
DROP TABLE IF EXISTS `db_server`;
CREATE TABLE `db_server` (
  `db_id` smallint(6) NOT NULL,
  `host` varchar(52) COLLATE utf8_unicode_ci NOT NULL,
  `port` smallint(6) NOT NULL,
  `user` varchar(52) COLLATE utf8_unicode_ci NOT NULL,
  `psw` varchar(52) COLLATE utf8_unicode_ci NOT NULL,
  `db_name` varchar(52) COLLATE utf8_unicode_ci NOT NULL,
  `db_connection` smallint(6) NOT NULL,
  PRIMARY KEY (`db_id`),
  KEY `db_name` (`db_name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Procedure structure for SP_COLUMNS_TYPE
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_COLUMNS_TYPE`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `SP_COLUMNS_TYPE`(IN szTableSchema VARCHAR(64), IN szTableName VARCHAR(64))
BEGIN
	SELECT `COLUMN_NAME`,`DATA_TYPE` FROM `information_schema`.`COLUMNS` WHERE `TABLE_SCHEMA` = szTableSchema COLLATE utf8_general_ci  AND `TABLE_NAME` = szTableName COLLATE utf8_general_ci;
END;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for SP_SELECT_CONFIG_OPTIONS
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_SELECT_CONFIG_OPTIONS`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `SP_SELECT_CONFIG_OPTIONS`()
BEGIN
	SELECT `name`,`value` FROM `config_options`;
END;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for SP_SELECT_CONTAINERS
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_SELECT_CONTAINERS`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `SP_SELECT_CONTAINERS`(IN szSchemas VARCHAR(256))
BEGIN
	DECLARE parm VARCHAR(512);
	DECLARE sql1 VARCHAR(1024);
	DECLARE startIdx INT;
	DECLARE endCount INT;
	DECLARE len INT;

	SET startIdx = 1;
	SET len = LENGTH(szSchemas);
	SET endCount = len;

	IF len > 0 THEN
		IF SUBSTRING(szSchemas,startIdx,1) = ',' THEN
			SET startIdx = startIdx + 1;
			SET endCount = endCount - 1;
		END IF;
		IF SUBSTRING(szSchemas,len,1) = ',' THEN
			SET endCount = endCount - 1;
		END IF;

		SET parm = SUBSTRING(szSchemas, startIdx, endCount);
		SET parm = REPLACE (parm, ',' , "' OR `db_schema` = '" );

		SET sql1 = concat("SELECT `name`,`db_schema`,`db_table`,`key_columns`,`value_columns`,`cas_column`,`unique_idx_name_on_key`,`flags` FROM `containers` WHERE `db_schema` = '",parm, "' ;" );

		SET @SQUERY = sql1;
   		PREPARE STMT FROM @SQUERY;
		EXECUTE STMT;
	END IF;
END;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for SP_SELECT_DBSERVERS
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_SELECT_DBSERVERS`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `SP_SELECT_DBSERVERS`(IN szSchemas VARCHAR(256))
BEGIN
	#Routine body goes here...
	DECLARE parm VARCHAR(512);
	DECLARE sql1 VARCHAR(1024);
	DECLARE startIdx INT;
	DECLARE endCount INT;
	DECLARE len INT;

	SET startIdx = 1;
	SET len = LENGTH(szSchemas);
	SET endCount = len;

	IF len > 0 THEN
		IF SUBSTRING(szSchemas,startIdx,1) = ',' THEN
			SET startIdx = startIdx + 1;
			SET endCount = endCount - 1;
		END IF;
		IF SUBSTRING(szSchemas,len,1) = ',' THEN
			SET endCount = endCount - 1;
		END IF;

		SET parm = SUBSTRING(szSchemas, startIdx, endCount);
		SET parm = REPLACE (parm, ',' , "' OR `db_name` = '" );

		SET sql1 = concat("SELECT `db_id`,`host`,`port`,`user`,`psw`,`db_name`,`db_connection` FROM `db_server` WHERE `db_name` = '",parm, "' ;" );

		SET @SQUERY = sql1;
   		PREPARE STMT FROM @SQUERY;
		EXECUTE STMT;
	END IF;
END;;
DELIMITER ;

-- ----------------------------
-- Records 
-- ----------------------------
INSERT INTO `config_options` VALUES ('separator', '|');
INSERT INTO `config_options` VALUES ('table_map_delimiter', '.');
INSERT INTO `containers` VALUES ('game_bal_user_id', 'wonderful', 'character', 'user_id', null, null, null, null, 'PRIMARY');
INSERT INTO `containers` VALUES ('game_bal_serv_id', 'wonderful', 'server_bal', 'server_id', null, null, null, null, 'PRIMARY');
INSERT INTO `containers` VALUES ('game_dir_serv_id', 'wonderful', 'server_dir', 'server_id', null, null, null, null, 'PRIMARY');
INSERT INTO `containers` VALUES ('game_character', 'wonderful', 'character', 'user_id', 'account_id|name|level|exp|gem|coin|create_time|offline_time|status', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `db_server` VALUES ('2', '127.0.0.1', '3306', 'root', '08155', 'wonderful', '3');
