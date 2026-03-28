#include <LiquidCrystal.h>

// RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int initialContrast = 10;
int contrast = initialContrast;
int contrastPin = 9;
int contrastButton = 7;

unsigned long t_pressed = 0;
int s_tot = 0;
int s = 0;
int m = 0;
int h = 0;

bool s_pair;
bool lap = 1;
bool menu = 0;
int press_trigger_lap = 10000;

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);

  pinMode(contrastPin, OUTPUT);
  pinMode(contrastButton, INPUT_PULLUP);
}

void loop() {

  //Button handling (choose mode)
  if (digitalRead(contrastButton) == HIGH) {

    if (t_pressed == 0) t_pressed = millis();
    else if ((millis() - t_pressed > press_trigger_lap)) menu = 1;

    contrast += 5;
    delay(200);
  }

  else t_pressed = 0;

  if (contrast >= 130) contrast = initialContrast;

  analogWrite(contrastPin, contrast);

  s_tot = (int)(millis() / 1000);
  s = s_tot % 60;
  m = (int)(s_tot / 60) % 60;
  h = (int)(s_tot / 3600);

  s_pair = (millis() / 500) % 2 == 0;

  if (lap) {
    // 🟦 First row: title
    lcd.setCursor(0, 0);
    if (s % 2 == 0) lcd.print("-");
    else lcd.print("|");
    lcd.print(" POMODORO ONE");
    if (s % 2 == 0) lcd.print(" -");
    else lcd.print(" |");

    // 🟩 Second row: centered time
    lcd.setCursor(4, 1);

    // Leading zeros (important)
    if (h < 10) lcd.print("0");
    lcd.print(h);

    // Blinking colon effect
    if (s_pair) lcd.print(":");
    else lcd.print(" ");

    if (m < 10) lcd.print("0");
    lcd.print(m);

    if (s_pair) lcd.print(":");
    else lcd.print(" ");

    if (s < 10) lcd.print("0");
    lcd.print(s);
    Serial.print(contrast);
    Serial.print("  pin7: ");
    Serial.println(digitalRead(contrastButton));
  }
}

//TODO:
// menu to pick between: contrast, stopwatch
//center the clock (Done)
//create new clock designs
