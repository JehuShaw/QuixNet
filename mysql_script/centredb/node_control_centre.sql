/*
Navicat MySQL Data Transfer

Source Server         : localhost
Source Server Version : 50637
Source Host           : localhost:3306
Source Database       : node_control_centre

Target Server Type    : MYSQL
Target Server Version : 50637
File Encoding         : 65001

Date: 2018-02-11 18:57:25
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `node_state`
-- ----------------------------
DROP TABLE IF EXISTS `node_state`;
CREATE TABLE `node_state` (
  `server_id` int(11) NOT NULL,
  `server_ip` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `server_name` varchar(250) COLLATE utf8_unicode_ci NOT NULL,
  `server_load` int(11) NOT NULL,
  `server_status` int(11) NOT NULL,
  `server_state` text COLLATE utf8_unicode_ci NOT NULL,
  `project_name` varchar(250) COLLATE utf8_unicode_ci DEFAULT NULL,
  `update_time` varchar(32) COLLATE utf8_unicode_ci DEFAULT NULL,
  `mc_flags` int(11) DEFAULT NULL,
  `mc_cas` bigint(20) DEFAULT NULL,
  `mc_expire` int(11) DEFAULT NULL,
  PRIMARY KEY (`server_id`),
  KEY `server_name` (`server_name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Records of node_state
-- ----------------------------

-- ----------------------------
-- Table structure for `server_bal`
-- ----------------------------
DROP TABLE IF EXISTS `server_bal`;
CREATE TABLE `server_bal` (
  `server_id` smallint(5) unsigned NOT NULL,
  PRIMARY KEY (`server_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Records of server_bal
-- ----------------------------

-- ----------------------------
-- Table structure for `server_dir`
-- ----------------------------
DROP TABLE IF EXISTS `server_dir`;
CREATE TABLE `server_dir` (
  `server_id` smallint(5) unsigned NOT NULL,
  PRIMARY KEY (`server_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Records of server_dir
-- ----------------------------

-- ----------------------------
-- Table structure for `user`
-- ----------------------------
DROP TABLE IF EXISTS `user`;
CREATE TABLE `user` (
  `index` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `server_id` smallint(6) unsigned NOT NULL,
  `account` bigint(20) unsigned NOT NULL,
  `creation_time` varchar(21) COLLATE utf8_unicode_ci NOT NULL,
  `server_region` int(10) unsigned zerofill NOT NULL COMMENT '该字段类型必须大于16 bit， 值是16bit内。',
  `mc_flags` int(11) DEFAULT NULL,
  `mc_cas` bigint(20) unsigned zerofill NOT NULL,
  `mc_expire` int(11) DEFAULT NULL,
  PRIMARY KEY (`account`,`index`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Records of user
-- ----------------------------

-- ----------------------------
-- Procedure structure for `SP_ROUTE_CREATE_TABLE`
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_ROUTE_CREATE_TABLE`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `SP_ROUTE_CREATE_TABLE`(IN server_name VARCHAR(250))
BEGIN
	DECLARE table_name VARCHAR(250);
	DECLARE sql1 VARCHAR(1024);
	SET table_name = concat("route_",LCASE (server_name) );

        SET sql1 = concat("CREATE TABLE IF NOT EXISTS `" , table_name,"` (`user_id` bigint(20) unsigned NOT NULL, `server_id` int(11) NOT NULL,  PRIMARY KEY (`user_id`)) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;");
	SET @SQUERY = sql1;
   	PREPARE STMT FROM @SQUERY;
	EXECUTE STMT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `SP_ROUTE_DROP_TABLE`
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_ROUTE_DROP_TABLE`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `SP_ROUTE_DROP_TABLE`(IN server_name VARCHAR(250))
BEGIN
	DECLARE table_name VARCHAR(250);
	DECLARE sql1 VARCHAR(300);
	SET table_name = concat("route_",LCASE (server_name) );

        SET sql1 = concat("DROP TABLE IF EXISTS `" , table_name,"`;");
	SET @SQUERY = sql1;
   	PREPARE STMT FROM @SQUERY;
	EXECUTE STMT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `SP_ROUTE_GET_RECORD`
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_ROUTE_GET_RECORD`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `SP_ROUTE_GET_RECORD`(IN server_name VARCHAR(250), IN user_id bigint(20))
BEGIN
	DECLARE table_name VARCHAR(250);
	DECLARE sql1 VARCHAR(512);
	SET table_name = concat("route_",LCASE (server_name) );

        SET sql1 = concat("SELECT `server_id` FROM `",table_name,"` WHERE `user_id` = ", user_id,";");
	SET @SQUERY = sql1;
   	PREPARE STMT FROM @SQUERY;
	EXECUTE STMT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `SP_ROUTE_INSERT_RECORD`
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_ROUTE_INSERT_RECORD`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `SP_ROUTE_INSERT_RECORD`(IN server_name VARCHAR(250), IN user_id bigint(20), IN server_id int(11))
BEGIN
	DECLARE table_name VARCHAR(250);
	DECLARE sql1 VARCHAR(512);
	SET table_name = concat("route_",LCASE (server_name) );

        SET sql1 = concat("REPLACE INTO `",table_name,"` VALUES(",user_id,",",server_id,");");
	SET @SQUERY = sql1;
   	PREPARE STMT FROM @SQUERY;
	EXECUTE STMT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `SP_ROUTE_REMOVE_RECORD`
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_ROUTE_REMOVE_RECORD`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `SP_ROUTE_REMOVE_RECORD`(IN server_name VARCHAR(250), IN user_id bigint(20))
BEGIN
	DECLARE table_name VARCHAR(250);
	DECLARE sql1 VARCHAR(512);
	SET table_name = concat("route_",LCASE (server_name) );

        SET sql1 = concat("DELETE FROM `",table_name,"` WHERE `user_id` = ", user_id,";");
	SET @SQUERY = sql1;
   	PREPARE STMT FROM @SQUERY;
	EXECUTE STMT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `SP_STATE_REFRESH`
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_STATE_REFRESH`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `SP_STATE_REFRESH`(IN server_id INT UNSIGNED, IN server_name VARCHAR(250), IN sever_load INT UNSIGNED,IN server_status INT UNSIGNED, IN server_state TEXT, IN update_time VARCHAR(32))
BEGIN
	DECLARE sql1 VARCHAR(2048);
        SET sql1 = concat("UPDATE `node_control_centre`.`node_state` SET `server_name` = '", server_name ,
	"', `server_load` = '", sever_load, "', `server_status` = '",server_status,"', `server_state` = '",server_state,
	"', `update_time` = '",  update_time,"' WHERE `server_id` = ",server_id," ;");
	SET @SQUERY = sql1;
   	PREPARE STMT FROM @SQUERY;
	EXECUTE STMT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `SP_STATE_REGISTER`
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_STATE_REGISTER`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `SP_STATE_REGISTER`(IN server_id INT UNSIGNED, IN server_ip VARCHAR(128), IN server_name VARCHAR(250), IN sever_load INT UNSIGNED,IN server_status INT UNSIGNED, IN server_state TEXT, IN project_name VARCHAR(250), IN update_time VARCHAR(32))
BEGIN
	DECLARE sql1 VARCHAR(1024);
        SET sql1 = concat("REPLACE INTO `node_control_centre`.`node_state` VALUES(", server_id,",'",server_ip,
	"','",server_name ,"',",sever_load ,",",server_status,",'",server_state,"', '",project_name,"','" , update_time,"',0,0,0);");
	SET @SQUERY = sql1;
   	PREPARE STMT FROM @SQUERY;
	EXECUTE STMT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `SP_STATE_UNREGISTER`
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_STATE_UNREGISTER`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `SP_STATE_UNREGISTER`(IN server_id INT UNSIGNED)
BEGIN
	DECLARE sql1 VARCHAR(256);
	SET sql1 = concat("DELETE FROM `node_control_centre`.`node_state`  WHERE `server_id` = ",server_id,"  ;" );
	SET @SQUERY = sql1;
  	PREPARE STMT FROM @SQUERY;
	EXECUTE STMT;
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for `SP_USER_LOGIN`
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_USER_LOGIN`;
DELIMITER ;;
CREATE DEFINER=`root`@`localhost` PROCEDURE `SP_USER_LOGIN`(IN nAccount BIGINT UNSIGNED, IN nServerId SMALLINT UNSIGNED)
BEGIN
	INSERT INTO `user` (`server_id`,`account`,`creation_time`,`server_region`,`mc_cas`) SELECT  nServerId,nAccount,NOW( ),0,0  FROM DUAL WHERE NOT EXISTS (SELECT `index`  FROM `user` WHERE `account` = nAccount)  ;
	SELECT `index`,`server_id`,`creation_time`,`server_region`  FROM `user` WHERE `account` = nAccount;
END
;;
DELIMITER ;
