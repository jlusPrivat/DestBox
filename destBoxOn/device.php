<?php

class Device {
	private $dbEntry = array();
	
	public function __construct ($deviceId) {
		global $mysql;
		if (is_numeric($deviceId)) {
			$this->dbEntry = $mysql->query("SELECT * FROM `destbox_devices` WHERE `did` = "
						  . $mysql->real_escape_string($deviceId) . " LIMIT 1")->fetch_assoc();
		}
	}
	
	public function deviceExists () {
		return (bool) $this->dbEntry;
	}
	
	public function getAttr ($attr) {
		return $this->dbEntry[$attr];
	}
	
	public function setAttr ($attr, $value) {
		if ($attr != 'did' && array_key_exists($attr, $this->dbEntry))
			$this->dbEntry[$attr] = $value;
	}
	
	public function commit () {
		global $mysql;
		$q = array();
		foreach ($this->dbEntry as $k => $v)
			$q[] = '`'.$k.'` = "' . $mysql->real_escape_string($v).'"';
		$mysql->query("UPDATE `destbox_devices` SET " . implode(', ', $q)
					  . ' WHERE `did` = ' . $this->dbEntry['did']);
		return $mysql->error;
	}
	
	public function regenerateKeys () {
		global $mysql;
		
		$characters = '23456789ABCDEFGHJKLMNPQRSTUVWXYZ';
		$ivLength = openssl_cipher_iv_length('aes128');
		$charMaxIndex = strlen($characters) - 1;
		$returner = ['shas'=>array(), 'ovrd_c'=>array()];
		$values = array();
		for ($i = 0; $i < 10; $i++) {
			$ovrd_k = sprintf('%010d', mt_rand(0, 9999999999));
			$returner['shas'][] =
				'{0x'.implode(', 0x', str_split(strtoupper(sha1($ovrd_k)), 8)).'}';
			
			$ovrd_c_k = '';
			for ($a = 0; $a < 14; $a++)
				$ovrd_c_k .= $characters[mt_rand(0, $charMaxIndex)];
			$returner['ovrd_c'][] = substr_replace($ovrd_c_k, '-', 7, 0);
			
			$iv = openssl_random_pseudo_bytes($ivLength);
			$values[] = '('.$i.', '.$this->getAttr('did').', null, "'
			. openssl_encrypt($ovrd_k, 'aes128', $ovrd_c_k, 0, $iv) . '", "'.bin2hex($iv).'")';
		}
		
		$mysql->query('DELETE FROM `destbox_keys` WHERE `did` = ' . $this->getAttr('did'));
		$mysql->query('INSERT INTO `destbox_keys` (`kid`, `did`, `used`, `crypt`, `iv`) '
					  . 'VALUES ' . implode(', ', $values));
		return $returner;
	}
	
	
	public function getNextOvrdKey ($ovrd_c_k) {
		global $mysql;
		
		$result = $mysql->query('SELECT * FROM `destbox_keys` WHERE `did` = '
				. $this->dbEntry['did'] . ' AND `used` is null ORDER BY `kid` ASC LIMIT 1')->fetch_assoc();
		if (!$result)
			return 'NO_KEY';
		if (($decrypt = sprintf('%010d', openssl_decrypt($result['crypt'], 'aes128',
										$ovrd_c_k, 0, hex2bin($result['iv'])))) === false)
			return 'WRONG_KEY';
		
		$mysql->query('UPDATE `destbox_keys` SET `used` = CURRENT_TIMESTAMP, `crypt` = ' . $decrypt
					  . ' WHERE `kid` = ' . $result['kid'] . ' AND `did` = ' . $result['did'] . ' LIMIT 1');
		return $decrypt;
	}
}

?>