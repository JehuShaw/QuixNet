/*
 Navicat Premium Data Transfer

 Source Server         : localhost
 Source Server Type    : MySQL
 Source Server Version : 50715
 Source Host           : localhost:3306
 Source Schema         : node_control_centre

 Target Server Type    : MySQL
 Target Server Version : 50715
 File Encoding         : 65001

 Date: 12/12/2020 18:05:56
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for admin
-- ----------------------------
DROP TABLE IF EXISTS `admin`;
CREATE TABLE `admin`  (
  `index` int(21) UNSIGNED NOT NULL AUTO_INCREMENT,
  `name` varchar(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `password` varchar(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `web_key` varchar(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`index`) USING BTREE,
  UNIQUE INDEX `name`(`name`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for node_state
-- ----------------------------
DROP TABLE IF EXISTS `node_state`;
CREATE TABLE `node_state`  (
  `server_id` int(11) UNSIGNED NOT NULL,
  `server_ip` varchar(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `server_name` varchar(250) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `server_load` int(11) NOT NULL,
  `server_status` int(11) NOT NULL,
  `server_state` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `project_name` varchar(250) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `update_time` varchar(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `mc_flags` int(11) NULL DEFAULT NULL,
  `mc_cas` bigint(20) NULL DEFAULT NULL,
  `mc_expire` int(11) NULL DEFAULT NULL,
  PRIMARY KEY (`server_id`) USING BTREE,
  INDEX `server_name`(`server_name`) USING BTREE
) ENGINE = MyISAM CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for server_bal
-- ----------------------------
DROP TABLE IF EXISTS `server_bal`;
CREATE TABLE `server_bal`  (
  `server_id` int(11) UNSIGNED NOT NULL,
  PRIMARY KEY (`server_id`) USING BTREE
) ENGINE = MyISAM CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = FIXED;

-- ----------------------------
-- Table structure for server_dir
-- ----------------------------
DROP TABLE IF EXISTS `server_dir`;
CREATE TABLE `server_dir`  (
  `server_id` int(11) UNSIGNED NOT NULL,
  PRIMARY KEY (`server_id`) USING BTREE
) ENGINE = MyISAM CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = FIXED;

-- ----------------------------
-- Table structure for user
-- ----------------------------
DROP TABLE IF EXISTS `user`;
CREATE TABLE `user`  (
  `index` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `hash_key` int(11) UNSIGNED NOT NULL,
  `account` bigint(20) UNSIGNED NOT NULL,
  `creation_time` varchar(19) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `server_region` int(10) UNSIGNED ZEROFILL NOT NULL COMMENT '该字段类型必须大于16 bit， 值是16bit内。',
  `login_count` int(10) UNSIGNED ZEROFILL NOT NULL,
  `map_id` int(11) UNSIGNED NOT NULL,
  `server_ip` varchar(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `server_id` int(11) UNSIGNED ZEROFILL NOT NULL,
  `online` tinyint(1) UNSIGNED ZEROFILL NOT NULL COMMENT '1 在线 0 离线',
  `mc_flags` int(11) NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(11) NULL DEFAULT NULL,
  PRIMARY KEY (`index`) USING BTREE,
  UNIQUE INDEX `userId`(`index`, `hash_key`) USING BTREE,
  INDEX `account`(`account`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Procedure structure for SP_ADMIN_CHECK
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_ADMIN_CHECK`;
delimiter ;;
CREATE PROCEDURE `SP_ADMIN_CHECK`(IN `szName` varchar(125))
BEGIN
	#Routine body goes here...
	SELECT `password`,`web_key` FROM `admin` WHERE `name` = szName; 
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_FREE_SERVER
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_FREE_SERVER`;
delimiter ;;
CREATE PROCEDURE `SP_FREE_SERVER`(IN nIndex INT UNSIGNED,IN nHashKey INT UNSIGNED, IN bLogout TINYINT UNSIGNED)
BEGIN
	#Routine body goes here...
	IF 0 = bLogout THEN
		UPDATE `user` SET `server_ip` = "", `server_id` = 0 WHERE `index` = nIndex AND `hash_key` = nHashKey AND `online` <> 1;
	ELSE
		UPDATE `user` SET `server_ip` = "", `server_id` = 0, `online` = 0 WHERE `index` = nIndex AND `hash_key` = nHashKey;
	END IF;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_ROUTE_CREATE_TABLE
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_ROUTE_CREATE_TABLE`;
delimiter ;;
CREATE PROCEDURE `SP_ROUTE_CREATE_TABLE`(IN szServerName VARCHAR(250))
BEGIN
	DECLARE table_name VARCHAR(250);
	DECLARE sql1 VARCHAR(1024);
	SET table_name = concat("route_",LCASE (szServerName) );

        SET sql1 = concat("CREATE TABLE IF NOT EXISTS `" , table_name,"` (`user_id` bigint(20) unsigned NOT NULL, `server_id` int(11) NOT NULL,  PRIMARY KEY (`user_id`)) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;");
	SET @SQUERY = sql1;
   	PREPARE STMT FROM @SQUERY;
	EXECUTE STMT;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_ROUTE_DROP_TABLE
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_ROUTE_DROP_TABLE`;
delimiter ;;
CREATE PROCEDURE `SP_ROUTE_DROP_TABLE`(IN szServerName VARCHAR(250))
BEGIN
	DECLARE table_name VARCHAR(250);
	DECLARE sql1 VARCHAR(300);
	SET table_name = concat("route_",LCASE (szServerName) );
	SET sql1 = concat("DROP TABLE IF EXISTS `" , table_name,"`;");
	SET @SQUERY = sql1;
   	PREPARE STMT FROM @SQUERY;
	EXECUTE STMT;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_ROUTE_GET_RECORD
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_ROUTE_GET_RECORD`;
delimiter ;;
CREATE PROCEDURE `SP_ROUTE_GET_RECORD`(IN szServerName VARCHAR(250), IN nUserId bigint(20))
BEGIN
	DECLARE table_name VARCHAR(250);
	DECLARE sql1 VARCHAR(512);
	SET table_name = concat("route_",LCASE (szServerName) );
	SET sql1 = concat("SELECT `server_id` FROM `",table_name,"` WHERE `user_id` = ", nUserId,";");
	SET @SQUERY = sql1;
   	PREPARE STMT FROM @SQUERY;
	EXECUTE STMT;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_ROUTE_INSERT_RECORD
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_ROUTE_INSERT_RECORD`;
delimiter ;;
CREATE PROCEDURE `SP_ROUTE_INSERT_RECORD`(IN szServerName VARCHAR(250), IN nUserId bigint(20), IN nServerId int(11))
BEGIN
	DECLARE table_name VARCHAR(250);
	DECLARE sql1 VARCHAR(512);
	SET table_name = concat("route_",LCASE (szServerName) );
	SET sql1 = concat("REPLACE INTO `",table_name,"` VALUES(",nUserId,",",nServerId,");");
	SET @SQUERY = sql1;
   	PREPARE STMT FROM @SQUERY;
	EXECUTE STMT;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_ROUTE_REMOVE_RECORD
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_ROUTE_REMOVE_RECORD`;
delimiter ;;
CREATE PROCEDURE `SP_ROUTE_REMOVE_RECORD`(IN szServerName VARCHAR(250), IN nUserId bigint(20))
BEGIN
	DECLARE table_name VARCHAR(250);
	DECLARE sql1 VARCHAR(512);
	SET table_name = concat("route_",LCASE (szServerName) );
	SET sql1 = concat("DELETE FROM `",table_name,"` WHERE `user_id` = ", nUserId,";");
	SET @SQUERY = sql1;
   	PREPARE STMT FROM @SQUERY;
	EXECUTE STMT;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_SEIZE_SERVER
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_SEIZE_SERVER`;
delimiter ;;
CREATE PROCEDURE `SP_SEIZE_SERVER`(IN nIndex INT UNSIGNED,IN nHashKey INT UNSIGNED, IN szServerIp varchar(32), IN nServerId INT UNSIGNED, IN bLogin TINYINT UNSIGNED)
BEGIN
	#Routine body goes here...
	SET @ip := "";
	SET @id := 0;
	SET @map := 0;
	IF 1 = bLogin THEN
		UPDATE `user` SET `server_ip` = (CASE WHEN `server_ip` = "" THEN szServerIp ELSE (SELECT @ip := `server_ip`) END), `server_id` = (CASE WHEN `server_id` = 0 THEN  nServerId ELSE (SELECT @id := `server_id`) END), `online` = bLogin WHERE `index` = nIndex AND `hash_key` = nHashKey AND (SELECT @map := `map_id`) ;
	ELSE
		UPDATE `user` SET `server_ip` = (CASE WHEN `server_ip` = "" THEN szServerIp ELSE (SELECT @ip := `server_ip`) END), `server_id` = (CASE WHEN `server_id` = 0 THEN  nServerId ELSE (SELECT @id := `server_id`) END) WHERE `index` = nIndex AND `hash_key` = nHashKey AND (SELECT @map := `map_id`) ;
	END IF;
	SELECT @ip,@id,@map;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_STATE_REFRESH
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_STATE_REFRESH`;
delimiter ;;
CREATE PROCEDURE `SP_STATE_REFRESH`(IN nServerId INT UNSIGNED, IN szServerName VARCHAR(250), IN nSeverLoad INT UNSIGNED,IN nServerStatus INT UNSIGNED, IN szServerState TEXT, IN szUpdateTime VARCHAR(32))
BEGIN
	DECLARE sql1 VARCHAR(2048);
        SET sql1 = concat("UPDATE `node_control_centre`.`node_state` SET `server_name` = '", szServerName ,
	"', `server_load` = '", nSeverLoad, "', `server_status` = '",nServerStatus,"', `server_state` = '",szServerState,
	"', `update_time` = '",  szUpdateTime,"' WHERE `server_id` = ",nServerId," ;");
	SET @SQUERY = sql1;
   	PREPARE STMT FROM @SQUERY;
	EXECUTE STMT;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_STATE_REGISTER
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_STATE_REGISTER`;
delimiter ;;
CREATE PROCEDURE `SP_STATE_REGISTER`(IN nServerId INT UNSIGNED, IN szServerIp VARCHAR(128), IN szServerName VARCHAR(250), IN nSeverLoad INT UNSIGNED,IN nServerStatus INT UNSIGNED, IN szServerState TEXT, IN szProjectName VARCHAR(250), IN szUpdateTime VARCHAR(32))
BEGIN
	DECLARE sql1 VARCHAR(1024);
        SET sql1 = concat("REPLACE INTO `node_control_centre`.`node_state` VALUES(", nServerId,",'",szServerIp,
	"','",szServerName ,"',",nSeverLoad ,",",nServerStatus,",'",szServerState,"', '",szProjectName,"','" , szUpdateTime,"',0,0,0);");
	SET @SQUERY = sql1;
   	PREPARE STMT FROM @SQUERY;
	EXECUTE STMT;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_STATE_UNREGISTER
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_STATE_UNREGISTER`;
delimiter ;;
CREATE PROCEDURE `SP_STATE_UNREGISTER`(IN nServerId INT UNSIGNED)
BEGIN
	DECLARE sql1 VARCHAR(256);
	SET sql1 = concat("DELETE FROM `node_control_centre`.`node_state`  WHERE `server_id` = ",nServerId,"  ;" );
	SET @SQUERY = sql1;
  	PREPARE STMT FROM @SQUERY;
	EXECUTE STMT;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_USER_CHECK
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_USER_CHECK`;
delimiter ;;
CREATE PROCEDURE `SP_USER_CHECK`(IN nIndex INT UNSIGNED,IN nHashKey INT UNSIGNED)
BEGIN
SELECT `creation_time`,`server_region`,`login_count`,`account`,`map_id`  FROM `user` WHERE `index` =  nIndex AND `hash_key` = nHashKey;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_USER_CREATE
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_USER_CREATE`;
delimiter ;;
CREATE PROCEDURE `SP_USER_CREATE`(IN nAccount BIGINT UNSIGNED, IN nHashKey INT UNSIGNED, IN nMapId INT  UNSIGNED, IN nMaxSize INT  UNSIGNED)
BEGIN
	Set @curSize = (SELECT count(`index`) FROM `user`  WHERE `account` = nAccount);

	if @curSize < nMaxSize || nMaxSize = 0 then
		INSERT INTO `user` (`hash_key`,`account`,`creation_time`,`server_region`,`login_count`,`map_id`,`server_ip`,`server_id`,`online`,`mc_cas`) VALUES(nHashKey,nAccount,NOW(),0,0,nMapId,'',0,0,0);
		SELECT @curSize, `index`,`hash_key`,`creation_time`,`server_region`,`login_count`  FROM `user` WHERE `index` = (SELECT LAST_INSERT_ID());
	else
		SELECT @curSize, 0, 0, '', 0, 0;
	end if;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_USER_DELETE
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_USER_DELETE`;
delimiter ;;
CREATE PROCEDURE `SP_USER_DELETE`(IN nIndex INT UNSIGNED,IN nHashKey INT UNSIGNED)
BEGIN
	DELETE FROM `user`  WHERE `index` =  nIndex AND `hash_key` = nHashKey;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_USER_GET
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_USER_GET`;
delimiter ;;
CREATE PROCEDURE `SP_USER_GET`(IN nAccount BIGINT UNSIGNED)
BEGIN
SELECT `index`,`hash_key`,`creation_time`,`server_region`,`login_count`,`map_id` FROM `user` WHERE `account` = nAccount;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_USER_UPDATE
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_USER_UPDATE`;
delimiter ;;
CREATE PROCEDURE `SP_USER_UPDATE`(IN nIndex INT UNSIGNED,IN nHashKey INT UNSIGNED,IN bLogin TINYINT UNSIGNED,IN nMapId INT UNSIGNED,IN nServerRegion INT UNSIGNED)
BEGIN
	IF 1 = bLogin THEN
		UPDATE `user` SET `login_count` = `login_count` + 1, `server_region` = nServerRegion WHERE `index` =  nIndex AND `hash_key` = nHashKey;
	ELSE
		UPDATE `user` SET `map_id` = nMapId WHERE `index` =  nIndex AND `hash_key` = nHashKey;
	END IF;
END
;;
delimiter ;

SET FOREIGN_KEY_CHECKS = 1;
