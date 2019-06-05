void actions::iterateStaticSwitches () {
  bool readingJack1 = digitalRead(pinJack1) == HIGH;
  bool readingJack2 = digitalRead(pinJack2) == HIGH;
  bool readingSw1 = digitalRead(pinSourceSw1) == LOW;
  bool readingSw2 = digitalRead(pinSourceSw2) == LOW;
  bool readingModeSw = digitalRead(pinModeSw) == LOW;

  if (readingJack1 && readingSw1) sourceLed1->dVal(true);
  else if (readingJack1 && !readingSw1) sourceLed1->fade();
  else if (!readingJack1 && readingSw1) sourceLed1->blink();
  else sourceLed1->dVal(false);

  if (readingJack2 && readingSw2) sourceLed2->dVal(true);
  else if (readingJack2 && !readingSw2) sourceLed2->fade();
  else if (!readingJack2 && readingSw2) sourceLed2->blink();
  else sourceLed2->dVal(false);

  if (authed2) {
    inTestMode = readingModeSw;
    modeLedGr->dVal(readingModeSw);
    modeLedRd->dVal(!readingModeSw);
  }
  else if (readingModeSw)
    modeLedRd->dVal(false);
  else
    modeLedRd->blink();
}


void actions::ignite () {
  if (!inTestMode && authed2) {
    if (digitalRead(pinJack1) == HIGH
      && digitalRead(pinSourceSw1) == LOW)
      digitalWrite(pinIgn1, LOW);
    if (digitalRead(pinJack2) == HIGH
      && digitalRead(pinSourceSw2) == LOW)
      digitalWrite(pinIgn2, LOW);
  }
  delay(1000);
  digitalWrite(pinIgn1, HIGH);
  digitalWrite(pinIgn2, HIGH);
  state = StateIgnited::getInstance();
}
