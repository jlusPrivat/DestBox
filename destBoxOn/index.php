<?php
session_start();
require_once 'mysql_inc.php';
require_once 'pages.php';
require_once 'device.php';

// get the sessionId (deviceId) and associate with it the legal actions
$sessId = isset($_SESSION['destBox']) ? $_SESSION['destBox'] : null;
$device = new Device($sessId);
$allowedActions = array();
if ($device->deviceExists())
	$allowedActions = ['home' => 'Home',
					   'getKey' => 'OVRD-Key beziehen',
					   'newKeys' => 'Neue Schl&uuml;ssel erzeugen',
					   'generalInfo' => 'Allgemeine Infos anzeigen',
					   'logout' => 'Logout'];
else
	$allowedActions = ['home' => 'Home',
					   'getKey' => 'OVRD-Key beziehen',
					   'login' => 'Login'];

$action = isset($_GET['action']) && array_key_exists($_GET['action'], $allowedActions) ? $_GET['action'] : 'home';

$content = call_user_func('Pages\\' . $action);

$mysql->close();
?>

<!DOCTYPE html>

<html>
	<head>
		<meta charset="UTF-8">
		<link rel="stylesheet" type="text/css" href="styles.css">
		<title>DestBox Key Manager</title>
	</head>
	<body>
		<h1>DestBox Key Manager</h1>
		<?=$content?>
	</body>
</html>
