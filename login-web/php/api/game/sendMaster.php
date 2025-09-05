<?php

//获取客户端ip
$ip = getenv("REMOTE_ADDR");

if('127.0.0.1' != $ip) {
	echo " ip = $ip\n";
	return;
}

include_once("masterSocket.php");

$adminName = 'web'; //$_GET['adminName'];
$password = '3c2eb4d99f35d2e4925e31a0bff3a23c'; //$_GET['password'];


if(empty($adminName) || empty($password)) {
	echo "Please set the administrator username and password!\n";
	return;
}

$cmd = $_GET['cmd'];
if(empty($cmd)) {
	echo "Please set cmd!\n";
	return;
}

$request = $_GET['request'];

$userId = $_GET['userId'];
if(!empty($userId)) {
	$response;
	$result = SendByUserId($adminName, $password, $userId, $cmd, $request, $response);
	echo "{\"result\":$result,\"data\":\"$response\"}\n";
	return;
} else {
	$servId = $_GET['servId'];
	if(!empty($servId)) {
		$response;
		$result = SendByServId($adminName, $password, $servId, $cmd, $request, $response);
		echo "{\"result\":$result,\"data\":\"$response\"}\n";
	} else {
		$response;
		$result = SendToMaster($adminName, $password, $cmd, $request, $response);
		echo "{\"result\":$result,\"data\":\"$response\"}\n";
	}
	return;
}

?>