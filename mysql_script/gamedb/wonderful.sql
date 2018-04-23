/*
MySQL Data Transfer
Source Host: localhost
Source Database: wonderful
Target Host: localhost
Target Database: wonderful
Date: 2018/2/27 23:52:51
*/

SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Table structure for character
-- ----------------------------
DROP TABLE IF EXISTS `character`;
CREATE TABLE `character` (
  `user_id` int(10) unsigned NOT NULL,
  `account_id` int(11) DEFAULT NULL,
  `name` varchar(32) COLLATE utf8_unicode_ci DEFAULT NULL,
  `level` int(11) DEFAULT NULL,
  `exp` int(11) DEFAULT NULL,
  `gem` int(11) DEFAULT NULL,
  `coin` int(11) DEFAULT NULL,
  `create_time` varchar(21) COLLATE utf8_unicode_ci DEFAULT NULL,
  `offline_time` varchar(21) COLLATE utf8_unicode_ci DEFAULT NULL,
  `status` int(11) DEFAULT NULL,
  `mc_flags` int(11) DEFAULT NULL,
  `mc_cas` bigint(20) unsigned zerofill NOT NULL,
  `mc_expire` int(11) unsigned zerofill DEFAULT NULL,
  PRIMARY KEY (`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Table structure for server_bal
-- ----------------------------
DROP TABLE IF EXISTS `server_bal`;
CREATE TABLE `server_bal` (
  `server_id` smallint(5) unsigned NOT NULL,
  PRIMARY KEY (`server_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Table structure for server_dir
-- ----------------------------
DROP TABLE IF EXISTS `server_dir`;
CREATE TABLE `server_dir` (
  `server_id` smallint(5) unsigned NOT NULL,
  PRIMARY KEY (`server_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Records 
-- ----------------------------
