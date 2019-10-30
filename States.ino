StateRomReset *StateRomReset::instance = NULL;

State *StateRomReset::getInstance () {
  if (!instance) instance = new StateRomReset();

  for (int i = 0; i < EEPROM.length(); i++)
    EEPROM.update(i, 0);

  EEPROM.write(ROM_RESET, EEPROM_VERSION);
  
  lcd.setCursor(3, 1);
  lcd.print("EEPROM geleert");
  lcd.setCursor(5, 2);
  lcd.print("# druecken");

  return instance;
}

void StateRomReset::keyboardContinue () {
  actions::state = StateStart::getInstance();
}







StateStart *StateStart::instance = NULL;

State *StateStart::getInstance () {
  if (!instance) instance = new StateStart();
  instance->currentPlace = 0;
  instance->currentInput = 0;
  
  // switch off seven segment display
  SevSeg::off();

  lcd.clear();
  lcd.print(" DestBox Version 1");
  lcd.setCursor(1, 2);
  lcd.print("Auth1 - Passwort:");
  lcd.setCursor(1, 3);
  lcd.print("_ _ _ _ _ _ _ _ _");
  authLedRd->dVal(true);
  authLedGr->dVal(false);
  modeLedRd->dVal(false);
  modeLedGr->dVal(true);
  ignLed->dVal(false);

  // added for a full reset of values, when returning to this state
  actions::authed2 = false;
  bool inTestMode = true;
  uint8_t mode = 0;
  uint32_t countdownTime = 0;

  return instance;
}

void StateStart::keyboardContinue () {
  uint32_t password = EEPROM.read(ROM_AUTH1_SET) == 225 ?
    ((((uint32_t) EEPROM.read(ROM_AUTH1(0)) << 24) & 0xFF000000)
    | (((uint32_t) EEPROM.read(ROM_AUTH1(1)) << 16) & 0xFF0000)
    | ((EEPROM.read(ROM_AUTH1(2)) << 8) & 0xFF00)
    | (EEPROM.read(ROM_AUTH1(3)) & 0xFF))
    : 123456789;
  
  if (password == currentInput) {
    EEPROM.update(ROM_FAILED_COUNTER, 0);
    actions::state = StateMode::getInstance();
  }
  else {
    EEPROM.write(ROM_FAILED_COUNTER,
                 EEPROM.read(ROM_FAILED_COUNTER) + 1);
    if (EEPROM.read(ROM_FAILED_COUNTER) >= 3) {
      actions::state = StateLocked::getInstance();
      return;
    }
    lcd.setCursor(1, 1);
    lcd.print("Falsches Passwort!");
    keyboardBack();
  }
}

void StateStart::keyboardBack () {
  lcd.setCursor(1, 3);
  lcd.print("_ _ _ _ _ _ _ _ _");
  currentInput = 0;
  currentPlace = 0;
}

void StateStart::keyboardBtn (uint8_t a) {
  if (currentPlace < 9) {
    uint32_t add = a;
    for (uint8_t i = 0; i < 8-currentPlace; i++)
      add *= 10;
    lcd.setCursor(1 + (2*currentPlace), 3);
    lcd.print("*");
    currentInput += add;
    currentPlace++;
  }
}

void StateStart::tick () {
}







StateMode *StateMode::instance = NULL;

State *StateMode::getInstance () {
  if (!instance) instance = new StateMode();
  instance->currentlySelected = 0;
  
  SevSeg::off();
  uint8_t arrowChar[] = {0x08, 0x0C,
    0x0E, 0x0F, 0x0E, 0x0C, 0x08, 0x00};
  lcd.createChar(0, arrowChar);
  uint8_t invArrowChar[] = {0x17, 0x13,
    0x11, 0x10, 0x11, 0x13, 0x17, 0x1F};
  lcd.createChar(1, invArrowChar);
  uint8_t arrowUp[] = {0x04, 0x0E,
    0x15, 0x04, 0x04, 0x04, 0x04, 0x00};
  lcd.createChar(2, arrowUp);
  uint8_t arrowDown[] = {0x04, 0x04,
    0x04, 0x04, 0x15, 0x0E, 0x04, 0x00};
  lcd.createChar(3, arrowDown);
  lcd.clear();
  lcd.print("  Modus aussuchen:");
  lcd.setCursor(0, 1);
  lcd.write(1);
  lcd.print(" Pre-Countdown");
  lcd.setCursor(0, 2);
  lcd.write((uint8_t) 0);
  lcd.print(" Post-Countdown");
  lcd.setCursor(0, 3);
  lcd.write((uint8_t) 0);
  lcd.print(" Neues Passwort");
  lcd.setCursor(18, 1);
  lcd.write(2); lcd.print("2");
  lcd.setCursor(18, 3);
  lcd.write(3); lcd.print("8");
  
  return instance;
}

