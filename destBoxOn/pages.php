<?php
namespace Pages;
use Device;

function getNavigation () {
	global $allowedActions, $action;
	$returner = '<ul class="navigation">';
	foreach ($allowedActions as $link => $description) {
		$returner .= "<li><a href=\"index.php?action=$link\""
		. ($link == $action ? ' class="selected"' : '')
		. ">$description</a></li>";
	}
	$returner .= '</ul>';
	return $returner;
}


function home () {
	return '<h2>Home</h2>' . getNavigation();
}


function getKey () {
	$returner = '<h2>Einen OVRD-Key erhalten</h2>' . getNavigation();
	$did = $_POST['did'] ?? '';
	$ovrd_c_k = $_POST['ovrd_c_k'] ?? '';
	
	if (($_GET['sent'] ?? 0) == 1) {
		$dev = new Device($did);
		if ($dev->deviceExists()) {
			$res = $dev->getNextOvrdKey(str_replace('-', '', $ovrd_c_k));
			if ($res != 'NO_KEY' && $res != 'WRONG_KEY')
				return $returner . '<div class="success">Der OVRD-K zum Entsperren ist '
				. $res . '.</div>';
		}
		$returner .= '<div class="failed">Eine der Angaben stimmt nicht.</div>';
	}
	
	return $returner . '<p>Hier k&ouml;nnen Sie einen neuen OVRD-K anfordern.</p>'
	. '<form action="index.php?action=getKey&sent=1" method="post">'
	. '<label for="did">Device-ID:</label>'
	. '<input type="number" name="did" id="did" value="'.$did.'" required><br>'
	. '<label for="ovrd_c_k">Overide-Consumer-Key:</label>'
	. '<input type="text" name="ovrd_c_k" id="ovrd_c_k" value="'.$ovrd_c_k.'" required><br>'
	. '<input type="submit" value="Schl&uuml;ssel erhalten"></form>';
}


function newKeys () {
	global $mysql, $device;
	$returner = '<h2>Neue Schl&uuml;ssel erzeugen</h2>' . getNavigation();
	
	if (($_GET['doIt'] ?? 0) == 1) {
		$result = $device->regenerateKeys();
		$romVer = $device->getAttr('eeprom_version')+1;
		$device->setAttr('eeprom_version', $romVer < 256 ? $romVer : 0);
		if ($mysql->errno)
			$returner .= '<div class="failed">'.$mysql->error.'</div>';
		else
			$returner .= '<div class="success">Neue Schl&uuml;ssel '
			. 'erfolgreich erzeugt und gespeichert.</div>';
		$device->commit();
		
		$returner .= '<p>Vergessen Sie nicht diese Wartung im Handbuch zu vermerken. '
		. 'Versiegeln Sie die neuen OVRD-Consumer-Keys m&ouml;glichst zeitnah.</p>'
		. '&Auml;ndern Sie in der constants.h diese Werte: <br>'
		. '<pre>#define DEVICE_ID ' . $device->getAttr('did') . '</pre><br>'
		. '<pre>#define EEPROM_VERSION ' . $device->getAttr('eeprom_version') . '</pre><br>'
		. 'Sowie alle <pre>ovrdKeyShas</pre> (zwischen den geschweiften Klammern):<br>';
		foreach ($result['shas'] as $num => $sha)
			$returner .= "<pre>$sha" . ($num == 9 ? '' : ',') . "</pre><br>";
		$returner .= '<br>Die neuen Override-Consumer-Keys (OVRD-C-Keys) sind wie folgt:';
		foreach ($result['ovrd_c'] as $num => $key)
			$returner .= "<br><pre>#$num: $key</pre>";
	}
	else
		$returner .= '<p>Hier k&ouml;nnen Sie neue OVRD-C-Keys und zugeh&ouml;rige '
		. 'OVRD-Keys erzeugen. Die alten Schl&uuml;ssel und die zugeh&ouml;rigen Logs '
		. 'werden unwiederbringlich gel&ouml;scht.</p>'
		. '<p>Sie sollten nur neue Schl&uuml;ssel generieren, wenn '
		. 'einer der Schl&uuml;ssel kompromitiert worden ist, einer nicht mehr zug&auml;nglich '
		. 'ist, oder alle bisherigen Schl&uuml;ssel bereits verbraucht worden sind.</p>'
		. '<p>Um die neuen Schl&uuml;ssel auf die DestBox &uuml;bertragen zu k&ouml;nnen, '
		. 'm&uuml;ssen Sie die Programmierung der DestBox komplett &uuml;berschreiben. '
		. 'Dabei gehen alle bisherigen Einstellungen (Auth1 und Auth2 Passphrasen) verloren.</p>'
		. '<p><a href="index.php?action=newKeys&doIt=1">Neue Schl&uuml;ssel '
		. 'trotzdem erzeugen</a></p>';
	
	return $returner;
}


