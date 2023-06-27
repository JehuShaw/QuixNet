<?php
    include_once("db_config.php");

    if (empty($_POST)) {
        exit("您提交的表单数据超过post_max_size! <br>");
    }
	
	$userName = $_POST['userName'];
	if(empty($userName)) {
		exit("用户名不能为空！");
	}

    // 判断输入密码与确认密码是否相同
    $password = $_POST['password'];
    $confirmPassword = $_POST['confirmPassword'];
	
    if ($password != $confirmPassword) {
        exit("输入的密码与确认密码不相等！");
    }

    // 判断用户名是否重复
    $conn = mysqli_connect($account_db_host, $account_db_username, $account_db_password, $account_db_database, $account_db_port);
    $resultSet = mysqli_query($conn, "select * from users where userName = '$userName'");
    if (mysqli_num_rows($resultSet) > 0) {
        exit("用户名已被占用，请更换其他用户名");
    }

    mysqli_query($conn, "insert into `users` (`equipId`,`userName`,`password`) values('', '$userName', '$password')");
    $userID = mysqli_insert_id($conn);

    $userResult = mysqli_query($conn, "select * from `users` where `index` = '$userID'");
    if ($user = mysqli_fetch_array($userResult)) {
        echo "您的注册用户名为：" . $user['userName'];
    } else {
        exit("用户注册失败！");
    }
    mysqli_close($conn);
	
    echo "注册成功<br>";
?>