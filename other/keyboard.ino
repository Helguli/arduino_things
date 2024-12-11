#include <Keyboard.h>
#include <Keyboard_hu_HU.h>

#define BUTTON_PIN 3


void rdDelay() {
  delay(100);
}

void rdLongerDelay() {
  for(int i = 0; i < 5; i++) rdDelay();
}

void rdWriteText(String text) {
  Keyboard.print(text);
  rdDelay();
}

void rdTypeKey(uint8_t key)
{
  Keyboard.press(key);
  rdDelay();
  Keyboard.release(key);
  rdDelay();
}

void rdKeyCombination(uint8_t holdKey, uint8_t targetKey) {
  Keyboard.press(holdKey);
  rdDelay();
  Keyboard.press(targetKey);
  rdDelay();
  Keyboard.releaseAll();
  rdDelay();
}

void rdKeyCombination(uint8_t holdKey1, uint8_t holdKey2, uint8_t targetKey) {
  Keyboard.press(holdKey1);
  rdDelay();
  rdKeyCombination(holdKey2, targetKey);
}

void rdKeyCombination(uint8_t holdKey1, uint8_t holdKey2, uint8_t holdKey3, uint8_t targetKey) {
  Keyboard.press(holdKey1);
  rdDelay();
  rdKeyCombination(holdKey2, holdKey3, targetKey);
}

void rdGuiCombination(uint8_t c) {
  rdKeyCombination(KEY_LEFT_GUI, c);
}

void rdAltCombination(uint8_t c) {
  rdKeyCombination(KEY_LEFT_ALT, c);
}

void rdCtrlCombination(uint8_t c) {
  rdKeyCombination(KEY_LEFT_CTRL, c);
}

void rdShiftCombination(uint8_t c) {
  rdKeyCombination(KEY_LEFT_SHIFT, c);
}

void rdRun(String program) {
  rdGuiCombination('r');
  Keyboard.print(program);
  rdDelay();
  rdTypeKey(KEY_RETURN);
}

void rdPrintScreen() {
  // some machines use 206 key as the PrtScreen key
  // others might use 229, and others might use both so
  // we use both instructions
  rdTypeKey(206);
  rdTypeKey(229);
  Keyboard.print(F("h"));
  rdDelay();
  Keyboard.print(F("b"));
  rdDelay();
}

void rdOpenJavascriptConsole() {
  rdKeyCombination(KEY_LEFT_CTRL, KEY_LEFT_SHIFT, 'i');
}

void rdHideWindow() {
  rdAltCombination(' ');
  Keyboard.print(F("M"));
  rdDelay();
  Keyboard.press(KEY_DOWN_ARROW);
  // 100 should be enough to guarantee the window is as low as possible
  // also please notice that 100 is not the real number of strokes since
  // some of the strokes are ignored.
  for(int i = 0; i < 10; i++) rdLongerDelay();;
  Keyboard.release(KEY_DOWN_ARROW);
  // return repositions the cursor back to its original position
  rdTypeKey(KEY_RETURN);
}

void rdShowDesktop() {
  rdGuiCombination('d');
}

void rdPaste() {
  rdCtrlCombination('v');
}

void rdCut() {
  rdCtrlCombination('x');
}

void rdCopy() {
  rdCtrlCombination('c');
}

void rdAcceptWindowsSmartScreen() {
  // Wait untill smart screen shows up
  rdLongerDelay();
  rdTypeKey(KEY_LEFT_ARROW);
  rdDelay();
  Keyboard.print(F(" "));
  rdDelay();
}

void rdOpenCommandPrompt(boolean admin) {
  if (admin) {
    rdGuiCombination('x');
    Keyboard.print(F("a"));
    rdAcceptWindowsSmartScreen();
  } else {
    rdRun("cmd");
  }
}

void rdOpenCommandPrompt() {
  rdOpenCommandPrompt(false);
}

void rdChangeKeyboardLayout() {
  rdAltCombination(KEY_LEFT_SHIFT);
}

void rdPowershellRun(String scripts) {
  char delimiter = '\n';
  String finalScript = "powershell ";
  while (scripts.indexOf('\n') > 0) {
    finalScript = finalScript + "(" + scripts.substring(0, scripts.indexOf('\n')) + ") ; ";
    scripts = scripts.substring(scripts.indexOf('\n') + 1);
  }
  finalScript = finalScript + "(" + scripts + ")";
  rdRun(finalScript);
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Keyboard.begin(KeyboardLayout_hu_HU);
  //Keyboard.begin();
}

void enter() {
    Keyboard.write(KEY_RETURN);
    rdDelay();
}

void tab() {
    Keyboard.write(KEY_TAB);
    rdDelay();
}

void stab() {
    rdShiftCombination(KEY_TAB);
    rdDelay();
}

void left() {
    Keyboard.write(KEY_LEFT_ARROW);
    rdDelay();
}

void right() {
    Keyboard.write(KEY_RIGHT_ARROW);
    rdDelay();
}

void up() {
    Keyboard.write(KEY_UP_ARROW);
    rdDelay();
}

void down() {
    Keyboard.write(KEY_DOWN_ARROW);
    rdDelay();
}

void space() {
    Keyboard.write(' ');
    rdDelay();
}

void tab(int num) {
    for (int i = 0; i < num; i++) tab();
}

void stab(int num) {
    for (int i = 0; i < num; i++) stab();
}

void left(int num) {
    for (int i = 0; i < num; i++) left();
}

void right(int num) {
    for (int i = 0; i < num; i++) right();
}

void up(int num) {
    for (int i = 0; i < num; i++) up();
}

void down(int num) {
    for (int i = 0; i < num; i++) down();
}

void space(int num) {
    for (int i = 0; i < num; i++) space();
}

void win() {
    rdTypeKey(KEY_LEFT_GUI);
    rdDelay();
}

void esc() {
    rdTypeKey(KEY_ESC);
    rdDelay();
}

void wait() {
    while(digitalRead(BUTTON_PIN) != LOW) {
        delay(50);
    }
    delay(200);
}

void wait(uint8_t key) {
    while(digitalRead(BUTTON_PIN) != LOW) {
        rdTypeKey(key);
        rdDelay();
    }
    delay(200);
}

void openPowerShell(bool admin) {
    rdGuiCombination('x');
    Keyboard.print("w");
    if (admin) {
        down();
    }
    enter();
    rdLongerDelay();
    if (admin) {
        rdAcceptWindowsSmartScreen();
    }
}

void demo() {
    wait();
    win();
    rdLongerDelay();
    rdLongerDelay();
    rdLongerDelay();
    rdLongerDelay();
    Keyboard.print("arduino");
    rdLongerDelay();
    rdLongerDelay();
    rdLongerDelay();
    enter();
}

void loop() {
    //restoreRxSetup();
    //programSetup();
    demo();
}
