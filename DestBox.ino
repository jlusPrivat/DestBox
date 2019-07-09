#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <SimpleHOTP.h>
#include "constants.h"
#define PRECOUNT 1
#define POSTCOUNT 2

// Configuration
const uint16_t ledFadeInterval = 3000; // In ms
const uint16_t ledBlinkInterval = 500; // In ms
const uint16_t btnDebounce = 100;

// Defining the pins
const uint8_t pin9VMeasure = A0,
  
  pinJack1 = 22,
  pinJack2 = 23,
  pinSourceSw1 = 24,
  pinSourceSw2 = 25,
  pinSourceLed1 = 2,
  pinSourceLed2 = 3,
  
  pinModeSw = 38,
  pinModeLedGr = 6,
  pinModeLedRd = 7,
  
  pinKeySw = 39,
  pinAuthLedGr = 4,
  pinAuthLedRd = 5,
  
  pinIgnSw = 40,
  pinIgnLed = 8,
  pinIgn1 = 41,
  pinIgn2 = 42,
  
  pinKbStar = 26,
  pinKb7 = 27,
  pinKb4 = 28,
  pinKb1 = 29,
  pinKb0 = 30,
  pinKb8 = 31,
  pinKb5 = 32,
  pinKb2 = 33,
  pinKbHash = 34,
  pinKb9 = 35,
  pinKb6 = 36,
  pinKb3 = 37,
  
  pinSevSegDout = 43,
  pinSevSegLtch = 44,
  pinSevSegClk = 45,

  pinLcdRs = 46,
  pinLcdE = 47,
  pinLcdD4 = 48,
  pinLcdD5 = 49,
  pinLcdD6 = 50,
  pinLcdD7 = 51;



// Defining classes for output and input
class led {
  private:
    uint8_t pin;
    bool state = false;
    bool blinking = false;
    bool fading = false;
  public:
    static uint32_t lastBlinked, lastFaded; // Yet to be made private
    static int16_t faderVal; // Yet to be made private
    static led* fxLeds[];
    static void iterateAll();
    led(uint8_t);
    void dVal(bool);
    void blink();
    void fade();
};
class btn {
  private:
    uint8_t pin;
    int8_t triggerEdge;
    uint32_t lastTriggered;
    bool currentState;
    void (*runOnTrigger)();
  public:
    static btn* fxBtns[];
    btn(uint8_t, void (*)(), int8_t);
    static void iterateAll();
};
class State {
  public:
    virtual void authorize();
    virtual void pressIgnSw();
    virtual void keyboardContinue();
    virtual void keyboardBack();
    virtual void keyboardBtn(uint8_t);
    virtual void tick();
};
class StateRomReset: public State {
  private:
    StateRomReset() {}
    static StateRomReset *instance;
  public:
    static State *getInstance();
    void authorize () {}
    void pressIgnSw() {}
    void keyboardContinue();
    void keyboardBack() {}
    void keyboardBtn(uint8_t) {}
    void tick() {}
};
class StateStart: public State {
  private:
    StateStart() {}
    static StateStart *instance;
    uint8_t currentPlace;
    uint32_t currentInput;
  public:
    static State *getInstance();
    void authorize () {}
    void pressIgnSw() {}
    void keyboardContinue();
    void keyboardBack();
    void keyboardBtn(uint8_t);
    void tick();
};
class StateMode: public State {
  private:
    StateMode() {}
    static StateMode *instance;
    void redrawArrows();
    uint8_t currentlySelected;
  public:
    static State *getInstance();
    void authorize () {}
    void pressIgnSw () {}
    void keyboardContinue();
    void keyboardBack() {}
    void keyboardBtn(uint8_t);
    void tick() {}
};
class StateChangePwd: public State {
  private:
    StateChangePwd() {}
    static StateChangePwd *instance;
    uint8_t digits[18];
    uint8_t pos;
    bool flagForNext;
  public:
    static State *getInstance();
    void authorize () {}
    void pressIgnSw () {}
    void keyboardContinue();
    void keyboardBack();
    void keyboardBtn(uint8_t);
    void tick() {}
};
class StateAuth2: public State {
  private:
    StateAuth2() {}
    static StateAuth2 *instance;
    uint8_t pos;
    uint8_t digits[6];
  public:
    static State *getInstance();
    void authorize () {}
    void pressIgnSw () {}
    void keyboardContinue ();
    void keyboardBack ();
    void keyboardBtn (uint8_t);
    void tick () {}
};
class StateEnterTime: public State {
  private:
    StateEnterTime() {}
    static StateEnterTime *instance;
    uint8_t pos;
    uint8_t digits[5];
  public:
    static State *getInstance();
    void authorize () {}
    void pressIgnSw () {}
    void keyboardContinue ();
    void keyboardBack ();
    void keyboardBtn (uint8_t);
    void tick () {}
};
class StateCounting: public State {
  private:
    StateCounting() {}
    static StateCounting *instance;
    void displayTime ();
    bool authed, counting;
    uint32_t originalTime;
    uint8_t prevLines;
    uint8_t backBtnTimer;
  public:
    static State *getInstance();
    void authorize ();
    void pressIgnSw ();
    void keyboardContinue () {}
    void keyboardBack ();
    void keyboardBtn (uint8_t) {}
    void tick ();
};
class StateIgnited: public State {
  private:
    StateIgnited() {}
    static StateIgnited *instance;
  public:
    static State *getInstance();
    void authorize () {}
    void pressIgnSw () {}
    void keyboardContinue () {}
    void keyboardBack ();
    void keyboardBtn (uint8_t) {}
    void tick () {}
};
class StateLocked: public State {
  private:
    StateLocked() {}
    static StateLocked *instance;
    uint8_t currentPlace = 0;
    uint8_t currentInput[10] = {};
    uint8_t timeWaiting = 0; // in 1/10 seconds
  public:
    static State *getInstance();
    void authorize () {}
    void pressIgnSw () {}
    void keyboardContinue ();
    void keyboardBack ();
    void keyboardBtn (uint8_t);
    void tick ();
};