void StateMode::keyboardContinue () {
  switch (currentlySelected) {
    case 0: case 1:
      actions::mode = currentlySelected == 0 ? PRECOUNT : POSTCOUNT;
      if (!actions::authed2)
        actions::state = StateAuth2::getInstance();
      else
        actions::state = StateEnterTime::getInstance();
      break;
    case 2:
      actions::state = StateChangePwd::getInstance();
      break;
  }
}

void StateMode::keyboardBtn (uint8_t a) {
  switch (a) {
    case 2: case 4:
      if (currentlySelected == 0) currentlySelected = 2;
      else currentlySelected--;
      redrawArrows();
      break;
    case 8: case 6:
      if (currentlySelected == 2) currentlySelected = 0;
      else currentlySelected++;
      redrawArrows();
      break;
    case 5:
      keyboardContinue();
      break;
  }
} 

void StateMode::redrawArrows () {
  for (uint8_t elem = 0; elem < 3; elem++) {
    lcd.setCursor(0, elem+1);
    if (elem == currentlySelected) lcd.write((uint8_t) 1);
    else lcd.write((uint8_t) 0);
  }
}







StateChangePwd *StateChangePwd::instance = NULL;

State *StateChangePwd::getInstance () {
  if (!instance) instance = new StateChangePwd();
  for (uint8_t i = 0; i < 18; i++)
    instance->digits[i] = 10;
  instance->pos = 0;
  instance->flagForNext = false;

  lcd.clear();
  lcd.print("  Neues Passwort:");
  lcd.setCursor(1, 1);
  lcd.print("_ _ _ _ _ _ _ _ _");
  lcd.setCursor(4, 2);
  lcd.print("Wiederholen:");
  lcd.setCursor(1, 3);
  lcd.print("_ _ _ _ _ _ _ _ _");
  
  return instance;
}

void StateChangePwd::keyboardContinue () {
  if (flagForNext) {actions::state = StateMode::getInstance(); return;}
  lcd.clear();
  bool notEqual = false;
  uint32_t toSave = 0;
  for (uint8_t i = 0; i < 9; i++) {
    if (digits[i] > 9 || digits[i] != digits[i + 9])
      notEqual = true;
    uint32_t add = digits[i];
    for (uint8_t a = 0; a < 8-i; a++)
      add *= 10;
    toSave += add;
  }
  if (pos != 18 || notEqual) {
    lcd.setCursor(2, 0);
    lcd.print("Die eingegebenen");
    lcd.setCursor(2, 1);
    lcd.print("Passwoerter sind");
    lcd.setCursor(3, 2);
    lcd.print("nicht gleich");
    lcd.setCursor(3, 3);
    lcd.print("oder zu kurz");
  }
  else {
    EEPROM.update(ROM_AUTH1_SET, 225);
    EEPROM.write(ROM_AUTH1(0), (uint8_t) (toSave >> 24) & 0xFF);
    EEPROM.write(ROM_AUTH1(1), (uint8_t) (toSave >> 16) & 0xFF);
    EEPROM.write(ROM_AUTH1(2), (uint8_t) (toSave >> 8) & 0xFF);
    EEPROM.write(ROM_AUTH1(3), (uint8_t) toSave& 0xFF);
    lcd.setCursor(3, 1);
    lcd.print("Neues Passwort");
    lcd.setCursor(4, 2);
    lcd.print("gespeichert.");
  }
  flagForNext = true;
}

void StateChangePwd::keyboardBack() {
  if (flagForNext) {actions::state = StateMode::getInstance(); return;}
  if (pos > 0) StateChangePwd::getInstance();
  else actions::state = StateMode::getInstance();
}

void StateChangePwd::keyboardBtn(uint8_t a) {
  if (flagForNext) {actions::state = StateMode::getInstance(); return;}
  if (pos < 18) {
    if (pos < 9) lcd.setCursor((2*pos) + 1, 1);
    else lcd.setCursor((2*(pos - 9)) + 1, 3);
    lcd.print("*");
    digits[pos] = a;
    pos++;
  }
}







