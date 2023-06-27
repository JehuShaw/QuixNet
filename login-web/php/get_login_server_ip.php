<?php
	include_once("db_config.php");
	
	$loginIp="";
	$conn=mysqli_connect($ctrl_db_host,$ctrl_db_username, $ctrl_db_password, $ctrl_db_database, $ctrl_db_port);
	mysqli_query($conn,'set names utf8');
	$sql='SELECT `server_ip` from `node_state` WHERE `server_name` = "LoginServer" and `server_status` = 1 ORDER BY `server_load` ASC LIMIT 1';
    $result=mysqli_query($conn,$sql);
	if($result) {
		$row = mysqli_fetch_row($result);
		$loginIp=$row[0];
	}
	mysqli_close($conn);
	echo $loginIp;
?>