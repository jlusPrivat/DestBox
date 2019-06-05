void SevSeg::init () {
  pinMode(pinSevSegLtch, OUTPUT);
  digitalWrite(pinSevSegLtch, LOW);
  pinMode(pinSevSegDout, OUTPUT);
  digitalWrite(pinSevSegDout, LOW);
  pinMode(pinSevSegClk, OUTPUT);
  digitalWrite(pinSevSegClk, LOW);
  off();
}


void SevSeg::off (bool showDashes) {
  uint8_t toWrite[] = {0, 0, 0, 0};
  if (showDashes) {
    toWrite[0] = transform('-');
    toWrite[1] = transform('-');
    toWrite[2] = transform('-');
    toWrite[3] = transform('-');
  }
  toDisplay(toWrite);
}


void SevSeg::writeNum (uint16_t num, uint8_t dots) {
  uint8_t toWrite[] = {
    uint8_t (transform('0' + (num / 1000)) + (dots >> 3)),
    uint8_t (transform('0' + ((num % 1000) / 100)) + ((dots >> 2) & B1)),
    uint8_t (transform('0' + ((num % 100) / 10)) + ((dots >> 1) & B1)),
    uint8_t (transform('0' + (num % 10)) + (dots & B1))
  };
  toDisplay(toWrite);
}


void SevSeg::toDisplay (uint8_t *data) {
  digitalWrite(pinSevSegLtch, LOW);
  // Send the rightmost position first
  for (int8_t i = 3; i >= 0; i--) {
    shiftOut(pinSevSegDout, pinSevSegClk, LSBFIRST, data[i]);
  }
  digitalWrite(pinSevSegLtch, HIGH);
}


uint8_t SevSeg::transform (char c) {
  switch (c) {
    case '0': return B11111100; break;
    case '1': return B01100000; break;
    case '2': return B11011010; break;
    case '3': return B11110010; break;
    case '4': return B01100110; break;
    case '5': return B10110110; break;
    case '6': return B10111110; break;
    case '7': return B11100000; break;
    case '8': return B11111110; break;
    case '9': return B11110110; break;
    case '-': return B00000010; break;
    default: return 0; break;
  }
}