StateGenerateKey *StateGenerateKey::instance = NULL;

State *StateGenerateKey::getInstance () {
  if (!instance) instance = new StateGenerateKey();

  // generate and export the key
  randomSeed(analogRead(pinRandomSeed));
  Key key(20);
  uint8_t exportedKey[20] = {};
  key.exportToArray(exportedKey);

  // generate the first otp
  SimpleHOTP gen(key, 1);
  uint32_t firstHOTP = gen.generateHOTP();
  char hotpString[7];
  hotpString[6] = '\0';
  for (int8_t i = 5; i >= 0; i--) {
    hotpString[i] = (firstHOTP % 10) + '0';
    firstHOTP /= 10;
  }

  // save the key and reset the counter
  for (uint8_t i = 0; i < 20; i++)
    EEPROM.write(ROM_AUTH2_KEY(0) + i, exportedKey[i]);
  EEPROM.write(ROM_AUTH2_SET, 255);
  EEPROM.write(ROM_AUTH2_COUNTER(0), 0);
  EEPROM.write(ROM_AUTH2_COUNTER(1), 0);
  EEPROM.write(ROM_AUTH2_COUNTER(2), 0);
  EEPROM.write(ROM_AUTH2_COUNTER(3), 2);

  // generate base32 output value
  uint8_t bytePos = 0;
  uint8_t bitPos = 0;
  union {uint8_t u8[34]; char c[2][17];} output = {};
  for (uint8_t i = 0; i < 33; i++) {
    if (i == 16) continue;
    output.u8[i]= exportedKey[bytePos] >> max(3-bitPos, 0);
    uint8_t nextRow = (bitPos+5) / 8;
    bitPos = (bitPos + 5) % 8;
    bytePos += nextRow;
    if (nextRow) {
      output.u8[i] = output.u8[i] << bitPos;
      output.u8[i] |= exportedKey[bytePos] >> (8-bitPos);
    }
    output.u8[i] &= 0x1F;
    if (output.u8[i] <= 25)
      output.u8[i] += 'A';
    else
      output.u8[i] += 24;
  }

  lcd.clear();
  lcd.print("|  Neuer HOTP Key  |");
  lcd.setCursor(2, 1);
  lcd.print(output.c[0]);
  lcd.setCursor(2, 2);
  lcd.print(output.c[1]);
  lcd.setCursor(2, 3);
  lcd.print("Test #1:  ");
  lcd.print((String) hotpString);

  return instance;
}

void StateGenerateKey::keyboardContinue () {
  actions::state = StateAuth2::getInstance();
}







StateAuth2 *StateAuth2::instance = NULL;

State *StateAuth2::getInstance () {
  if (EEPROM.read(ROM_AUTH2_SET) != 255)
    return StateGenerateKey::getInstance();
  
  if (!instance) instance = new StateAuth2();
  instance->pos = 0;
  for (uint8_t i = 0; i < 6; i++)
    instance->digits[i] = 10;
  
  lcd.clear();
  lcd.print("  Autorisation II");
  lcd.setCursor(1, 1);
  lcd.print("Fuer Test optional");
  lcd.setCursor(0, 3);
  lcd.print("#:");
  lcd.print(((uint32_t) EEPROM.read(ROM_AUTH2_COUNTER(0)) << 24) |
        ((uint32_t) EEPROM.read(ROM_AUTH2_COUNTER(1)) << 16) |
        ((uint32_t) EEPROM.read(ROM_AUTH2_COUNTER(2)) << 8) |
        EEPROM.read(ROM_AUTH2_COUNTER(3)));
  lcd.setCursor(8, 3);
  lcd.print("_ _ _ _ _ _");
  
  return instance;
}