function generalInfo () {
	global $device, $mysql;
	$softVer = $_POST['softVer'] ?? $device->getAttr('software_version');
	$bookVer = $_POST['bookVer'] ?? $device->getAttr('handbook_number');
	$maintainer = $_POST['maintainer'] ?? $device->getAttr('maintainer_mail');
	$pwd = $_POST['pwd'] ?? '';
	
	$returner = '<h2>Ger&auml;te-Informationen</h2>' . getNavigation();
	
	if (($_GET['sent'] ?? 0) == 1) {
		if (strlen($softVer) <= 10 && strlen($softVer) > 0)
			$device->setAttr('software_version', $softVer);
		if (is_numeric($bookVer))
			$device->setAttr('handbook_number', $bookVer);
		if (filter_var($maintainer, FILTER_VALIDATE_EMAIL))
			$device->setAttr('maintainer_mail', $maintainer);
		if (!empty($pwd))
			$device->setAttr('password', password_hash($pwd, PASSWORD_DEFAULT));
		if (!empty($error = $device->commit()))
			$returner .= '<div class="failed">'.$error.'</div>';
		else
			$returner .= '<div class="success">Erfolgreich gespeichert</div>';
	}
	
	$tablerows = '';
	$query = $mysql->query('SELECT `kid`, `used`, `crypt` FROM `destbox_keys` WHERE `did` = '
						   . $device->getAttr('did'));
	while ($row = $query->fetch_assoc())
		$tablerows .= '<tr><td>' . $row['kid'] . '</td><td>' . $row['used']
		. '</td><td>' . ($row['used'] ? sprintf('%010d', $row['crypt']) : '') . '</td></tr>';
	
	return $returner . '<form action="index.php?action=generalInfo&sent=1" method="post">'
	. 'Id: ' . $device->getAttr('did') . '<br>'
	. 'EEPROM-Version: ' . $device->getAttr('eeprom_version') . '<br>'
	. '<label for="softVer">Aktuelle Software Version:</label>'
	. '<input type="text" name="softVer" id="softVer" value="'.$softVer.'" required><br>'
	. '<label for="bookVer">Aktuelle Handbuch Nummer:</label>'
	. '<input type="number" name="bookVer" id="bookVer" value="'.$bookVer.'" required><br>'
	. '<label for="maintainer">Wartungsperson E-Mail:</label>'
	. '<input type="text" name="maintainer" id="maintainer" value="'.$maintainer.'" required><br>'
	. '<label for="pwd">Neues Passwort:</label>'
	. '<input type="password" name="pwd" id="pwd"><br>'
	. '<input type="submit" value="Speichern"> <input type="reset" value="Zur&uuml;cksetzen">'
	. '</form>'
	. '<p>Ein neues Handbuch sowie der gesamte Quelltext kann '
	. '<a href="https://github.com/jlusPrivat/DestBox" target="_blank">hier</a> herunter geladen werden.</p>'
	. '<table border="1"><tr><th>OVRD-K-Nummer: (PreCheck)</th><th>Benutzt am/um</th><th>'
	. 'Verwendeter OVRD-K</th></tr>' . $tablerows . '</table>';
}


function login () {
	$returner = '<h2>Login</h2>' . getNavigation();
	if (($_GET["sent"] ?? 0) == 1) {
		$id = $_POST["id"] ?? null;
		$pwd = $_POST["pwd"] ?? '';
		$device = new Device($id);
		if ($device->deviceExists()) {
			if (password_verify($pwd, $device->getAttr('password'))) {
				$_SESSION['destBox'] = $id;
				return $returner . '<div class="success">Sie wurden erfolgreich angemeldet.</div>';
			}
		}
		$returner .= '<div class="failed">Sie konnten nicht angemeldet werden.</div>';
	}
	return $returner . '<form action="index.php?action=login&sent=1" method="post">'
	. '<label for="id">Ger&auml;te-ID:</label>'
	. '<input type="number" name="id" id="id" min="0" size="3" required><br>'
	. '<label for="pwd">Passwort:</label>'
	. '<input type="password" name="pwd" id="pwd" required><br>'
	. '<input type="submit" value="Einloggen">'
	. '</form>';
}


function logout () {
	session_destroy();
	return '<h2>Logout</h2>' . getNavigation()
	. '<div class="success">Sie wurden erfolgreich abgemeldet.</div>';
}
?>