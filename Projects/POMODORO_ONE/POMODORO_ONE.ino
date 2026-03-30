#include <LiquidCrystal.h>

// Hardware
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // RS, E, D4, D5, D6, D7

int contrast_pin = 9;
int button = 7;
unsigned long pressed_time = 0;
typedef enum {
  NO_EVENT,
  SHORT_PRESS,
  LONG_PRESS
} ClickType;

// Stopwatch
unsigned long clock_start = 0;
int time;
int s = 0;
int m = 0;
int h = 0;
int long_press_threshold = 5000;
bool s_pair;

// Menu
unsigned long time_buffer = 0;
int time_buffer_threshold = 2000;

typedef enum {
  STOPWATCH,
  MENU,
  CONTRAST,
  LAP_HISTORY
} Mode;

Mode current_mode = STOPWATCH;

typedef struct node {
    char *name;
    Mode data;
    struct node *next;
} Node;

Node *selected_mode;

Node stopwatch_mode;

Node contrast_mode;

Node lap_history_mode;


// Contrast
int initial_contrast = 10;
int contrast = initial_contrast;

// Lap History
char lap_history[35][10];


//Hardware
ClickType click_handler() {
  //determines if the click of the button is short or long
  if (digitalRead(button) == HIGH) {
    unsigned long time = millis();

    if (pressed_time == 0 && time - time_buffer > time_buffer_threshold) pressed_time = time; //prevent the system from detecting a press 1 second right after a long press
    else if (time - pressed_time > long_press_threshold && pressed_time != 0 ) { //the second condition avoids detecting a long press just because time - 0 is a big
      Serial.println("Long Press");
      pressed_time = 0;
      time_buffer = time;
      return LONG_PRESS;
    }
  } 
  else {
    if (pressed_time != 0) {
      pressed_time = 0;
      Serial.println("Short Press");
      return SHORT_PRESS;
    }
  }

  return NO_EVENT;
}


//Lap History
void update_lap_history(char* lap_history) {
  //updates the lap history with data from the stopwatch
}

void display_lap_history(char* lap_history) {
  //displays and allows us to browse through the lap history
}

Mode handle_history_event() {
  //handles button presses when in lap history mode
}


//Stopwatch
void display_clock(unsigned long time) {
  //computes the seconds, minutes and hours elapsed during the lap and displays them
  s = time % 60;
  m = (time / 60) % 60;
  h = (time / 3600);

  s_pair = (millis() / 500) % 2 == 0;

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
}

Mode reset_clock(int s, int m, int h) {
  //resets the pomodoro lap to 0 and sends the ended lap info to lap history
  char lap[10];
  snprintf(lap, sizeof(lap), "%02d:%02d:%02d", s, m, h);
  update_lap_history(lap);

  clock_start = millis();
  Serial.println(clock_start);
  return STOPWATCH;
}

Mode handle_clock_event() {
  //handles button presses when in clock mode.
  ClickType click = click_handler();
  if (click == LONG_PRESS) {
    lcd.clear();
    return MENU;
  }

  else if (click == NO_EVENT) {
    return STOPWATCH;
  }

  else if (click == SHORT_PRESS) {
    return reset_clock(s, m, h);
  }
}


//Menu
Mode handle_menu_event() {
  //handles button presses when in menu mode
  ClickType click = click_handler();

  if (click == SHORT_PRESS) {
    selected_mode = selected_mode ->next;
    lcd.clear();
    return MENU;
  }

  else if (click == NO_EVENT) {
    return MENU;
  }

  else if (click == LONG_PRESS) {
    lcd.clear();
    return selected_mode ->data;
  }
}

void display_menu(Node *selected_mode) {
  //displays the POMODORO ONE's different functionalities
  lcd.setCursor(0, 0);
  lcd.print(selected_mode->name);
}


//Contrast
void display_contrast(int contrast) {
  //displays the current contrast of the lcd screen
  if (contrast >= 130) contrast = initial_contrast;
  analogWrite(contrast_pin, contrast);
}

Mode handle_contrast_event() {
  //handles button presses when in menu mode
}

void setup() {
  //Initial setup
  lcd.begin(16, 2);
  Serial.begin(9600);

  pinMode(contrast_pin, OUTPUT);
  pinMode(button, INPUT);

  stopwatch_mode.name = "STOPWATCH MODE";
  stopwatch_mode.data = STOPWATCH;
  stopwatch_mode.next = &contrast_mode;

  contrast_mode.name = "CONTRAST MODE";
  contrast_mode.data = CONTRAST;
  contrast_mode.next = &lap_history_mode;

  lap_history_mode.name = "LAP HISTORY";
  lap_history_mode.data = LAP_HISTORY;
  lap_history_mode.next = &stopwatch_mode;

  selected_mode = &stopwatch_mode;

}

void loop() {
  //Main loop
  delay(50);

  unsigned long m = millis();

  time = (m - clock_start) / 1000;

  switch (current_mode) {
    
    case STOPWATCH:
      display_clock(time);
      current_mode = handle_clock_event();
      // Serial.print(clock_start);
      // Serial.print(" ");
      // Serial.print(m);
      // Serial.print(" ");
      // Serial.println(time);
      break;

    case MENU:
      display_menu(selected_mode);
      current_mode = handle_menu_event();
      break;

    case CONTRAST:
      current_mode = STOPWATCH;
      break;

    case LAP_HISTORY:
      current_mode = STOPWATCH;
      break;
  }

}

//TODO:
// menu to pick between: contrast, stopwatch
// seperate declaration from initialization
//create new clock designs
