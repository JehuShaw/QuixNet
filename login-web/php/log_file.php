<?php

/**
 * ******************
 * 1、写入内容到文件,追加内容到文件
 * 2、打开并读取文件内容
 * *******************
 */
 
 ini_set('date.timezone','Asia/Shanghai');

function save_log($path, $msg)
{
    if(! is_dir($path)) {
        mkdir($path);
    }
    $filename = $path . '/' . date('YmdHi') . '.txt';
    $content = date("Y-m-d H:i:s")."\r\n".$msg."\r\n \r\n \r\n ";
    file_put_contents($filename, $content, FILE_APPEND);
}

?>