void StateAuth2::keyboardContinue () {
  if (pos == 0)
    // skip the authentication
    actions::state = StateEnterTime::getInstance();
    
  else if (pos == 6) {
    // load the key
    uint8_t keyArray[20];
    for (uint8_t i = 0; i < 20; i++)
      keyArray[i] = EEPROM.read(ROM_AUTH2_KEY(0) + i);
    Key key(keyArray, 20);
    SimpleHOTP validator(key,
        ((uint32_t) EEPROM.read(ROM_AUTH2_COUNTER(0)) << 24) |
        ((uint32_t) EEPROM.read(ROM_AUTH2_COUNTER(1)) << 16) |
        ((uint32_t) EEPROM.read(ROM_AUTH2_COUNTER(2)) << 8) |
        EEPROM.read(ROM_AUTH2_COUNTER(3)));
    validator.setThrottle(0);

    if (uint32_t newCounter = validator.validate(digits)) {
      // the entered otp is correct
      actions::authed2 = true;
      EEPROM.update(ROM_FAILED_COUNTER, 0);
      EEPROM.update(ROM_AUTH2_COUNTER(0),
          (uint8_t) (newCounter >> 24) & 0xFF);
      EEPROM.update(ROM_AUTH2_COUNTER(1),
          (uint8_t) (newCounter >> 16) & 0xFF);
      EEPROM.update(ROM_AUTH2_COUNTER(2),
          (uint8_t) (newCounter >> 8) & 0xFF);
      EEPROM.update(ROM_AUTH2_COUNTER(3),
          (uint8_t) newCounter & 0xFF);
      actions::state = StateEnterTime::getInstance();
    }
    else {
      // the entered otp is not correct
      EEPROM.write(ROM_FAILED_COUNTER,
               EEPROM.read(ROM_FAILED_COUNTER) + 1);
      if (EEPROM.read(ROM_FAILED_COUNTER) >= 3) {
        actions::state = StateLocked::getInstance();
        return;
      }
      keyboardBack();
      lcd.setCursor(0, 2);
      lcd.print("Falsches OTP");
    }
  }
}

void StateAuth2::keyboardBack () {
  if (pos > 0)
    getInstance();
  else
    actions::state = StateMode::getInstance();
}

void StateAuth2::keyboardBtn (uint8_t a) {
  if (pos < 6) {
    digits[pos] = a;
    lcd.setCursor((pos*2) + 8, 3);
    lcd.print(a);
    pos++;
  }
}







StateEnterTime *StateEnterTime::instance = NULL;

State *StateEnterTime::getInstance (uint8_t timerIndex) {
  if (!instance) instance = new StateEnterTime();
  instance->timerIndex = timerIndex;
  instance->pos = 0;
  for (uint8_t i = 0; i < 5; i++)
    instance->digits[i] = 10;
  
  lcd.clear();
  if (actions::mode == POSTCOUNT)
    lcd.print("   POST-COUNTDOWN");
  else
    lcd.print("   PRE-COUNTDOWN");
  if (timerIndex == 1) {
    lcd.setCursor(0, 1);
    lcd.print("Springbar");
  }
  lcd.setCursor(12, 1);
  lcd.print("(Port ");
  lcd.print(timerIndex + 1);
  lcd.print(")");
  lcd.setCursor(0, 2);
  lcd.print("Std     Min     Sek");
  lcd.setCursor(0, 3);
  lcd.print("_   :   _ _  :  _ _");
  
  return instance;
}

void StateEnterTime::keyboardContinue () {
  if (pos == 5)  {
    actions::countdownTime[timerIndex] =
      (uint32_t) ((((digits[0] * 60) + (digits[1] * 10)
      + digits[2]) * 60) + (digits[3] * 10) + digits[4]) * 10;
    if (timerIndex == 1)
      actions::state = StateCounting::getInstance();
    else
      actions::state = StateEnterTime::getInstance(1);
  }
  else if (pos == 0 && timerIndex == 1) {
    actions::countdownTime[1] = actions::countdownTime[0];
    actions::state = StateCounting::getInstance();
  }
}

void StateEnterTime::keyboardBack () {
  if (pos > 0)
    getInstance(timerIndex);
  else if (timerIndex == 1)
    actions::state = StateEnterTime::getInstance();
  else
    actions::state = StateMode::getInstance();
}

void StateEnterTime::keyboardBtn (uint8_t a) {
  if ((pos == 1 || pos == 3) && a >= 6) return;
  if (pos < 5) {
    if (pos == 0)
      lcd.setCursor(0, 3);
    else if (pos < 3)
      lcd.setCursor(((pos-1)*2) + 8, 3);
    else
      lcd.setCursor(((pos-3)*2) + 16, 3);
    lcd.print(a);
    digits[pos] = a;
    pos++;
  }
}







StateCounting *StateCounting::instance = NULL;

