<?php
	include_once("db_config.php");
	
	// 
	$appName = $_POST['appName'];
	if(empty($appName)) {
		return;
	}

	$curVersion="";
	$conn=mysqli_connect($account_db_host, $account_db_username, $account_db_password, $account_db_database, $account_db_port);
	mysqli_query($conn,'set names utf8');
	$sql= "SELECT `version` from `app` WHERE `name` = \"$appName\"";
    $result=mysqli_query($conn,$sql);
	if($result) {
		$row = mysqli_fetch_row($result);
		$curVersion=$row[0];
	}
	mysqli_close($conn);
	echo $curVersion;
?>