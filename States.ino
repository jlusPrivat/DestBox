StateStart *StateStart::instance = NULL;

State *StateStart::getInstance () {
  if (!instance) instance = new StateStart();
  instance->currentPlace = 0;
  instance->currentInput = 0;
  
  lcd.print(" DestBox Version 1");
  lcd.setCursor(1, 2);
  lcd.print("Auth1 - Passwort:");
  lcd.setCursor(1, 3);
  lcd.print("_ _ _ _ _ _ _ _ _");
  authLedRd->dVal(true);
  modeLedGr->dVal(true);

  return instance;
}

void StateStart::keyboardContinue () {
  uint32_t password = EEPROM.read(0) == 225 ?
    ((((uint32_t) EEPROM.read(1) << 24) & 0xFF000000)
    | (((uint32_t) EEPROM.read(2) << 16) & 0xFF0000)
    | ((EEPROM.read(3) << 8) & 0xFF00)
    | (EEPROM.read(4) & 0xFF))
    : 123456789;
  
  if (password == currentInput)
    actions::state = StateMode::getInstance();
  else {
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
  // Output Voltage
  SevSeg::writeNum(map(analogRead(A0), 0, 1023, 0, 450) * 2, 4);
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
    EEPROM.write(0, 225);
    EEPROM.write(1, (uint8_t) (toSave >> 24) & 0xFF);
    EEPROM.write(2, (uint8_t) (toSave >> 16) & 0xFF);
    EEPROM.write(3, (uint8_t) (toSave >> 8) & 0xFF);
    EEPROM.write(4, (uint8_t) toSave& 0xFF);
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







StateAuth2 *StateAuth2::instance = NULL;

State *StateAuth2::getInstance () {
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
  lcd.print(Crypt::counter);
  lcd.setCursor(8, 3);
  lcd.print("_ _ _ _ _ _");
  
  return instance;
}

void StateAuth2::keyboardContinue () {
  if (pos == 0)
    actions::state = StateEnterTime::getInstance();
  else if (pos == 6 && Crypt::isCorrect(digits)) {
    actions::authed2 = true;
    actions::state = StateEnterTime::getInstance();
  }
  else {
    keyboardBack();
    lcd.setCursor(0, 2);
    lcd.print("Falsches OTP");
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

State *StateEnterTime::getInstance () {
  if (!instance) instance = new StateEnterTime();
  instance->pos = 0;
  for (uint8_t i = 0; i < 5; i++)
    instance->digits[i] = 10;
  
  lcd.clear();
  if (actions::mode == POSTCOUNT)
    lcd.print("   POST-COUNTDOWN");
  else
    lcd.print("   PRE-COUNTDOWN");
  lcd.setCursor(0, 2);
  lcd.print("Std     Min     Sek");
  lcd.setCursor(0, 3);
  lcd.print("_   :   _ _  :  _ _");
  
  return instance;
}

void StateEnterTime::keyboardContinue () {
  if (pos == 5)  {
    actions::countdownTime = 
      (uint32_t) ((((digits[0] * 60) + (digits[1] * 10)
      + digits[2]) * 60) + (digits[3] * 10) + digits[4]) * 10;
    actions::state = StateCounting::getInstance();
  }
}

void StateEnterTime::keyboardBack () {
  if (pos > 0)
    getInstance();
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
  instance->originalTime = actions::countdownTime;
  instance->prevLines = 0;

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
    if (actions::mode == POSTCOUNT) {
      counting = true;
      ignLed->dVal(false);
    }
    else if (actions::mode == PRECOUNT
      && actions::countdownTime == 0) {
      ignLed->dVal(false);
      actions::ignite();
    }
  }
}

void StateCounting::tick () {
  if (actions::countdownTime == 0 && counting) {
    counting = false;
    if (actions::mode == POSTCOUNT)
      actions::ignite();
    else
      ignLed->fade();
  }
  else if (counting) {
    actions::countdownTime--;
    displayTime();
  }
}

void StateCounting::displayTime () {
  if (actions::countdownTime < 600)
    SevSeg::writeNum(actions::countdownTime, 0xA);
  else if (actions::countdownTime < 36000) {
    uint8_t minutes = actions::countdownTime/600;
    uint8_t seconds = (actions::countdownTime/10) - (minutes*60);
    SevSeg::writeNum((minutes * 100) + seconds, 0x4);
  }
  else {
    uint8_t hours = actions::countdownTime/36000;
    uint8_t minutes = (actions::countdownTime/600) - (hours*60);
    SevSeg::writeNum((hours * 1000) + (minutes * 10), 0xA);
  }
  float percent = originalTime == 0 ? 1
    : (float) (originalTime - actions::countdownTime) / originalTime;
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


