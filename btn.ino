btn::btn (uint8_t pin, void rot(), int8_t triggerEdge) {
  pinMode(pin, INPUT_PULLUP);
  this->pin = pin;
  this->triggerEdge = triggerEdge;
  this->runOnTrigger = rot;
  lastTriggered = 0;
  currentState = false; // True is low; Pressed
}


void btn::iterateAll () {
  for (uint8_t i = 0; i < sizeof(btn::fxBtns) / sizeof(btn::fxBtns[0]); i++) {
    bool reading = digitalRead(fxBtns[i]->pin) == LOW;
    if (millis() - fxBtns[i]->lastTriggered > btnDebounce) {
      if (fxBtns[i]->currentState != reading) {
        fxBtns[i]->currentState = reading;
        if (fxBtns[i]->triggerEdge == CHANGE
        || (reading && fxBtns[i]->triggerEdge == RISING)
        || (!reading && fxBtns[i]->triggerEdge == FALLING))
          fxBtns[i]->runOnTrigger();
      }
    }
  }
}
