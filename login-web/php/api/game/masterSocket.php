<?php

include_once("config.php");
include_once("vendor/autoload.php");
include_once("GPBMetadata/MsgMasterData.php");
include_once("Node/MasterDataPacket.php");


function SendByServId($adminName, $password, $servId, $cmd, &$requestData = NULL, &$responseData = NULL) {
	$dpRequest = new Node\MasterDataPacket();
	$dpRequest->setServerid(0);
	$dpRequest->setUserid($userId);
	$dpRequest->setCmd($cmd);
	if(!empty($requestData)) {
		$dpRequest->setData($requestData);
	}
	$dpRequest->setAdminname($adminName);
	$dpRequest->setAuthcode(GetAuthCode($adminName, $password));
	$dpResponse = new Node\MasterDataPacket();
	SendNodePacket($dpRequest, $dpResponse);
	if(!empty($responseData)) {
		$responseData = $dpResponse->getData();
	}
	return $dpResponse->getResult();
}

function SendByUserId($adminName, $password, $userId, $cmd, &$requestData = NULL, &$responseData = NULL) {
	$dpRequest = new Node\MasterDataPacket();
	$dpRequest->setServerid(0);
	$dpRequest->setUserid($userId);
	$dpRequest->setCmd($cmd);
	if(!empty($requestData)) {
		$dpRequest->setData($requestData);
	}

	$dpRequest->setAdminname($adminName);
	$dpRequest->setAuthcode(GetAuthCode($adminName, $password));
	$dpResponse = new Node\MasterDataPacket();
	SendNodePacket($dpRequest, $dpResponse);
	if(!empty($responseData)) {
		$responseData = $dpResponse->getData();
	}
	return $dpResponse->getResult();
}

function SendToMaster($adminName, $password, $cmd, &$requestData = NULL, &$responseData = NULL) {
	$dpRequest = new Node\MasterDataPacket();
	$dpRequest->setCmd($cmd);
	if(!empty($requestData)) {
		$dpRequest->setData($requestData);
	}

	$dpRequest->setAdminname($adminName);
	$dpRequest->setAuthcode(GetAuthCode($adminName, $password));
	$dpResponse = new Node\MasterDataPacket();
	SendNodePacket($dpRequest, $dpResponse);
	if(!empty($responseData)) {
		$responseData = $dpResponse->getData();
	}
	return $dpResponse->getResult();
}

function GetAuthCode($adminName, $password) {
	global $webKey, $authTimestampSpan;
	$timestamp = time() & 0xFFFFFFFF;
	$timestamp /= $authTimestampSpan;
	$timestamp = sprintf("%u", $timestamp);
	//echo "adminName = $adminName, password = $password, webKey = $webKey, timestamp = $timestamp\n";
	return md5($adminName.$password.$webKey.$timestamp);
}

function SendNodePacket($dpRequest, &$dpResponse)
{
	global $masterServerIP, $masterServerPort;
	
    $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
	if ($socket < 0) {
		return -1;
	}
	$result = socket_connect($socket, $masterServerIP, $masterServerPort);
	if ($result < 0) {
		return -2;
	}
	
	$bytes = $dpRequest->serializeToString();
	$dataSize = strlen($bytes);
	$packetSize = $dataSize + 4;

	$format = "N1N1A$dataSize";
	$packet = pack($format,$packetSize,$dataSize,$bytes);
	
	if(socket_write($socket, $packet, strlen($packet)) != false) {
		//读取服务端返回来的套接流信息
		$packetSize = unpack("N1", socket_read($socket, 4))[1];
		$dataSize = unpack("N1", socket_read($socket, 4))[1];
		if ($packetSize == $dataSize + 4) {
			if($buffer = socket_read($socket, $dataSize)) {
				$dpResponse->mergeFromString($buffer);
			}
			return 0;
		}
	}
	return -3;
}

?>