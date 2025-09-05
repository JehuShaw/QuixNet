/*
 Navicat Premium Data Transfer

 Source Server         : localhost
 Source Server Type    : MySQL
 Source Server Version : 50715
 Source Host           : localhost:3306
 Source Schema         : xyproject_memcache

 Target Server Type    : MySQL
 Target Server Version : 50715
 File Encoding         : 65001

 Date: 28/12/2020 17:22:45
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
INSERT INTO `containers` VALUES ('game_bal_user_id', 'xyproject', 'character', 'user_id', NULL, NULL, NULL, NULL, 'PRIMARY');
INSERT INTO `containers` VALUES ('game_bal_serv_id', 'xyproject', 'server_bal', 'server_id', NULL, NULL, NULL, NULL, 'PRIMARY');
INSERT INTO `containers` VALUES ('game_dir_serv_id', 'xyproject', 'server_dir', 'server_id', NULL, NULL, NULL, NULL, 'PRIMARY');
INSERT INTO `containers` VALUES ('game_character', 'xyproject', 'character', 'user_id', 'account_id|cfg_id|name|level|exp|gem|coin|create_time|offline_time|status|prologue_process', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_character_pos', 'xyproject', 'character_pos', 'user_id', 'x|y|z|faceX|faceY|faceZ', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_character_quest', 'xyproject', 'character_quest', 'user_id', 'quest_ids', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_character_quest_done', 'xyproject', 'character_quest_done', 'user_id', 'quest_ids', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_quest_main', 'xyproject', 'quest_main', 'user_id|cfg_id', 'state|progress', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_quest_branch', 'xyproject', 'quest_branch', 'user_id|cfg_id', 'state|progress', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_character_quest_acc', 'xyproject', 'character_quest_accessible', 'user_id', 'quest_ids', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_quest_accessible', 'xyproject', 'quest_accessible', 'user_id|cfg_id', 'count|progress', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_quest_event', 'xyproject', 'quest_event', 'user_id|cfg_id', 'count|progress', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_backpack', 'xyproject', 'character_backpack', 'user_id', 'item_count|cell_count|unlock_count|container', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_item', 'xyproject', 'item', 'item_id', 'cfg_id|type|sub_type|stack_num|create_time', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_check_name', 'xyproject', 'character', 'name', NULL, 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_pet', 'xyproject', 'pet', 'pet_id', 'cfg_id|property_id|type|name|quality|level|hp|max_hp|exp|atk|crit|block|hit|dodge|speed|pos|status', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_character_pet_skill', 'xyproject', 'character_pet_skill', 'pet_id', 'skill_ids', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_pet_skill', 'xyproject', 'pet_skill', 'skill_id', 'cfg_id|type|target|fly_speed|active_level|effect_ids|trigger_conds', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_character_pet', 'xyproject', 'character_pet', 'user_id', 'pet_ids', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_trigger', 'xyproject', 'trigger', 'user_id|cfg_id', 'count|progress', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_pet_equip', 'xyproject', 'pet_equip', 'equip_id', 'cfg_id|max_ph|atk|crit|block|hit|dodge|speed', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_character_pet_equip', 'xyproject', 'character_pet_equip', 'pet_id', 'equip_ids', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_character_map', 'xyproject', 'character_map', 'user_id', 'map_ids', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_map_state', 'xyproject', 'map_state', 'user_id|map_id', 'state', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_character_dress', 'xyproject', 'character_dress', 'user_id', 'dress_ids', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_dress', 'xyproject', 'dress', 'dress_id', 'cfg_id|part|quality|property|create_time|duration_time|default_sel|state', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_check_blacklist', 'xyproject', 'SP_CHECK_BLACKLIST', 'bigint', 'bigint', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_chat_private_init', 'xyproject', 'SP_CHAT_PRIVATE_INIT', 'bigint|bigint|int', 'int', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_chat_private_add', 'xyproject', 'SP_CHAT_PRIVATE_ADD', 'bigint|int|varchar|int|int|int|bigint|int|varchar|int|int|int|int|blob|bigint|tinyint|int', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_friend_application', 'xyproject', 'friend_application', 'user_id', 'user_ids', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_match_player', 'xyproject', 'SP_MATCH_PLAYER', 'varchar', 'bigint|int|varchar|int', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_chat_system_add', 'xyproject', 'SP_CHAT_SYSTEM_ADD', 'bigint|int|blob|bigint|int|int', 'bigint|int|text|bigint', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_chat_system_get', 'xyproject', 'SP_CHAT_SYSTEM_GET', 'int|int', 'bigint|int|text|bigint', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_rand_player_list', 'xyproject', 'SP_RAND_PLAYER_LIST', 'int|int|bigint', 'int|bigint|int|varchar|int', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_friend_list', 'xyproject', 'friend_list', 'user_id', 'friend_ids|order_count', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_friend_blacklist', 'xyproject', 'friend_blacklist', 'user_id', 'user_ids', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_friend_detail', 'xyproject', 'friend_detail', 'user_id|friend_id', 'friend_cfg_id|note|order', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_friend_apply_result', 'xyproject', 'friend_apply_result', 'user_id|target_id', 'target_cfg_id|target_name|result', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_friend_result_list', 'xyproject', 'SP_FRIEND_RESULT_LIST', 'bigint', 'varchar|int', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_chat_private_get', 'xyproject', 'SP_CHAT_PRIVATE_GET', 'bigint|bigint|int|int', 'bigint|int|varchar|int|int|int|bigint|int|varchar|int|int|int|int|text|bigint|tinyint', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_chat_private_del', 'xyproject', 'SP_CHAT_PRIVATE_DEL', 'bigint|bigint', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_chat_private_del_all', 'xyproject', 'SP_CHAT_PRIVATE_DEL_ALL', 'bigint|bigint', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_chat_private_list_add', 'xyproject', 'SP_CHAT_PRIVATE_LIST_ADD', 'bigint|bigint|int|varchar|int|int|int|int|text|bigint|int', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_chat_private_list_get', 'xyproject', 'SP_CHAT_PRIVATE_LIST_GET', 'bigint', 'bigint|int|varchar|int|int|int|int|text|bigint', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_chat_private_list_del', 'xyproject', 'SP_CHAT_PRIVATE_LIST_DEL', 'bigint|bigint', '', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_chat_private_list_exist', 'xyproject', 'SP_CHAT_PRIVATE_LIST_EXIST', 'bigint|bigint', 'int', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_chat_private_list_del_all', 'xyproject', 'SP_CHAT_PRIVATE_LIST_DEL_ALL', 'bigint', 'bigint', NULL, NULL, NULL, '');
INSERT INTO `containers` VALUES ('game_rank_character_fc', 'xyproject', 'rank_character_fc', 'id', 'score|time|count', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_rank_character_lv', 'xyproject', 'rank_character_lv', 'id', 'score|time|count', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_rank_pet_chinese', 'xyproject', 'rank_pet_chinese', 'id', 'user_id|score|time|count', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_rank_pet_english', 'xyproject', 'rank_pet_english', 'id', 'user_id|score|time|count', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_rank_pet_fc', 'xyproject', 'rank_pet_fc', 'id', 'user_id|score|time|count', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_rank_pet_math', 'xyproject', 'rank_pet_math', 'id', 'user_id|score|time|count', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_rank_pet_science', 'xyproject', 'rank_pet_science', 'id', 'user_id|score|time|count', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_mail_get_list', 'xyproject', 'SP_MAIL_GET_LIST', 'bigint|int|int', 'int|int|bigint|int|varchar|bigint|tinyint', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_mail_get_detail', 'xyproject', 'SP_MAIL_GET_DETAIL', 'int|bigint', 'int|int|int|blob|int|blob|tinyint', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_mail_get_attachment', 'xyproject', 'SP_MAIL_GET_ATTACHMENT', 'int|bigint', 'tinyint|int|int|blob', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_mail_add', 'xyproject', 'SP_MAIL_ADD', 'int|bigint|bigint|int|varchar|int|blob|int|blob|bigint|tinyint|int', 'int', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_mail_init', 'xyproject', 'SP_MAIL_INIT', 'bigint|bigint|int', 'int', NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_mail_del', 'xyproject', 'SP_MAIL_DEL', 'bigint|int', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_mail_del_all', 'xyproject', 'SP_MAIL_DEL_ALL', 'bigint', NULL, NULL, NULL, NULL, NULL);
INSERT INTO `containers` VALUES ('game_mail_attachment_taken', 'xyproject', 'SP_MAIL_ATTACHMENT_TAKEN', 'int|bigint|tinyint', NULL, NULL, NULL, NULL, '');
INSERT INTO `containers` VALUES ('game_character_title', 'xyproject', 'character_title', 'user_id', 'title_ids', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_title', 'xyproject', 'title', 'title_id', 'cfg_id|group|atrr|create_time|duration_time|state', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_character_head', 'xyproject', 'character_head', 'user_id', 'head_ids', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_head', 'xyproject', 'head_state', 'user_id|head_id', 'state', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_character_head_frame', 'xyproject', 'character_head_frame', 'user_id', 'head_frame_ids', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_head_frame', 'xyproject', 'head_frame_state', 'user_id|head_frame_id', 'state', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_advert_incent', 'xyproject', 'SP_ADVERT_INCENT', 'bigint|int|int', 'int', NULL, NULL, NULL, '');
INSERT INTO `containers` VALUES ('game_character_card', 'xyproject', 'character_card', 'user_id', 'card_ids', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_character_card_piece', 'xyproject', 'character_card_piece', 'user_id', 'piece_ids', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_task_schedule', 'xyproject', 'task_schedule', 'user_id|cfg_id|type', 'cur_num|target|state', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_quest_list', 'xyproject', 'quest_list', 'user_id|fun_id|type', 'quest_cfg_id|level|max_level|max_level_count', 'mc_flags', 'mc_cas', 'mc_exprie', 'PRIMARY');
INSERT INTO `containers` VALUES ('game_character_quest_list', 'xyproject', 'character_quest_list', 'user_id', 'fun_id|type|quest_cfg_id|quest_status|quest_progress|daily_count', 'mc_flags', 'mc_cas', 'mc_expire', 'PRIMARY');

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
INSERT INTO `db_server` VALUES (1, '127.0.0.1', 3306, 'root', 'xmxy08155', 'xyproject', 3);

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