State *StateCounting::getInstance () {
  if (!instance) instance = new StateCounting();
  instance->authed = false;
  instance->counting = false;
  instance->originalTime = max(actions::countdownTime[0], actions::countdownTime[1]);
  instance->prevLines = 0;
  instance->backBtnTimer = 0;

  uint8_t row = 0;
  for (uint8_t i = 0; i < 5; i++) {
    row = row | (B1 << (4-i));
    uint8_t character[] = {row, row, row, row,
      row, row, row, row};
    lcd.createChar(i, character);
  }
  lcd.clear();
  if (actions::mode == POSTCOUNT)
    lcd.print("   POST-COUNTDOWN");
  else
    lcd.print("   PRE-COUNTDOWN");
  if (actions::countdownTime[0] != actions::countdownTime[1]) {
    lcd.setCursor(12, 1);
    lcd.print("(Port  )");
    instance->lowestTimer =
      actions::countdownTime[0] < actions::countdownTime[1] ? 0 : 1;
    lcd.setCursor(18, 1);
    lcd.print(instance->lowestTimer + 1);
  }
  lcd.setCursor(0, 2);
  lcd.print("|");
  lcd.setCursor(19, 2);
  lcd.print("|");
  lcd.setCursor(2, 3);
  lcd.print("Autorisation III");
  instance->displayTime();
  
  return instance;
}

void StateCounting::authorize () {
  authed = !authed;
  if (!authed) {
    ignLed->dVal(false);
    counting = false;
    lcd.setCursor(2, 3);
    lcd.print("Autorisation III");
  }
  else {
    lcd.setCursor(2, 3);
    lcd.print("                ");
    if (actions::mode == POSTCOUNT)
      ignLed->fade();
    else
      counting = true;
  }
  authLedGr->dVal(authed);
  authLedRd->dVal(!authed);
}

void StateCounting::pressIgnSw () {
  if (authed) {
    counting = true;
    ignLed->dVal(false);
    if (actions::mode == PRECOUNT) {
      for (uint8_t i = 0; i <= 1; i++) {
        if (actions::countdownTime[i] == 0)
          actions::ignite(i);
      }
    }
  }
}

void StateCounting::keyboardBack () {
  if (backBtnTimer == 0)
    backBtnTimer = 1;
  else
    actions::state = StateStart::getInstance();
}

void StateCounting::tick () {
  if (backBtnTimer > 0 && (backBtnTimer++ > 20))
    backBtnTimer = 0;

  bool bothFinished = true;
  for (uint8_t i = 0; i <= 1; i++) {
    if (counting && actions::countdownTime[i] > 0) {
      actions::countdownTime[i]--;
      if (i != lowestTimer && (actions::countdownTime[i]
          < actions::countdownTime[lowestTimer] || actions::countdownTime[lowestTimer] < 0)) {
        lowestTimer = i;
        lcd.setCursor(18, 1);
        lcd.print(i + 1);
      }
    }
    else if (counting && actions::countdownTime[i] == 0) {
      switch (actions::mode) {
        case POSTCOUNT: actions::ignite(i); break;
        default: ignLed->fade(); counting = false; break;
      }
    }
    else if (actions::countdownTime[i] < -1) {
      if (++actions::countdownTime[i] == -1) {
        if (i == 0) digitalWrite(pinIgn1, HIGH);
        else digitalWrite(pinIgn2, HIGH);
      }
    }
    if (actions::countdownTime[i] != -1)
      bothFinished = false;
  }
  if (counting)
    displayTime();
  if (bothFinished)
    actions::state = StateIgnited::getInstance();
}

