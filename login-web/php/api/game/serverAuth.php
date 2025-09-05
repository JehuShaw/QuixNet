<?php
include_once("config.php");
include_once("define.php");

// 解析参数
$accountId = $_GET['accountId'];
if(!$accountId) {
	exit('{"code":0,"state":'.ERROR_ACCOUNTID_EMPTY.'}');
}
$sessionKey = $_GET['sessionKey'];
if(!$sessionKey) {
	exit('{"code":0,"state":'.ERROR_SESSIONKEY_EMPTY.'}');
}

$serAuth = md5($accountId.$sessionKey.$secretKey);
if($serAuth == $_GET['authKey']) {
	session_id($sessionKey);
	session_save_path($sessionSavePath);
	session_start();
	if(session_status() == PHP_SESSION_ACTIVE) {
		$account = $_SESSION["accountId"];
		unset($_SESSION["accountId"]);
		if($accountId == $account) {
			echo "{\"code\":1,\"accountId\":$accountId}";
		} else {
			echo '{"code":0,"state":'.ERROR_SESSIONKEY_INCORRECT.'}';
		}
	} else {
		// 设置 session 失败
		echo '{"code":0,"state":'.ERROR_SESSIONKEY_NOT_EXIST.'}';
	}
	session_destroy();
} else {
	// 验证失败返回json 格式错误 0
	echo "{\"code\":0,\"state\":".ERROR_INCORRECT_URL_REQUEST.",\"authKey\":\"$serAuth\"}";
}

?>