// Prototypes
namespace actions {
  State *state;
  bool authed2 = false;
  bool inTestMode = true;
  uint8_t mode = 0;
  uint32_t countdownTime = 0; // In 1/10s
  void authorize() {state->authorize();}
  void pressIgnSw() {state->pressIgnSw();}
  void keyboardContinue() {state->keyboardContinue();}
  void keyboardBack() {state->keyboardBack();}
  void keyboardBtn(uint8_t a) {state->keyboardBtn(a);}
  void tick() {state->tick();}
  void iterateStaticSwitches();
  void ignite();
}
namespace SevSeg {
  void init();
  void off(bool = false);
  void writeNum (uint16_t, uint8_t);
  void toDisplay(uint8_t*);
  uint8_t transform(char);
}
namespace Crypt {
  bool isCorrect(uint8_t*);
}


// Initialize active components
LiquidCrystal lcd(pinLcdRs, pinLcdE, pinLcdD4,
pinLcdD5, pinLcdD6, pinLcdD7);

led* sourceLed1 = new led(pinSourceLed1);
led* sourceLed2 = new led(pinSourceLed2);
led* modeLedGr = new led(pinModeLedGr);
led* modeLedRd = new led(pinModeLedRd);
led* authLedGr = new led(pinAuthLedGr);
led* authLedRd = new led(pinAuthLedRd);
led* ignLed = new led(pinIgnLed);
led* led::fxLeds[] = {sourceLed1, sourceLed2, modeLedGr, modeLedRd,
                     authLedGr, authLedRd, ignLed};

btn* keySw = new btn(pinKeySw, actions::authorize, RISING);
btn* ignSw = new btn(pinIgnSw, actions::pressIgnSw, FALLING);
btn* kbStar = new btn(pinKbStar, actions::keyboardBack, RISING);
btn* kbHash = new btn(pinKbHash, actions::keyboardContinue, RISING);
btn* kb0 = new btn(pinKb0, []()->void {actions::keyboardBtn(0);}, RISING);
btn* kb1 = new btn(pinKb1, []()->void {actions::keyboardBtn(1);}, RISING);
btn* kb2 = new btn(pinKb2, []()->void {actions::keyboardBtn(2);}, RISING);
btn* kb3 = new btn(pinKb3, []()->void {actions::keyboardBtn(3);}, RISING);
btn* kb4 = new btn(pinKb4, []()->void {actions::keyboardBtn(4);}, RISING);
btn* kb5 = new btn(pinKb5, []()->void {actions::keyboardBtn(5);}, RISING);
btn* kb6 = new btn(pinKb6, []()->void {actions::keyboardBtn(6);}, RISING);
btn* kb7 = new btn(pinKb7, []()->void {actions::keyboardBtn(7);}, RISING);
btn* kb8 = new btn(pinKb8, []()->void {actions::keyboardBtn(8);}, RISING);
btn* kb9 = new btn(pinKb9, []()->void {actions::keyboardBtn(9);}, RISING);
btn* btn::fxBtns[] = {keySw, ignSw, kbStar, kbHash, kb0, kb1, kb2, kb3,
                     kb4, kb5, kb6, kb7, kb8, kb9};


// Initialize system-wide vars
uint32_t lastRunUpdater = 0;


void setup() {
  pinMode(pinIgn1, OUTPUT); digitalWrite(pinIgn1, HIGH);
  pinMode(pinIgn2, OUTPUT); digitalWrite(pinIgn2, HIGH);
  pinMode(pinJack1, INPUT_PULLUP);
  pinMode(pinJack2, INPUT_PULLUP);
  pinMode(pinSourceSw1, INPUT_PULLUP);
  pinMode(pinSourceSw2, INPUT_PULLUP);
  pinMode(pinModeSw, INPUT_PULLUP);
  lcd.begin(20, 4);
  SevSeg::init();
  if (EEPROM.read(ROM_RESET) != EEPROM_VERSION)
    actions::state = StateRomReset::getInstance();
  else if (EEPROM.read(ROM_FAILED_COUNTER) >= 3)
    actions::state = StateLocked::getInstance();
  else
    actions::state = StateStart::getInstance();
}




void loop() {
  // Do the repetetive tasks
  led::iterateAll();
  btn::iterateAll();
  actions::iterateStaticSwitches(); // modeSw, SourceSw, Jack
  
  // Run every 10ths second
  if (millis() - lastRunUpdater >= 100) {
    lastRunUpdater = millis();
    actions::tick();
  }
}
