<?php
	// 加密密钥
	$secretKey = '#XueYou#!&^!!';
	// 获取登录服务器地址的URL, 这个页面返回localhost:2425格式的字符串。
	$loginUrl = "http://127.0.0.1/quixnet/get_login_server_ip.php";
	// session 保持时间 （单位秒）
	$sessionLifeTime = 30;
	// 设置一个存放目录
	$sessionSavePath = './session_save_dir/';
	// 调试log 存放目录
	$logSavePath = './log_file/';
	// Master Server Ip
	$masterServerIP = '127.0.0.1';
	// Master server port
	$masterServerPort = 9055;
	// Master crypt "webKey" (Max char length 32)
	$webKey = 'Ko*#@%0Z1*YSk@Jn';
	// 10 second
	$authTimestampSpan = 10;

?>