<?php
$mysql = @new mysqli('localhost', 'testuser', 'testaa', 'test');
if ($mysql->connect_errno)
	die($mysql->connect_error);
else
	$mysql->set_charset('utf8');
?>