void StateCounting::displayTime () {
  int32_t lcdTime, sevSegTime;
  if (actions::countdownTime[0] == actions::countdownTime[1])
    lcdTime = sevSegTime = actions::countdownTime[0];
  else {
    if (actions::countdownTime[1] > actions::countdownTime[0])
      lcdTime = actions::countdownTime[1];
    else
      lcdTime = actions::countdownTime[0];
    sevSegTime = actions::countdownTime[lowestTimer];
  }
  lcdTime = lcdTime < 0 ? 0 : lcdTime;
  sevSegTime = sevSegTime < 0 ? 0 : sevSegTime;

  if (sevSegTime < 600)
    SevSeg::writeNum(sevSegTime, 0xA);
  else if (sevSegTime < 36000) {
    uint8_t minutes = sevSegTime/600;
    uint8_t seconds = (sevSegTime/10) - (minutes*60);
    SevSeg::writeNum((minutes * 100) + seconds, 0x4);
  }
  else {
    uint8_t hours = sevSegTime/36000;
    uint8_t minutes = (sevSegTime/600) - (hours*60);
    uint8_t tenSeconds = (sevSegTime/100)
        - (hours*360) - (minutes*6);
    SevSeg::writeNum((hours * 1000) + (minutes * 10) + tenSeconds, 0xA);
  }
  float percent = originalTime == 0 ? 1
    : (float) (originalTime - lcdTime) / originalTime;
  uint8_t lines = 90 * percent;
  if (prevLines != lines) {
    prevLines = lines;
    uint8_t pos = 1;
    while (lines > 0) {
      lcd.setCursor(pos, 2);
      if (lines >= 5) {
        lcd.write((uint8_t) 4);
        lines -= 5;
      }
      else {
        lcd.write((uint8_t) lines-1);
        lines = 0;
      }
      pos++;
    }
  }
}







StateIgnited *StateIgnited::instance = NULL;

State *StateIgnited::getInstance () {
  if (!instance) instance = new StateIgnited();
    
  SevSeg::off(true);
  lcd.clear();
  lcd.print("--------------------");
  lcd.setCursor(0, 1);
  lcd.print("| ZUENDUNG ERFOLGT |");
  lcd.setCursor(0, 2);
  lcd.print(actions::inTestMode
    ? "|      (Test)      |"
    : "|     (Scharf)     |");
  lcd.setCursor(0, 3);
  lcd.print("--------------------");
  authLedGr->dVal(false);
  authLedRd->dVal(true);

  return instance;
}

void StateIgnited::keyboardBack () {
  actions::state = StateStart::getInstance();
}






StateLocked *StateLocked::instance = NULL;

State *StateLocked::getInstance () {
  if (!instance) instance = new StateLocked();

  SevSeg::off(true);
  lcd.clear();
  lcd.print("  DestBox gesperrt");
  lcd.setCursor(0,1);
  lcd.print("Device-ID: ");
  lcd.print(DEVICE_ID);
  instance->keyboardBack();

  return instance;
}

void StateLocked::keyboardContinue () {
  if (currentPlace == 10 && !timeWaiting) {
    uint32_t userHash[5] = {};
    SimpleSHA1::generateSHA(currentInput, 80, userHash);
    uint32_t *systemHash = ovrdKeyShas[EEPROM.read(ROM_NEXT_OVRD)];
    
    bool correct = true;
    for (uint8_t i = 0; i < 5; i++) {
      if (userHash[i] != systemHash[i])
        correct = false;
    }

    if (correct) {
      EEPROM.write(ROM_NEXT_OVRD, EEPROM.read(ROM_NEXT_OVRD) + 1);
      EEPROM.write(ROM_FAILED_COUNTER, 0);
      EEPROM.update(ROM_AUTH1_SET, 0);
      EEPROM.update(ROM_AUTH2_SET, 0);
      actions::state = StateStart::getInstance();
    }
    else {
      timeWaiting = 300;
    }
  }
}

void StateLocked::keyboardBack () {
  if (!timeWaiting) {
    currentPlace = 0;
    lcd.setCursor(0, 2);
    lcd.print("OVRD-K | PreCheck: ");
    lcd.print(EEPROM.read(ROM_NEXT_OVRD));
    lcd.setCursor(0, 3);
    lcd.print("_ _ _ _ _ _ _ _ _ _");
  }
}

void StateLocked::keyboardBtn (uint8_t input) {
  if (currentPlace < 10 && !timeWaiting) {
    lcd.setCursor(2*currentPlace, 3);
    lcd.print(input);
    currentInput[currentPlace++] = input + '0';
  }
}

void StateLocked::tick () {
  if (timeWaiting) {
    if ((timeWaiting % 10) == 0) {
      lcd.setCursor(0, 2);
      lcd.print("  Falsche Eingabe   ");
      lcd.setCursor(17, 3);
      lcd.print("   ");
      lcd.setCursor(0, 3);
      lcd.print(" Bitte warten: ");
      lcd.print(timeWaiting/10);
      lcd.print("s");
    }
    if (--timeWaiting == 0)
      keyboardBack();
  }
}
