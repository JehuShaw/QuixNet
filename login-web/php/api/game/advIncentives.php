<?php
include_once("masterSocket.php");
include_once("controlCmd.php");
include_once("GPBMetadata/MsgGameAdvincentives.php");
include_once("Game/AdvIncentivesPacket.php");

$sign = $_GET['sign'];
if(empty($sign)) {
	echo "{\"isValid\": false,\"sign\":\"$sign\"}";
	return;
}

$transId = $_GET['trans_id'];
if(empty($transId)) {
	echo "{\"isValid\": false,\"sign\":\"$sign\"}";
	return;
}
// 您在头条媒体平台新建奖励视频代码位获取到的密钥
$appSecurityKey = "6b323a03223954f5f2626ea39c4f3145";

$checkSignRaw = "$appSecurityKey:$transId";
$curSign = hash("sha256", $checkSignRaw);
if($curSign != $sign) {
	echo "{\"isValid\": false,\"sign\":\"$sign\"}";
	return;
}
// 角色ID
$userId = $_GET['user_id'];
if(empty($userId)) {
	echo "{\"isValid\": false,\"sign\":\"$sign\"}";
	return;
}
// 奖励数量
$rewardAmount = $_GET['reward_amount'];
// 奖励名称
$rewardName = $_GET['reward_name'];
// 扩展参数
$extra = $_GET['extra'];

$advIncPacket = new Game\AdvIncentivesPacket();
if(!empty($rewardAmount)) {
	$advIncPacket->setRewardAmount($rewardAmount);
}
if(!empty($rewardName)) {
	$advIncPacket->setRewardName($rewardName);
}
if(!empty($extra)) {
	$advIncPacket->setExtra($extra);
}

$request = $advIncPacket->serializeToString();

global $C_CMD_CTM_ADV_INCENTIVES;

$result = SendByUserId('web', '3c2eb4d99f35d2e4925e31a0bff3a23c', $userId, $C_CMD_CTM_ADV_INCENTIVES, $request);
if($result == 1) {
	echo "{\"isValid\": true,\"sign\":\"$sign\"}";
} else {
	echo "{\"isValid\": false,\"sign\":\"$sign\"}";
}

?>