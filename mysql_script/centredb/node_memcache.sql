/*
 Navicat Premium Data Transfer

 Source Server         : localhost
 Source Server Type    : MySQL
 Source Server Version : 50715
 Source Host           : localhost:3306
 Source Schema         : node_memcache

 Target Server Type    : MySQL
 Target Server Version : 50715
 File Encoding         : 65001

 Date: 12/12/2020 18:06:06
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for config_options
-- ----------------------------
DROP TABLE IF EXISTS `config_options`;
CREATE TABLE `config_options`  (
  `name` varchar(50) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `value` varchar(50) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  PRIMARY KEY (`name`) USING BTREE
) ENGINE = MyISAM CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of config_options
-- ----------------------------
INSERT INTO `config_options` VALUES ('separator', '|');
INSERT INTO `config_options` VALUES ('table_map_delimiter', '.');

-- ----------------------------
-- Table structure for containers
-- ----------------------------
DROP TABLE IF EXISTS `containers`;
CREATE TABLE `containers`  (
  `name` varchar(50) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `db_schema` varchar(250) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `db_table` varchar(250) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `key_columns` varchar(250) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `value_columns` varchar(250) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `flags` varchar(250) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `cas_column` varchar(250) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `expire_time_column` varchar(250) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `unique_idx_name_on_key` varchar(250) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL DEFAULT '',
  PRIMARY KEY (`name`) USING BTREE,
  INDEX `db_schema`(`db_schema`) USING BTREE
) ENGINE = MyISAM CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Records of containers
-- ----------------------------
INSERT INTO `containers` VALUES ('node_state_register', 'node_control_centre', 'SP_STATE_REGISTER', 'int|varchar(128)|varchar(250)|int|int|text|varchar(260)|varchar(32)', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('node_state_refresh', 'node_control_centre', 'SP_STATE_REFRESH', 'int|varchar(250)|int|int|text|varchar(32)', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('node_state_unregister', 'node_control_centre', 'SP_STATE_UNREGISTER', 'int', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('node_route_create', 'node_control_centre', 'SP_ROUTE_CREATE_TABLE', 'varchar(250)', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('node_route_drop', 'node_control_centre', 'SP_ROUTE_DROP_TABLE', 'varchar(250)', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('node_route_insert', 'node_control_centre', 'SP_ROUTE_INSERT_RECORD', 'varchar(250)|bigint(20)|int(11)', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('node_route_get', 'node_control_centre', 'SP_ROUTE_GET_RECORD', 'varchar(250)|bigint(20)', 'smallint', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('node_route_remove', 'node_control_centre', 'SP_ROUTE_REMOVE_RECORD', 'varchar(250)|bigint(20)', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('node_user_create', 'node_control_centre', 'SP_USER_CREATE', 'bigint|smallint|int|int', 'int|int|smallint|varchar(21)|int|int', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('node_user_check', 'node_control_centre', 'SP_USER_CHECK', 'int|smallint', 'varchar(21)|int|int|bigint|int', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('node_bal_user_id', 'node_control_centre', 'user', 'index|hash_key', NULL, NULL, NULL, NULL, 'PRIMARY');
INSERT INTO `containers` VALUES ('node_bal_serv_id', 'node_control_centre', 'server_bal', 'server_id', NULL, NULL, NULL, NULL, 'PRIMARY');
INSERT INTO `containers` VALUES ('node_dir_serv_id', 'node_control_centre', 'server_dir', 'server_id', NULL, NULL, NULL, NULL, 'PRIMARY');
INSERT INTO `containers` VALUES ('node_user_get', 'node_control_centre', 'SP_USER_GET', 'bigint', 'int|smallint|varchar(21)|int|int|int', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('node_user_update', 'node_control_centre', 'SP_USER_UPDATE', 'int|smallint|int|int|int', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('node_user_delete', 'node_control_centre', 'SP_USER_DELETE', 'int|smallint', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('node_seize_server', 'node_control_centre', 'SP_SEIZE_SERVER', 'int|smallint|varchar|smallint|tinyint', 'varchar(32)|smallint|int', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('node_free_server', 'node_control_centre', 'SP_FREE_SERVER', 'int|smallint|tinyint', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('node_admin_check', 'node_control_centre', 'SP_ADMIN_CHECK', 'varchar', 'varchar|varchar', NULL, NULL, NULL, NULL);

-- ----------------------------
-- Table structure for db_server
-- ----------------------------
DROP TABLE IF EXISTS `db_server`;
CREATE TABLE `db_server`  (
  `db_id` smallint(6) NOT NULL,
  `host` varchar(52) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `port` smallint(6) NOT NULL,
  `user` varchar(52) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `psw` varchar(52) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `db_name` varchar(52) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `db_connection` smallint(6) NOT NULL,
  PRIMARY KEY (`db_id`) USING BTREE,
  INDEX `db_name`(`db_name`) USING BTREE
) ENGINE = MyISAM CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of db_server
-- ----------------------------
INSERT INTO `db_server` VALUES (1, '127.0.0.1', 3306, 'root', 'xmxy08155', 'node_control_centre', 1);

-- ----------------------------
-- Procedure structure for SP_COLUMNS_TYPE
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_COLUMNS_TYPE`;
delimiter ;;
CREATE PROCEDURE `SP_COLUMNS_TYPE`(IN szTableSchema VARCHAR(64), IN szTableName VARCHAR(64))
BEGIN
	SELECT `COLUMN_NAME`,`DATA_TYPE` FROM `information_schema`.`COLUMNS` WHERE `TABLE_SCHEMA` = szTableSchema COLLATE utf8_general_ci  AND `TABLE_NAME` = szTableName COLLATE utf8_general_ci;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_SELECT_CONFIG_OPTIONS
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_SELECT_CONFIG_OPTIONS`;
delimiter ;;
CREATE PROCEDURE `SP_SELECT_CONFIG_OPTIONS`()
BEGIN
	SELECT `name`,`value` FROM `config_options`;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_SELECT_CONTAINERS
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_SELECT_CONTAINERS`;
delimiter ;;
CREATE PROCEDURE `SP_SELECT_CONTAINERS`(IN szSchemas VARCHAR(256))
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
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_SELECT_DBSERVERS
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_SELECT_DBSERVERS`;
delimiter ;;
CREATE PROCEDURE `SP_SELECT_DBSERVERS`(IN szSchemas VARCHAR(256))
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
END
;;
delimiter ;

SET FOREIGN_KEY_CHECKS = 1;
