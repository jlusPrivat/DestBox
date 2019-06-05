uint32_t led::lastBlinked = 0, led::lastFaded = 0;
bool blinkState = false;
int16_t led::faderVal = 0;

// Constructing the object
led::led (uint8_t pin) {
  this->pin = pin;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}


void led::dVal (bool a) {
  this->fading = false;
  this->state = a;
  this->blinking = false;
  digitalWrite(this->pin, a ? HIGH : LOW);
}


void led::blink () {
  this->blinking = true;
  this->fading = false;
}


void led::fade () {
  this->fading = true;
  this->blinking = false;
  this->state = false;
}


void led::iterateAll () {
  bool hasToBlink = (millis() - led::lastBlinked) > (ledBlinkInterval / 2);
  if (hasToBlink) {
    led::lastBlinked = millis();
    blinkState = !blinkState;
  }
  bool hasToFade = (millis() - led::lastFaded) > (ledFadeInterval / 512);
  if (hasToFade) {
    led::lastFaded = millis();
    led::faderVal++;
    if (led::faderVal >= 255) led::faderVal = -255;
  }
  
  for (uint8_t i = 0; i < sizeof(led::fxLeds) / sizeof(led::fxLeds[0]); i++) {
    if (hasToBlink && led::fxLeds[i]->blinking)
      digitalWrite(led::fxLeds[i]->pin, blinkState ? HIGH : LOW);
    else if (hasToFade && led::fxLeds[i]->fading)
      analogWrite(led::fxLeds[i]->pin, abs(led::faderVal));
  }
}
