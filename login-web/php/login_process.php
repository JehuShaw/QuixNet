<?php
    include_once("db_config.php");
	
    $userName = addslashes($_POST['userName']);
    $password = addslashes($_POST['password']);
	
    $conn = mysqli_connect($account_db_host, $account_db_username, $account_db_password, $account_db_database, $account_db_port);
    $loginSQL = "select * from `users` where userName='$userName' and password='$password'";
    $resultLogin = mysqli_query($conn, $loginSQL);
    if (mysqli_num_rows($resultLogin) > 0) {
        echo $_POST['userName']." 登录成功";
    } else {
        echo $_POST['userName']." 登录失败";
    }
    mysqli_close($conn);
?>