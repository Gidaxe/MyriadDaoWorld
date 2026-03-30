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
int long_press_threshold = 3000;
bool s_pair;

// Menu
unsigned long time_buffer = 0;
int time_buffer_threshold = 1000;

typedef enum {
  STOPWATCH,
  MENU,
  CONTRAST,
  LAP_HISTORY
} Mode;

Mode current_mode = STOPWATCH;

typedef struct node {
    char *name;
    char *lap;
    Mode mode;
    struct node *next;
} Node;

Node *selected_mode;

Node stopwatch_mode;

Node contrast_mode;

Node lap_history_mode;


// Contrast
int initial_contrast = 64;
int contrast = initial_contrast;
int contrast_bar_level = 0;
int min_luminosity = 128;

// Lap History
Node *previous_lap;
Node *selected_lap;
Node head;

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
void update_lap_history(char* lap) {
  //updates the lap history with mode from the stopwatch
  Node *new_lap =malloc(sizeof(Node));
  new_lap->name = NULL;
  new_lap->next = NULL;

  previous_lap->name = lap;
  previous_lap->next = new_lap;

  previous_lap = new_lap;
}

void display_lap_history() {
  //displays and allows us to browse through the lap history
  //display selected_lap and it's next lap. When at the end of the chain list, go back to &head
  if (selected_lap->name != NULL) {  
    lcd.setCursor(1, 0);
    lcd.print(selected_lap->name); //handle case of head not initialized
    }
  else {
    lcd.setCursor(0, 0);
    lcd.print("                ");
  }
  // if (selected_lap->next != NULL) {
  //   lcd.setCursor(1, 1);
  //   lcd.print(selected_lap->next->name);
  // }
  // else {
  //   lcd.setCursor(0, 1);
  //   lcd.print("                ");
  // }
}

Mode handle_history_event() {
  //handles button presses when in lap history mode
  ClickType click = click_handler();

  if (click == SHORT_PRESS) {
    if (selected_lap -> next != NULL) selected_lap = selected_lap -> next;
    else selected_lap = &head;
    lcd.clear();
    return LAP_HISTORY;
  }

  else if (click == NO_EVENT) {
    return LAP_HISTORY;
  }

  else if (click == LONG_PRESS) {
    lcd.clear();
    return MENU;
  }
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
  snprintf(lap, sizeof(lap), "%02d:%02d:%02d", h, m, s);

  char *lap_copy = malloc(sizeof(lap));
  strcpy(lap_copy, lap);

  update_lap_history(lap_copy);

  clock_start = millis();
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
    return selected_mode ->mode;
  }
}

void display_menu(Node *selected_mode) {
  //displays the POMODORO ONE's different functionalities
  lcd.setCursor(0, 0);
  lcd.print(selected_mode->name);
}


//Contrast
void display_contrast() {
  //displays the current contrast of the lcd screen
  if (contrast > min_luminosity) contrast = 0;
  analogWrite(contrast_pin, contrast);

  contrast_bar_level = contrast / 8;

  lcd.setCursor(4, 0);
  lcd.print("CONTRAST");

  for (int i = 0; i < contrast_bar_level; i++) {
    lcd.setCursor(i, 1);
    lcd.write(255);
  }
}

Mode handle_contrast_event() {
  //handles button presses when in menu mode
  ClickType click = click_handler();
  if (click == LONG_PRESS) {
    lcd.clear();
    return MENU;
  }

  else if (click == NO_EVENT) {
    return CONTRAST;
  }

  else if (click == SHORT_PRESS) {
    lcd.clear();
    contrast += 8;
    return CONTRAST;
  }
} 


void setup() {
  //Initial setup
  lcd.begin(16, 2);
  Serial.begin(9600);

  analogWrite(contrast_pin, initial_contrast);

  pinMode(contrast_pin, OUTPUT);
  pinMode(button, INPUT);

  stopwatch_mode.name = "STOPWATCH MODE";
  stopwatch_mode.mode = STOPWATCH;
  stopwatch_mode.next = &contrast_mode;

  contrast_mode.name = "CONTRAST MODE";
  contrast_mode.mode = CONTRAST;
  contrast_mode.next = &lap_history_mode;

  lap_history_mode.name = "LAP HISTORY";
  lap_history_mode.mode = LAP_HISTORY;
  lap_history_mode.next = &stopwatch_mode;

  selected_mode = &stopwatch_mode;

  head.name = NULL;
  head.next = NULL;

  previous_lap = &head;
  selected_lap = &head;

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
      break;

    case MENU:
      display_menu(selected_mode);
      current_mode = handle_menu_event();
      break;

    case CONTRAST:
      display_contrast();
      current_mode = handle_contrast_event();
      break;

    case LAP_HISTORY:
      display_lap_history();
      current_mode = handle_history_event();
      break;
  }

}

//TODO:
// menu to pick between: contrast, stopwatch
// seperate declaration from initialization
//create new clock designs
