<?php
include_once("config.php");
include_once("define.php");
include_once("../../db_config.php");

// 判断访问类型
$type = $_GET['type'];
if(1 == $type) {
	$equipmentId = $_GET['equipmentId'];
	if(!$equipmentId) {
		exit('{"code":0,"state":'.ERROR_EQUIPMENT_ID_EMPTY.'}');
	}
	$serAuth = md5($equipmentId.$type.$secretKey);
	if($serAuth == $_GET['authKey']) {
		$loginIp = file_get_contents($loginUrl);
		if($loginIp) {
			$conn = mysqli_connect($account_db_host, $account_db_username, $account_db_password, $account_db_database, $account_db_port);	
			$equipIdResult = mysqli_query($conn, "SELECT `index` FROM `users` WHERE `equipId` = '$equipmentId';");
			if(mysqli_num_rows($equipIdResult) > 0) {
				if($idxRow = mysqli_fetch_array($equipIdResult)) {
					$account = $idxRow[0];
					
					session_save_path($sessionSavePath);
					session_start();
					$sessionId = session_id();
					setcookie(session_name(), $sessionId, time() + $sessionLifeTime, "/");
					$_SESSION["accountId"] = $account;
					echo "{\"code\":1,\"accountId\":$account,\"sessionKey\":\"$sessionId\",\"loginIp\":\"$loginIp\"}";
				} else {
					echo '{"code":0,"state":'.ERROR_QUERY_DB_FAILT.',"num":1}';
				}
			} else {
				$maxIdx = 0;
				$maxIdxResult = mysqli_query($conn, "SELECT max(`index`) FROM `users`;");
				if($maxIdxRow = mysqli_fetch_array($maxIdxResult)) {
					$maxIdx = $maxIdxRow[0] + 1;
					
					$userName = 'anonymous'.$maxIdx;
					if(mysqli_query($conn, "insert into `users` (`equipId`,`userName`,`password`) values('$equipmentId', '$userName', '')")) {
						$account = mysqli_insert_id($conn);

						session_save_path($sessionSavePath);
						session_start();
						$sessionId = session_id();
						setcookie(session_name(), $sessionId, time() + $sessionLifeTime, "/");
						$_SESSION["accountId"] = $account;
						echo "{\"code\":1,\"accountId\":$account,\"sessionKey\":\"$sessionId\",\"loginIp\":\"$loginIp\"}";
					} else {
						echo '{"code":0,"state":'.ERROR_QUERY_DB_FAILT.',"num":2}';
					}
				} else {
					echo '{"code":0,"state":'.ERROR_QUERY_DB_FAILT.',"num":3}';
				}
			}
			mysqli_close($conn);
		} else {
			// 获取登录服失败返回json 格式错误 0
			echo '{"code":0,"state":'.ERROR_GET_LOGIN_IP_FAILED.'}';
		}
	} else {
		// 验证失败返回json 格式错误 0
		echo '{"code":0,"state":'.ERROR_INCORRECT_URL_REQUEST.'}';
	}
} else if(2 == $type) {
	$equipmentId = $_GET['equipmentId'];
	if(!$equipmentId) {
		exit('{"code":0,"state":'.ERROR_EQUIPMENT_ID_EMPTY.'}');
	}
	if(!$_GET['username']) {
		exit('{"code":0,"state":'.ERROR_USER_NAME_EMPTY.'}');
	}
	if(!$_GET['password']) {
		exit('{"code":0,"state":'.ERROR_PASSWORD_EMPTY.'}');
	}
	
	$serAuth = md5($equipmentId.$type.$_GET['username'].$_GET['password'].$secretKey);
	if($serAuth == $_GET['authKey']) {
		$loginIp = file_get_contents($loginUrl);
		if($loginIp) {
			$userName = addslashes($_GET['username']);
			$password = addslashes($_GET['password']);
			$conn = mysqli_connect($account_db_host, $account_db_username, $account_db_password, $account_db_database, $account_db_port);
			$loginSQL = "select `index`,`equipId` from users where userName='$userName' and password='$password' limit 1";
			$resultLogin = mysqli_query($conn, $loginSQL);
			if(mysqli_num_rows($resultLogin) > 0) {
				$row = mysqli_fetch_row($resultLogin);
				$account = $row[0];
				$equipId = $row[1];
				if($equipId != $equipmentId) {
					mysqli_query($conn, "update `users` set `equipId` = '$equipmentId' where `index` = $account; ");
				}

				session_save_path($sessionSavePath);
				session_start();
				$sessionId = session_id();
				setcookie(session_name(), $sessionId, time() + $sessionLifeTime, "/");
				$_SESSION["accountId"] = $account;
				echo "{\"code\":1,\"accountId\":$account,\"sessionKey\":\"$sessionId\",\"loginIp\":\"$loginIp\"}";
			} else {
				// 验证失败返回json 格式错误 0
				echo '{"code":0,"state":'.ERROR_VERIFICATION_FAILED.'}';
			}
			mysqli_close($conn);
		} else {
			// 获取登录服失败返回json 格式错误 0
			echo '{"code":0,"state":'.ERROR_GET_LOGIN_IP_FAILED.'}';
		}
	} else {
		// 验证失败返回json 格式错误 0
		echo '{"code":0,"state":'.ERROR_INCORRECT_URL_REQUEST.'}';
	}
} else {
	// 返回json 格式错误 0
	echo '{"code":0,"state":'.ERROR_TYPE_CANNT_IDENTIFIED.'}';
}
?>