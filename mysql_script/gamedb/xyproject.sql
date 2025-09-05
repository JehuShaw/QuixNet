/*
 Navicat Premium Data Transfer

 Source Server         : localhost
 Source Server Type    : MySQL
 Source Server Version : 50715
 Source Host           : localhost:3306
 Source Schema         : xyproject

 Target Server Type    : MySQL
 Target Server Version : 50715
 File Encoding         : 65001

 Date: 28/12/2020 10:53:03
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for advert_incentives
-- ----------------------------
DROP TABLE IF EXISTS `advert_incentives`;
CREATE TABLE `advert_incentives`  (
  `user_id` bigint(20) UNSIGNED ZEROFILL NOT NULL COMMENT '角色ID',
  `cfg_id` int(10) UNSIGNED NOT NULL COMMENT '视频激励配置表ID',
  `count` int(11) NOT NULL COMMENT '奖励领取次数',
  UNIQUE INDEX `id`(`user_id`, `cfg_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for character
-- ----------------------------
DROP TABLE IF EXISTS `character`;
CREATE TABLE `character`  (
  `idx` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` bigint(10) UNSIGNED NOT NULL,
  `account_id` bigint(11) UNSIGNED ZEROFILL NOT NULL,
  `cfg_id` int(10) UNSIGNED ZEROFILL NOT NULL,
  `name` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `level` int(11) NOT NULL,
  `exp` int(11) NOT NULL,
  `gem` int(11) NOT NULL,
  `coin` int(11) NOT NULL,
  `create_time` varchar(21) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `offline_time` varchar(21) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `status` int(11) NOT NULL,
  `prologue_process` tinyint(8) NOT NULL COMMENT '序章进度',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(11) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`idx`, `user_id`) USING BTREE,
  INDEX `name`(`name`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for character_backpack
-- ----------------------------
DROP TABLE IF EXISTS `character_backpack`;
CREATE TABLE `character_backpack`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `item_count` int(10) UNSIGNED ZEROFILL NOT NULL COMMENT '物品数量',
  `cell_count` int(10) UNSIGNED NOT NULL COMMENT '可以使用的背包格子',
  `unlock_count` int(10) NOT NULL COMMENT '解锁格子次数',
  `container` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '物品格子',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(11) NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for character_card
-- ----------------------------
DROP TABLE IF EXISTS `character_card`;
CREATE TABLE `character_card`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `card_ids` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '卡片ID',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for character_card_piece
-- ----------------------------
DROP TABLE IF EXISTS `character_card_piece`;
CREATE TABLE `character_card_piece`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `piece_ids` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '卡片碎片ID',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for character_dress
-- ----------------------------
DROP TABLE IF EXISTS `character_dress`;
CREATE TABLE `character_dress`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `dress_ids` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '装扮部件ID',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for character_head
-- ----------------------------
DROP TABLE IF EXISTS `character_head`;
CREATE TABLE `character_head`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `head_ids` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '头像ID',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for character_head_frame
-- ----------------------------
DROP TABLE IF EXISTS `character_head_frame`;
CREATE TABLE `character_head_frame`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `head_frame_ids` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '头像框ID',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for character_map
-- ----------------------------
DROP TABLE IF EXISTS `character_map`;
CREATE TABLE `character_map`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `map_ids` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '地图ID',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for character_pet
-- ----------------------------
DROP TABLE IF EXISTS `character_pet`;
CREATE TABLE `character_pet`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `pet_ids` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '玩家拥有的宠物的ID列表',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(11) NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for character_pet_equip
-- ----------------------------
DROP TABLE IF EXISTS `character_pet_equip`;
CREATE TABLE `character_pet_equip`  (
  `pet_id` bigint(20) UNSIGNED NOT NULL COMMENT '宠物实例ID',
  `equip_ids` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '宠物拥有的装备ID列表',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(11) NULL DEFAULT NULL,
  PRIMARY KEY (`pet_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for character_pet_skill
-- ----------------------------
DROP TABLE IF EXISTS `character_pet_skill`;
CREATE TABLE `character_pet_skill`  (
  `pet_id` bigint(20) UNSIGNED NOT NULL COMMENT '宠物实例ID',
  `skill_ids` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '宠物拥有的技能ID列表',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(11) NULL DEFAULT NULL,
  PRIMARY KEY (`pet_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for character_pos
-- ----------------------------
DROP TABLE IF EXISTS `character_pos`;
CREATE TABLE `character_pos`  (
  `user_id` bigint(10) UNSIGNED NOT NULL,
  `x` float(10, 2) NOT NULL,
  `y` float(10, 2) NOT NULL,
  `z` float(10, 2) NOT NULL,
  `faceX` float(10, 2) NOT NULL,
  `faceY` float(10, 2) NOT NULL,
  `faceZ` float(10, 2) NOT NULL,
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for character_quest
-- ----------------------------
DROP TABLE IF EXISTS `character_quest`;
CREATE TABLE `character_quest`  (
  `user_id` bigint(20) UNSIGNED NOT NULL,
  `quest_ids` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for character_quest_accessible
-- ----------------------------
DROP TABLE IF EXISTS `character_quest_accessible`;
CREATE TABLE `character_quest_accessible`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '玩家ID',
  `quest_ids` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '可接取的任务（不包含主线）',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(11) NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for character_quest_done
-- ----------------------------
DROP TABLE IF EXISTS `character_quest_done`;
CREATE TABLE `character_quest_done`  (
  `user_id` bigint(20) UNSIGNED NOT NULL,
  `quest_ids` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for character_quest_list
-- ----------------------------
DROP TABLE IF EXISTS `character_quest_list`;
CREATE TABLE `character_quest_list`  (
  `user_id` bigint(20) UNSIGNED ZEROFILL NOT NULL COMMENT '角色ID',
  `fun_id` int(11) NOT NULL COMMENT '功能id (对应页签）',
  `type` int(11) NOT NULL COMMENT '类型（页签内每一项位置）',
  `quest_cfg_id` int(11) UNSIGNED NOT NULL COMMENT '任务榜正在进行的任务配置表ID',
  `quest_status` int(11) NOT NULL COMMENT '任务状态： 0 未接取 1 正在进行 2 放弃 3 已完成',
  `quest_progress` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '任务进度',
  `daily_count` int(11) NOT NULL COMMENT '每日完成次数',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(11) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for character_title
-- ----------------------------
DROP TABLE IF EXISTS `character_title`;
CREATE TABLE `character_title`  (
  `user_id` bigint(20) UNSIGNED ZEROFILL NOT NULL COMMENT '角色ID',
  `title_ids` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '角色称号实例',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(11) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for chat_private
-- ----------------------------
DROP TABLE IF EXISTS `chat_private`;
CREATE TABLE `chat_private`  (
  `index` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `receiver_id` bigint(20) UNSIGNED NOT NULL COMMENT '接收消息的角色ID',
  `receiver_cfg_id` int(11) NOT NULL COMMENT '接收消息的角色配置表ID',
  `receiver_name` varchar(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '接收消息的角色名',
  `receiver_level` int(10) NOT NULL COMMENT '接收消息的角色等级',
  `receiver_head` int(11) NOT NULL COMMENT '接收消息的角色头像ID',
  `receiver_head_frame` int(11) NOT NULL COMMENT '接收消息的角色头像框ID',
  `sender_id` bigint(20) UNSIGNED NOT NULL COMMENT '发送消息的角色ID',
  `sender_cfg_id` int(11) NOT NULL COMMENT '发送消息的角色配置表ID',
  `sender_name` varchar(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '发送消息的角色名',
  `sender_level` int(10) UNSIGNED NOT NULL COMMENT '发送消息的角色等级',
  `sender_head` int(11) NOT NULL COMMENT '发送消息的角色头像ID',
  `sender_head_frame` int(11) NOT NULL COMMENT '发送消息的角色头像框ID',
  `content_type` int(11) NOT NULL COMMENT '聊天内容类型',
  `content` blob NOT NULL COMMENT '聊天内容',
  `send_time` bigint(20) NOT NULL COMMENT '发送时间',
  `read` tinyint(1) NOT NULL,
  PRIMARY KEY (`index`) USING BTREE,
  INDEX `send_time`(`send_time`) USING BTREE,
  INDEX `chat_key`(`receiver_id`, `sender_id`) USING BTREE,
  INDEX `receiver_id`(`receiver_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for chat_private_list
-- ----------------------------
DROP TABLE IF EXISTS `chat_private_list`;
CREATE TABLE `chat_private_list`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `other_id` bigint(20) UNSIGNED NOT NULL COMMENT '和该角色有私聊关系的角色ID',
  `other_cfg_id` int(10) UNSIGNED NULL DEFAULT NULL COMMENT '和该角色有私聊关系的配置表ID',
  `other_name` varchar(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '和该角色有私聊关系的角色名',
  `other_level` int(11) NOT NULL COMMENT '和该角色有私聊关系的角色等级',
  `other_head` int(11) NOT NULL COMMENT '和该角色有私聊关系的角色头像ID',
  `other_head_frame` int(11) NOT NULL COMMENT '发和该角色有私聊关系的角色头像框ID',
  `content_type` int(11) UNSIGNED NOT NULL COMMENT '聊天内容类型',
  `content` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '聊天内容 (只存字符类型聊天内容）',
  `send_time` bigint(20) UNSIGNED NOT NULL COMMENT '消息发送的时间',
  UNIQUE INDEX `id`(`user_id`, `other_id`) USING BTREE,
  INDEX `user_id`(`user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for chat_system
-- ----------------------------
DROP TABLE IF EXISTS `chat_system`;
CREATE TABLE `chat_system`  (
  `chat_id` bigint(20) UNSIGNED NOT NULL COMMENT '聊天实例ID',
  `content_type` int(11) NOT NULL COMMENT '聊天内容类型（1为本 2 语音）',
  `content` blob NOT NULL COMMENT '聊天内容',
  `send_time` bigint(20) NOT NULL COMMENT '发送时间',
  PRIMARY KEY (`chat_id`) USING BTREE,
  INDEX `send_time`(`send_time`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for dress
-- ----------------------------
DROP TABLE IF EXISTS `dress`;
CREATE TABLE `dress`  (
  `dress_id` bigint(20) UNSIGNED NOT NULL COMMENT '装扮部件实例ID',
  `cfg_id` int(11) NOT NULL COMMENT '配置表ID',
  `part` int(11) NOT NULL COMMENT '部位',
  `quality` int(11) NOT NULL COMMENT '品质',
  `property` int(11) NOT NULL COMMENT '属性',
  `create_time` bigint(20) NOT NULL COMMENT '部件创建时间（0，非0 为有期限部件）',
  `duration_time` int(11) NULL DEFAULT NULL COMMENT '部件有效时长（0，非0 为具体时长）',
  `default_sel` int(11) NOT NULL COMMENT '默认部件（0 未定义，1 非默认选择，2 默认选择部件 ）',
  `state` int(11) NOT NULL COMMENT '部件状态（0 未定义，1 未装扮， 2 已装扮，）',
  `mc_flags` int(20) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(20) NULL DEFAULT NULL,
  PRIMARY KEY (`dress_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for friend_application
-- ----------------------------
DROP TABLE IF EXISTS `friend_application`;
CREATE TABLE `friend_application`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '获得好友申请的角色ID',
  `user_ids` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '有哪些角色ID在申请',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for friend_apply_result
-- ----------------------------
DROP TABLE IF EXISTS `friend_apply_result`;
CREATE TABLE `friend_apply_result`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `target_id` bigint(20) UNSIGNED NOT NULL COMMENT '对方角色ID',
  `target_cfg_id` int(11) NOT NULL COMMENT '对方角色配置表ID',
  `target_name` varchar(128) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '对方角色名',
  `result` int(11) UNSIGNED ZEROFILL NOT NULL COMMENT '0 没有处理 1 同意 2 拒绝 ',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  UNIQUE INDEX `id`(`user_id`, `target_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for friend_blacklist
-- ----------------------------
DROP TABLE IF EXISTS `friend_blacklist`;
CREATE TABLE `friend_blacklist`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '黑名单拥有者ID',
  `user_ids` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '哪些角色ID 被列入黑名单',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for friend_detail
-- ----------------------------
DROP TABLE IF EXISTS `friend_detail`;
CREATE TABLE `friend_detail`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '拥有好友的角色ID',
  `friend_id` bigint(20) UNSIGNED NOT NULL COMMENT '好友角色ID',
  `friend_cfg_id` int(11) NOT NULL COMMENT '好友角色配置表ID',
  `note` varchar(128) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '备注',
  `order` bigint(20) UNSIGNED NOT NULL COMMENT '置顶次序',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  UNIQUE INDEX `id`(`user_id`, `friend_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for friend_list
-- ----------------------------
DROP TABLE IF EXISTS `friend_list`;
CREATE TABLE `friend_list`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '好友拥有者角色ID',
  `friend_ids` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '拥有的好友列表',
  `order_count` bigint(20) UNSIGNED NOT NULL COMMENT '置顶次序累加器',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for head_frame_state
-- ----------------------------
DROP TABLE IF EXISTS `head_frame_state`;
CREATE TABLE `head_frame_state`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `head_frame_id` int(10) UNSIGNED NOT NULL,
  `state` int(11) NOT NULL COMMENT '任务状态（0 未定义，1 未使用， 2 使用， ）',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  UNIQUE INDEX `character_head_frame_id`(`user_id`, `head_frame_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for head_state
-- ----------------------------
DROP TABLE IF EXISTS `head_state`;
CREATE TABLE `head_state`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `head_id` int(10) UNSIGNED NOT NULL,
  `state` int(11) NOT NULL COMMENT '任务状态（0 未定义，1 未使用， 2 使用， ）',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  UNIQUE INDEX `character_head_id`(`user_id`, `head_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for item
-- ----------------------------
DROP TABLE IF EXISTS `item`;
CREATE TABLE `item`  (
  `item_id` bigint(20) UNSIGNED NOT NULL COMMENT '物品实例ID',
  `cfg_id` int(11) NOT NULL COMMENT '配置表ID',
  `type` int(11) NOT NULL COMMENT '物品类型',
  `sub_type` int(11) NOT NULL COMMENT '物品子类型',
  `stack_num` int(11) NOT NULL COMMENT '物品叠加数量',
  `create_time` bigint(20) NOT NULL COMMENT '物品获得的时间',
  `mc_flags` int(20) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(20) NULL DEFAULT NULL,
  PRIMARY KEY (`item_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for mail
-- ----------------------------
DROP TABLE IF EXISTS `mail`;
CREATE TABLE `mail`  (
  `index` int(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `mail_type` int(20) NOT NULL COMMENT '邮件类型',
  `receiver_id` bigint(20) UNSIGNED NOT NULL COMMENT '接收者ID',
  `sender_id` bigint(20) UNSIGNED NOT NULL COMMENT '发送者ID',
  `title_type` int(20) NOT NULL COMMENT '邮件标题类型',
  `title` varchar(255) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '邮件标题',
  `content_type` int(20) NOT NULL COMMENT '邮件内容类型',
  `content` blob NOT NULL COMMENT '邮件内容',
  `attachment_type` int(20) NOT NULL COMMENT '附加内容类型',
  `attachment` blob NOT NULL COMMENT '邮件附件',
  `send_time` bigint(20) UNSIGNED NOT NULL COMMENT '发送时间',
  `read` tinyint(1) UNSIGNED ZEROFILL NOT NULL COMMENT '邮件是否已读取',
  `attachment_taken` tinyint(1) UNSIGNED ZEROFILL NOT NULL COMMENT '是否已领取附件',
  PRIMARY KEY (`index`) USING BTREE,
  INDEX `receiver_id`(`receiver_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for map_state
-- ----------------------------
DROP TABLE IF EXISTS `map_state`;
CREATE TABLE `map_state`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `map_id` int(10) UNSIGNED NOT NULL,
  `state` int(11) NOT NULL COMMENT '任务状态（0 未定义，1 未解锁， 2 解锁， ）',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  UNIQUE INDEX `character_map_id`(`user_id`, `map_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for pet
-- ----------------------------
DROP TABLE IF EXISTS `pet`;
CREATE TABLE `pet`  (
  `pet_id` bigint(20) UNSIGNED NOT NULL COMMENT '宠物实例ID',
  `cfg_id` int(11) NOT NULL COMMENT '配置表ID',
  `property_id` int(11) NOT NULL COMMENT '初始属性ID',
  `type` int(11) NOT NULL COMMENT '宠物类型',
  `name` varchar(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '宠物昵称',
  `quality` int(11) NOT NULL COMMENT '宠物品质(进化相关)',
  `level` int(11) NOT NULL COMMENT '宠物等级',
  `hp` int(11) NOT NULL COMMENT '宠物当前血量',
  `max_hp` int(11) NOT NULL COMMENT '宠物血条',
  `exp` int(11) NOT NULL COMMENT '宠物当前经验',
  `atk` int(11) NOT NULL COMMENT '宠物攻击力',
  `crit` int(11) NOT NULL COMMENT '宠物暴击',
  `block` int(11) NOT NULL COMMENT '宠物格挡',
  `hit` int(11) NOT NULL COMMENT '宠物命中',
  `dodge` int(11) NOT NULL COMMENT '宠物闪避',
  `speed` float(11, 0) NOT NULL COMMENT '宠物移动速动',
  `pos` int(11) NOT NULL COMMENT '宠物的位置',
  `status` int(11) NOT NULL COMMENT '宠物的状态（0休息，1出战）',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(11) NULL DEFAULT NULL,
  PRIMARY KEY (`pet_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for pet_equip
-- ----------------------------
DROP TABLE IF EXISTS `pet_equip`;
CREATE TABLE `pet_equip`  (
  `equip_id` bigint(20) UNSIGNED NOT NULL COMMENT '装备实例ID',
  `cfg_id` int(11) NOT NULL COMMENT '装备配置表ID',
  `max_ph` int(11) NOT NULL COMMENT '最大血量加成',
  `atk` int(11) NOT NULL COMMENT '攻击力加成',
  `crit` int(11) NOT NULL COMMENT '暴击加成',
  `block` int(11) NOT NULL COMMENT '格挡加成',
  `hit` int(11) NOT NULL COMMENT '命中加成',
  `dodge` int(11) NOT NULL COMMENT '闪避加成',
  `speed` int(11) NOT NULL COMMENT '移动速度加成',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(11) NULL DEFAULT NULL,
  PRIMARY KEY (`equip_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for pet_skill
-- ----------------------------
DROP TABLE IF EXISTS `pet_skill`;
CREATE TABLE `pet_skill`  (
  `skill_id` bigint(20) UNSIGNED NOT NULL COMMENT '技能实例ID',
  `cfg_id` int(11) NOT NULL COMMENT '配置表ID',
  `type` int(11) NOT NULL COMMENT '技能类型',
  `target` int(11) NOT NULL COMMENT '技能目标',
  `fly_speed` float(10, 2) NOT NULL COMMENT '飞行速度',
  `active_level` int(11) NOT NULL COMMENT '技能解锁等级',
  `effect_ids` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '效果id',
  `trigger_conds` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '触发条件',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(11) NULL DEFAULT NULL,
  PRIMARY KEY (`skill_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for quest_accessible
-- ----------------------------
DROP TABLE IF EXISTS `quest_accessible`;
CREATE TABLE `quest_accessible`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `cfg_id` int(10) UNSIGNED NOT NULL COMMENT '配置文件的任务ID',
  `count` int(11) NOT NULL COMMENT '已完成条件的数量',
  `progress` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '具体哪个条件完成了',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  UNIQUE INDEX `quest_id`(`user_id`, `cfg_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for quest_branch
-- ----------------------------
DROP TABLE IF EXISTS `quest_branch`;
CREATE TABLE `quest_branch`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `cfg_id` int(10) UNSIGNED NOT NULL COMMENT '置文件配的任务ID',
  `state` int(11) NOT NULL COMMENT '任务状态（1 完成未提交， 2 已接， 3 完成已提交）',
  `progress` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '任务进度',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  UNIQUE INDEX `quest_id`(`user_id`, `cfg_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for quest_event
-- ----------------------------
DROP TABLE IF EXISTS `quest_event`;
CREATE TABLE `quest_event`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `cfg_id` int(10) UNSIGNED NOT NULL COMMENT '配置文件的任务ID',
  `count` int(11) NOT NULL COMMENT '已完成条件的数量',
  `progress` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '具体哪个条件完成了',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  UNIQUE INDEX `quest_id`(`user_id`, `cfg_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for quest_list
-- ----------------------------
DROP TABLE IF EXISTS `quest_list`;
CREATE TABLE `quest_list`  (
  `user_id` bigint(20) UNSIGNED ZEROFILL NOT NULL COMMENT '角色ID',
  `fun_id` int(10) UNSIGNED ZEROFILL NOT NULL COMMENT '功能id (对应页签）',
  `type` int(10) UNSIGNED NOT NULL COMMENT '类型（页签内每一项位置）',
  `quest_cfg_id` int(10) UNSIGNED ZEROFILL NOT NULL COMMENT '任务榜配置表ID',
  `level` int(11) NOT NULL COMMENT '当前任务等级',
  `max_level` int(10) NOT NULL COMMENT '当前最高等级',
  `max_level_count` int(11) NOT NULL COMMENT '当前最高等级完成了多少次',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(11) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  UNIQUE INDEX `id`(`user_id`, `fun_id`, `type`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for quest_main
-- ----------------------------
DROP TABLE IF EXISTS `quest_main`;
CREATE TABLE `quest_main`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `cfg_id` int(10) UNSIGNED NOT NULL COMMENT '置文件配的任务ID',
  `state` int(11) NOT NULL COMMENT '任务状态（1 完成未提交， 2 已接， 3 完成已提交）',
  `progress` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '任务进度',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  UNIQUE INDEX `quest_id`(`user_id`, `cfg_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for rank_character_fc
-- ----------------------------
DROP TABLE IF EXISTS `rank_character_fc`;
CREATE TABLE `rank_character_fc`  (
  `id` bigint(20) UNSIGNED NOT NULL COMMENT '实例ID',
  `score` int(10) UNSIGNED NOT NULL COMMENT '用于排序的值',
  `time` bigint(20) UNSIGNED NOT NULL COMMENT '更新时间(用于排序先后）',
  `count` int(10) UNSIGNED NULL DEFAULT NULL COMMENT '更新次数累计(用于排序先后）',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for rank_character_lv
-- ----------------------------
DROP TABLE IF EXISTS `rank_character_lv`;
CREATE TABLE `rank_character_lv`  (
  `id` bigint(20) UNSIGNED NOT NULL COMMENT '实例ID',
  `score` int(10) UNSIGNED NOT NULL COMMENT '用于排序的值',
  `time` bigint(20) UNSIGNED NOT NULL COMMENT '更新时间(用于排序先后）',
  `count` int(10) UNSIGNED NULL DEFAULT NULL COMMENT '更新次数累计(用于排序先后）',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for rank_pet_chinese
-- ----------------------------
DROP TABLE IF EXISTS `rank_pet_chinese`;
CREATE TABLE `rank_pet_chinese`  (
  `id` bigint(20) UNSIGNED NOT NULL COMMENT '实例ID',
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID （宠物拥有者）',
  `score` int(10) UNSIGNED NOT NULL COMMENT '用于排序的值',
  `time` bigint(20) UNSIGNED NOT NULL COMMENT '更新时间(用于排序先后）',
  `count` int(10) UNSIGNED NULL DEFAULT NULL COMMENT '更新次数累计(用于排序先后）',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `id`(`id`, `user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for rank_pet_english
-- ----------------------------
DROP TABLE IF EXISTS `rank_pet_english`;
CREATE TABLE `rank_pet_english`  (
  `id` bigint(20) UNSIGNED NOT NULL COMMENT '实例ID',
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID （宠物拥有者）',
  `score` int(10) UNSIGNED NOT NULL COMMENT '用于排序的值',
  `time` bigint(20) UNSIGNED NOT NULL COMMENT '更新时间(用于排序先后）',
  `count` int(10) UNSIGNED NULL DEFAULT NULL COMMENT '更新次数累计(用于排序先后）',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `id`(`id`, `user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for rank_pet_fc
-- ----------------------------
DROP TABLE IF EXISTS `rank_pet_fc`;
CREATE TABLE `rank_pet_fc`  (
  `id` bigint(20) UNSIGNED NOT NULL COMMENT '实例ID',
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID （宠物拥有者）',
  `score` int(10) UNSIGNED NOT NULL COMMENT '用于排序的值',
  `time` bigint(20) UNSIGNED NOT NULL COMMENT '更新时间(用于排序先后）',
  `count` int(10) UNSIGNED NULL DEFAULT NULL COMMENT '更新次数累计(用于排序先后）',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `id`(`id`, `user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for rank_pet_math
-- ----------------------------
DROP TABLE IF EXISTS `rank_pet_math`;
CREATE TABLE `rank_pet_math`  (
  `id` bigint(20) UNSIGNED NOT NULL COMMENT '实例ID',
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID （宠物拥有者）',
  `score` int(10) UNSIGNED NOT NULL COMMENT '用于排序的值',
  `time` bigint(20) UNSIGNED NOT NULL COMMENT '更新时间(用于排序先后）',
  `count` int(10) UNSIGNED NULL DEFAULT NULL COMMENT '更新次数累计(用于排序先后）',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `id`(`id`, `user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for rank_pet_science
-- ----------------------------
DROP TABLE IF EXISTS `rank_pet_science`;
CREATE TABLE `rank_pet_science`  (
  `id` bigint(20) UNSIGNED NOT NULL COMMENT '实例ID',
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID （宠物拥有者）',
  `score` int(10) UNSIGNED NOT NULL COMMENT '用于排序的值',
  `time` bigint(20) UNSIGNED NOT NULL COMMENT '更新时间(用于排序先后）',
  `count` int(10) UNSIGNED NULL DEFAULT NULL COMMENT '更新次数累计(用于排序先后）',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `id`(`id`, `user_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for server_bal
-- ----------------------------
DROP TABLE IF EXISTS `server_bal`;
CREATE TABLE `server_bal`  (
  `server_id` smallint(5) UNSIGNED NOT NULL,
  PRIMARY KEY (`server_id`) USING BTREE
) ENGINE = MyISAM CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = FIXED;

-- ----------------------------
-- Table structure for server_dir
-- ----------------------------
DROP TABLE IF EXISTS `server_dir`;
CREATE TABLE `server_dir`  (
  `server_id` smallint(5) UNSIGNED NOT NULL,
  PRIMARY KEY (`server_id`) USING BTREE
) ENGINE = MyISAM CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = FIXED;

-- ----------------------------
-- Table structure for task_schedule
-- ----------------------------
DROP TABLE IF EXISTS `task_schedule`;
CREATE TABLE `task_schedule`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `cfg_id` int(10) UNSIGNED NOT NULL COMMENT 'schedule配置文件的任务ID',
  `type` tinyint(2) NOT NULL COMMENT '类型（1.卡片进度，2.其他任务扩展）',
  `cur_num` int(11) NOT NULL COMMENT '当前数量',
  `target` int(11) NOT NULL COMMENT '目标数量',
  `state` tinyint(2) NOT NULL COMMENT '完成状态（0.未完成；1.已完成）',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  UNIQUE INDEX `task_id`(`user_id`, `cfg_id`, `type`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for title
-- ----------------------------
DROP TABLE IF EXISTS `title`;
CREATE TABLE `title`  (
  `title_id` bigint(20) UNSIGNED NOT NULL COMMENT '称号实例ID',
  `cfg_id` int(11) NOT NULL COMMENT '配置表ID',
  `group` int(11) NOT NULL COMMENT '组',
  `atrr` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '属性',
  `create_time` bigint(20) NOT NULL COMMENT '部件创建时间（0，非0 为有期限部件）',
  `duration_time` int(11) NULL DEFAULT NULL COMMENT '部件有效时长（0，非0 为具体时长）',
  `state` int(11) NOT NULL COMMENT '状态（0 未定义，1 未使用， 2 已使用，）',
  `mc_flags` int(20) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(20) NULL DEFAULT NULL,
  PRIMARY KEY (`title_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Table structure for trigger
-- ----------------------------
DROP TABLE IF EXISTS `trigger`;
CREATE TABLE `trigger`  (
  `user_id` bigint(20) UNSIGNED NOT NULL COMMENT '角色ID',
  `cfg_id` int(10) UNSIGNED NOT NULL COMMENT '配置文件的任务ID',
  `count` int(11) NOT NULL COMMENT '已完成条件的数量',
  `progress` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL COMMENT '具体哪个条件完成了',
  `mc_flags` int(11) UNSIGNED NULL DEFAULT NULL,
  `mc_cas` bigint(20) UNSIGNED ZEROFILL NOT NULL,
  `mc_expire` int(10) UNSIGNED ZEROFILL NULL DEFAULT NULL,
  UNIQUE INDEX `trigger_id`(`user_id`, `cfg_id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Procedure structure for SP_ADVERT_INCENT
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_ADVERT_INCENT`;
delimiter ;;
CREATE PROCEDURE `SP_ADVERT_INCENT`(IN userId BIGINT UNSIGNED,IN cfgId INT UNSIGNED,IN maxCount INT)
BEGIN
	#Routine body goes here...
	DECLARE nCount INT UNSIGNED;
	SET nCount = (SELECT `count` FROM `advert_incentives` WHERE `user_id` = userId AND `cfg_id` = cfgId);
	IF nCount is NULL THEN
		SET nCount = 1;
		INSERT INTO `advert_incentives` (`user_id`,`cfg_id`,`count`) VALUES (userId,cfgId, nCount);
	ELSEIF nCount < maxCount THEN
		SET nCount = nCount + 1;
		UPDATE `advert_incentives` SET `count` = nCount WHERE `user_id` = userId AND `cfg_id` = cfgId;
	END IF;
	SELECT nCount;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_CHAT_PRIVATE_ADD
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_CHAT_PRIVATE_ADD`;
delimiter ;;
CREATE PROCEDURE `SP_CHAT_PRIVATE_ADD`(IN receiverId BIGINT UNSIGNED, IN receiverCfgId INT UNSIGNED, IN receiverName VARCHAR(32), IN receiverLevel INT, IN receiverHead INT UNSIGNED, IN receiverHeadFrame INT UNSIGNED, IN senderId BIGINT UNSIGNED, IN senderCfgId INT UNSIGNED, IN senderName VARCHAR(32), IN senderLevel INT,  IN senderHead INT UNSIGNED, IN senderHeadFrame INT UNSIGNED, IN contentType INT, IN content BLOB, IN sendTime BIGINT UNSIGNED, IN bRead TINYINT, IN keepSize INT)
BEGIN
	DECLARE nDelCount INT;
	
	INSERT INTO `chat_private` (`receiver_id`,`receiver_cfg_id`,`receiver_name`,`receiver_level`,`receiver_head`,`receiver_head_frame`,`sender_id`,`sender_cfg_id`,`sender_name`,`sender_level`,`sender_head`,`sender_head_frame`,`content_type`,`content`,`send_time`,`read`) VALUES (receiverId, receiverCfgId, receiverName, receiverLevel, receiverHead, receiverHeadFrame, senderId, senderCfgId, senderName, senderLevel, senderHead, senderHeadFrame, contentType, content, sendTime, bRead);
	
	SET nDelCount = (SELECT COUNT(`index`) - keepSize FROM `chat_private` WHERE `receiver_id` = receiverId);
	IF nDelCount > 0 THEN
		DELETE FROM `chat_private` WHERE `index` IN (SELECT C.`index` FROM (SELECT `index` FROM `chat_private` ORDER BY `send_time` ASC LIMIT nDelCount) C);
	END IF;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_CHAT_PRIVATE_DEL
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_CHAT_PRIVATE_DEL`;
delimiter ;;
CREATE PROCEDURE `SP_CHAT_PRIVATE_DEL`(IN receiverId BIGINT UNSIGNED, IN senderId BIGINT UNSIGNED)
BEGIN
	#Routine body goes here...
	DELETE FROM `chat_private` WHERE (`receiver_id` = receiverId AND `sender_id` = senderId) OR (`receiver_id` = senderId AND `sender_id` = receiverId);
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_CHAT_PRIVATE_DEL_ALL
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_CHAT_PRIVATE_DEL_ALL`;
delimiter ;;
CREATE PROCEDURE `SP_CHAT_PRIVATE_DEL_ALL`(IN receiverId BIGINT UNSIGNED, IN senderId BIGINT UNSIGNED)
BEGIN
	#Routine body goes here...
	DELETE FROM `chat_private` WHERE `receiver_id` = receiverId OR `receiver_id` = senderId;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_CHAT_PRIVATE_GET
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_CHAT_PRIVATE_GET`;
delimiter ;;
CREATE PROCEDURE `SP_CHAT_PRIVATE_GET`(IN receiverId BIGINT UNSIGNED, IN senderId BIGINT UNSIGNED, IN nOffset INT UNSIGNED, IN nCount INT UNSIGNED)
BEGIN
	SELECT `receiver_id`, `receiver_cfg_id`, `receiver_name`, `receiver_level`, `receiver_head`, `receiver_head_frame`, `sender_id`, `sender_cfg_id`, `sender_name`, `sender_level`, `sender_head`, `sender_head_frame`, `content_type`, `content`, `send_time`, `read` FROM `chat_private` WHERE (`receiver_id` = receiverId AND `sender_id` = senderId) OR (`receiver_id` = senderId AND `sender_id` = receiverId) ORDER BY `send_time` DESC LIMIT nOffset, nCount;
	UPDATE `chat_private` SET `read` = 1 WHERE `receiver_id` = receiverId AND `read` <> 1;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_CHAT_PRIVATE_INIT
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_CHAT_PRIVATE_INIT`;
delimiter ;;
CREATE PROCEDURE `SP_CHAT_PRIVATE_INIT`(IN receiverId BIGINT UNSIGNED, IN curTime bigint(20), IN duration int(11))
BEGIN
	DELETE FROM `chat_private` WHERE (`send_time` + duration) < curTime;
	SELECT COUNT(`read`) FROM `chat_private` WHERE `receiver_id` = receiverId AND `read` <> 1;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_CHAT_PRIVATE_LIST_ADD
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_CHAT_PRIVATE_LIST_ADD`;
delimiter ;;
CREATE PROCEDURE `SP_CHAT_PRIVATE_LIST_ADD`(IN `userId` BIGINT UNSIGNED,IN `otherId` BIGINT UNSIGNED,IN `otherCfgId` INT UNSIGNED,IN `otherName` VARCHAR(32),IN `otherLevel` INT,IN `otherHead` INT UNSIGNED,IN `otherHeadFrame` INT UNSIGNED,IN `contentType` INT,IN `content` BLOB, IN `sendTime` BIGINT UNSIGNED, IN keepSize INT)
BEGIN
	#Routine body goes here...
	DECLARE nDelCount INT;
	
	REPLACE	INTO `chat_private_list` (`user_id`,`other_id`,`other_cfg_id`,`other_name`,`other_level`,`other_head`,`other_head_frame`,`content_type`,`content`,`send_time`) VALUES (userId, otherId, otherCfgId, otherName, otherLevel, otherHead, otherHeadFrame, contentType, content, sendTime);
	
	SET nDelCount = (SELECT COUNT(`other_id`) - keepSize FROM `chat_private_list` WHERE `user_id` = userId);
	IF nDelCount > 0 THEN
		DELETE FROM `chat_private_list` WHERE `other_id` IN (SELECT C.`other_id` FROM (SELECT `other_id` FROM `chat_private_list` WHERE `user_id` = userId ORDER BY `send_time` ASC LIMIT nDelCount) C);
	END IF;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_CHAT_PRIVATE_LIST_DEL
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_CHAT_PRIVATE_LIST_DEL`;
delimiter ;;
CREATE PROCEDURE `SP_CHAT_PRIVATE_LIST_DEL`(IN `userId` BIGINT UNSIGNED, IN `otherId` BIGINT UNSIGNED)
BEGIN
	#Routine body goes here...
	DELETE FROM `chat_private_list` WHERE `user_id` = userId AND `other_id` = otherId;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_CHAT_PRIVATE_LIST_DEL_ALL
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_CHAT_PRIVATE_LIST_DEL_ALL`;
delimiter ;;
CREATE PROCEDURE `SP_CHAT_PRIVATE_LIST_DEL_ALL`(IN `userId` BIGINT UNSIGNED)
BEGIN
	#Routine body goes here...
	SELECT `other_id` FROM `chat_private_list` WHERE `user_id` = userId;
	DELETE FROM `chat_private_list` WHERE `user_id` = userId;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_CHAT_PRIVATE_LIST_EXIST
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_CHAT_PRIVATE_LIST_EXIST`;
delimiter ;;
CREATE PROCEDURE `SP_CHAT_PRIVATE_LIST_EXIST`(IN receiverId BIGINT UNSIGNED, IN senderId BIGINT UNSIGNED)
BEGIN
	#Routine body goes here... 
	SELECT COUNT(`user_id`) FROM `chat_private_list` WHERE `user_id` = receiverId AND `other_id` = senderId;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_CHAT_PRIVATE_LIST_GET
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_CHAT_PRIVATE_LIST_GET`;
delimiter ;;
CREATE PROCEDURE `SP_CHAT_PRIVATE_LIST_GET`(IN `userId` BIGINT UNSIGNED)
BEGIN
	#Routine body goes here...
	SELECT `other_id`, `other_cfg_id`, `other_name`, `other_level`, `other_head`, `other_head_frame`, `content_type`, `content`, `send_time` FROM `chat_private_list` WHERE `user_id` = userId;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_CHAT_SYSTEM_ADD
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_CHAT_SYSTEM_ADD`;
delimiter ;;
CREATE PROCEDURE `SP_CHAT_SYSTEM_ADD`(IN chat_id bigint(20), IN content_type int(11), IN content text, IN cur_time bigint(20), IN duration int(11), IN keepSize INT)
BEGIN
	#Routine body goes here...
	DECLARE nDelCount INT;

	INSERT INTO `chat_system` (`chat_id`, `content_type`, `content`, `send_time`) VALUES (chat_id, content_type, content, cur_time);
	
		SET nDelCount = (SELECT COUNT(`chat_id`) - keepSize FROM `chat_system`);
	IF nDelCount > 0 THEN
		DELETE FROM `chat_system` WHERE `chat_id` IN (SELECT C.`chat_id` FROM (SELECT `chat_id` FROM `chat_system` ORDER BY `send_time` ASC LIMIT nDelCount) C) OR (`send_time` + duration) < cur_time;
	ELSE
		DELETE FROM `chat_system` WHERE (`send_time` + duration) < cur_time;
	END IF;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_CHAT_SYSTEM_GET
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_CHAT_SYSTEM_GET`;
delimiter ;;
CREATE PROCEDURE `SP_CHAT_SYSTEM_GET`(IN nOffset INT UNSIGNED, IN nCount INT UNSIGNED)
BEGIN
	#Routine body goes here...
	SELECT `chat_id`, `content_type`, `content`, `send_time` FROM `chat_system` ORDER BY `send_time` DESC LIMIT nOffset, nCount;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_CHECK_BLACKLIST
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_CHECK_BLACKLIST`;
delimiter ;;
CREATE PROCEDURE `SP_CHECK_BLACKLIST`(IN userId BIGINT UNSIGNED)
BEGIN
	SELECT `user_id` FROM `friend_blacklist` WHERE FIND_IN_SET(userId,user_ids);
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_FRIEND_RESULT_LIST
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_FRIEND_RESULT_LIST`;
delimiter ;;
CREATE PROCEDURE `SP_FRIEND_RESULT_LIST`(IN userId bigint)
BEGIN
	#Routine body goes here...
	SELECT `target_name`, `result` FROM `friend_apply_result` WHERE `user_id` = userId;
	DELETE FROM `friend_apply_result` WHERE `user_id` = userId;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_MAIL_ADD
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_MAIL_ADD`;
delimiter ;;
CREATE PROCEDURE `SP_MAIL_ADD`(IN mailType INT, IN receiverId BIGINT UNSIGNED, IN senderId BIGINT UNSIGNED, IN titleType INT, IN title VARCHAR(255), IN contentType INT, IN content BLOB, IN attachmentType INT, IN attachment BLOB, IN sendTime BIGINT UNSIGNED, IN bRead TINYINT, IN keepSize INT)
BEGIN
	#Routine body goes here...
		DECLARE nDelCount INT;
	
	INSERT INTO `mail` (`mail_type`,`receiver_id`,`sender_id`,`title_type`,`title`,`content_type`,`content`,`attachment_type`,`attachment`,`send_time`,`read`,`attachment_taken`) VALUES (mailType, receiverId, senderId, titleType, title, contentType, content, attachmentType, attachment, sendTime, bRead, 0);
	SELECT LAST_INSERT_ID();
	
	SET nDelCount = (SELECT COUNT(`index`) - keepSize FROM `mail` WHERE `receiver_id` = receiverId);
	IF nDelCount > 0 THEN
		DELETE FROM `mail` WHERE `index` IN (SELECT C.`index` FROM (SELECT `index` FROM `mail` ORDER BY `send_time` ASC LIMIT nDelCount) C);
	END IF;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_MAIL_ATTACHMENT_TAKEN
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_MAIL_ATTACHMENT_TAKEN`;
delimiter ;;
CREATE PROCEDURE `SP_MAIL_ATTACHMENT_TAKEN`(IN nIndex INT UNSIGNED, IN receiverId BIGINT UNSIGNED, IN bTaken TINYINT UNSIGNED)
BEGIN
	#Routine body goes here...
	UPDATE `mail` SET `attachment_taken` = bTaken WHERE `index` = nIndex AND `receiver_id` = receiverId;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_MAIL_DEL
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_MAIL_DEL`;
delimiter ;;
CREATE PROCEDURE `SP_MAIL_DEL`(IN receiverId BIGINT UNSIGNED, IN nIndex INT UNSIGNED)
BEGIN
	#Routine body goes here...
	DELETE FROM `mail` WHERE `index` = nIndex AND `receiver_id` = receiverId;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_MAIL_DEL_ALL
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_MAIL_DEL_ALL`;
delimiter ;;
CREATE PROCEDURE `SP_MAIL_DEL_ALL`(IN receiverId BIGINT UNSIGNED)
BEGIN
	#Routine body goes here...
	DELETE FROM `mail` WHERE `receiver_id` = receiverId;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_MAIL_GET_ATTACHMENT
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_MAIL_GET_ATTACHMENT`;
delimiter ;;
CREATE PROCEDURE `SP_MAIL_GET_ATTACHMENT`(IN nIndex INT UNSIGNED, IN receiverId BIGINT UNSIGNED)
BEGIN
	#Routine body goes here...
	SELECT `attachment_taken`, `index`, `attachment_type`, `attachment` FROM `mail` WHERE `index` = nIndex AND `receiver_id` = receiverId;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_MAIL_GET_DETAIL
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_MAIL_GET_DETAIL`;
delimiter ;;
CREATE PROCEDURE `SP_MAIL_GET_DETAIL`(IN nIndex INT UNSIGNED, IN receiverId BIGINT UNSIGNED)
BEGIN
	#Routine body goes here...
	SELECT `index`, `mail_type`, `content_type`, `content`, `attachment_type`, `attachment`, `attachment_taken` FROM `mail` WHERE `index` = nIndex AND `receiver_id` = receiverId;
	UPDATE `mail` SET `read` = 1 WHERE `index` = nIndex AND `receiver_id` = receiverId AND `read` <> 1;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_MAIL_GET_LIST
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_MAIL_GET_LIST`;
delimiter ;;
CREATE PROCEDURE `SP_MAIL_GET_LIST`(IN receiverId BIGINT UNSIGNED, IN nOffset INT UNSIGNED, IN nCount INT UNSIGNED)
BEGIN
	#Routine body goes here...
	SELECT `index`, `mail_type`, `sender_id`, `title_type`, `title`, `send_time`, `read` FROM `mail` WHERE `receiver_id` = receiverId ORDER BY `send_time` DESC LIMIT nOffset, nCount;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_MAIL_INIT
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_MAIL_INIT`;
delimiter ;;
CREATE PROCEDURE `SP_MAIL_INIT`(IN `receiverId` bigint,IN `curTime` bigint,IN `duration` int)
BEGIN
	#Routine body goes here...
	DELETE FROM `mail` WHERE (`send_time` + duration) < curTime;
	SELECT COUNT(`read`) FROM `mail` WHERE `receiver_id` = receiverId AND `read` <> 1;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_MATCH_PLAYER
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_MATCH_PLAYER`;
delimiter ;;
CREATE PROCEDURE `SP_MATCH_PLAYER`(IN szMatch VARCHAR(256))
BEGIN
	#Routine body goes here...
	SELECT `user_id`, `cfg_id`, `name`, `level` FROM `character` WHERE `name` = szMatch or `user_id` = szMatch;
END
;;
delimiter ;

-- ----------------------------
-- Procedure structure for SP_RAND_PLAYER_LIST
-- ----------------------------
DROP PROCEDURE IF EXISTS `SP_RAND_PLAYER_LIST`;
delimiter ;;
CREATE PROCEDURE `SP_RAND_PLAYER_LIST`(IN min_lv int(11), IN max_lv int(11), IN userId BIGINT UNSIGNED)
BEGIN
	#Routine body goes here...
	SELECT `idx`, `user_id`, `cfg_id`, `name`, `level` FROM `character` AS t1 JOIN (SELECT ROUND(RAND() * (SELECT MAX(`idx`) FROM `character`)) - 3 AS id) AS t2 WHERE !FIND_IN_SET(t1.`user_id`,(SELECT `user_ids` FROM `friend_blacklist` WHERE `user_id` = userId)) AND !FIND_IN_SET(t1.`user_id`,(SELECT `friend_ids` FROM `friend_list` WHERE `user_id` = userId)) AND `user_id` <> userId AND `level` >= min_lv AND `level` <= max_lv AND t1.idx >= t2.id ORDER BY t1.idx LIMIT 3;
END
;;
delimiter ;

SET FOREIGN_KEY_CHECKS = 1;
