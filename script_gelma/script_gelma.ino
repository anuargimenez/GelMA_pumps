// Include required libraries
#include <Wire.h> // for I2C communication
#include <LiquidCrystal_I2C.h> // for LCD display
#include <Keypad.h> // for keypad input

// Define constant values
#define ROWS 4 // number of rows in keypad
#define COLS 4 // number of columns in keypad
#define RELAY_PIN 2 // pin number for relay
#define LEVEL_SENSOR_PIN 12 // pin number for level sensor

// Pump parameters
int Qmax_pump = 150; // Max Flow rate of the pump(l/h)
long volume = 0;
long volume_glass = 0;
int keyPressed = 0;
float rem_Vol = 835; // Remanent Volume needed to ensure that pump is allways filled

// Define arrays and variables
const byte ROW_PINS[ROWS] = { 3, 4, 5, 6 }; // array of row pin numbers
const byte COL_PINS[COLS] = { 7, 8, 9, 10 }; // array of column pin numbers
char keys[ROWS][COLS] = { // array of characters corresponding to each keypad button
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
int days = 0; // number of days in timer
int hours = 0; // number of hours in timer
int minutes = 0; // number of minutes in timer
int seconds = 0; // number of seconds in timer
int selectedMenu = 0; // selected menu option (0-3)

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD object with I2C address and dimensions

Keypad keypad = Keypad(makeKeymap(keys), ROW_PINS, COL_PINS, ROWS, COLS); // keypad object with defined pin configurations

// Setup function to be run once at startup
void setup() {
  pinMode(LEVEL_SENSOR_PIN, INPUT_PULLUP); // set pin in high mode
  pinMode(RELAY_PIN, OUTPUT);

  Serial.begin(9600);
  lcd.init(); // initialize LCD
  lcd.backlight(); // turn on backlight
  lcd.clear(); // clear LCD screen
  lcd.setCursor(0, 0); // set cursor to top-left corner
  lcd.print("BTELab"); // display startup message
  delay(2500); // wait for 2.5 seconds

  // Select Volume to be pumped
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Volume (mL): ");

  while (keyPressed != '#') {
    keyPressed = keypad.getKey();
    if (keyPressed >= '0' && keyPressed <= '9') {
      if (volume * 10 + (keyPressed - '0') <= 99999) { // check if adding the new digit will exceed 99999
        volume = volume * 10 + (keyPressed - '0');
        lcd.setCursor(0, 1);
        lcd.print(volume, 1);
      }
    } else if (keyPressed == '*') {
      volume = volume / 10;
      lcd.setCursor(0, 1);
      lcd.print("          ");
      lcd.setCursor(0, 1);
      lcd.print(volume, 1);
    }
  }
  volume_glass = volume - rem_Vol;
  lcd.clear();
  lcd.setCursor(0, 1);
  delay(500);

  lcd.clear(); // clear LCD screen
  lcd.setCursor(0, 0); // set cursor to top-left corner
  updateDisplay(); // update display with current timer values
}

// Loop function to be run continuously
void loop() {

  // Timer
  char key = keypad.getKey(); // read keypad input

  if (key != NO_KEY) { // if a key is pressed
    switch (key) {
      case 'A': // if A is pressed
        selectedMenu = 0; // set selected menu option to days
        break;
      case 'B': // if B is pressed
        selectedMenu = 1; // set selected menu option to hours
        break;
      case 'C': // if C is pressed
        selectedMenu = 2; // set selected menu option to minutes
        break;
      case 'D': // if D is pressed
        selectedMenu = 3; // set selected menu option to seconds
        break;
      case '*': // if * is pressed
        resetTimer(); // reset the timer to 0 for the selected menu option
        break;
      case '#': // if # is pressed
        startTimer(); // start the timer with the selected values
        break;
      default: // for any other key pressed
        updateTimer(key); // update
        break;
    }
    updateDisplay();
  }

  delay(100);
}

// This function updates the LCD display with the current time remaining and selected menu item
void updateDisplay() {
  lcd.setCursor(0, 1);
  lcd.print("                "); // clear previous menu selection

  // Display the days, hours, minutes, and seconds on the first line of the LCD display
  lcd.setCursor(0, 0);
  lcd.print(days < 10 ? "0" : ""); // Add leading zero if necessary
  lcd.print(days);
  lcd.print("d ");
  lcd.print(hours < 10 ? "0" : ""); // Add leading zero if necessary
  lcd.print(hours);
  lcd.print("h ");
  lcd.print(minutes < 10 ? "0" : ""); // Add leading zero if necessary
  lcd.print(minutes);
  lcd.print("m ");
  lcd.print(seconds < 10 ? "0" : ""); // Add leading zero if necessary
  lcd.print(seconds);
  lcd.print("s");

  // Display the selected menu item on the second line of the LCD display
  switch (selectedMenu) {
    case 0:
      lcd.setCursor(0, 1);
      lcd.print(">Days    ");
      break;
    case 1:
      lcd.setCursor(0, 1);
      lcd.print(">Hours   ");
      break;
    case 2:
      lcd.setCursor(0, 1);
      lcd.print(">Minutes ");
      break;
    case 3:
      lcd.setCursor(0, 1);
      lcd.print(">Seconds ");
      break;
  }
}

// This function updates the days, hours, minutes, or seconds value based on the selected menu item and user input
void updateTimer(char key) {
  if (selectedMenu == 0) { // If the "Days" menu item is selected, update the "days" value
    days = days * 10 + (key - '0'); // Add the new digit to the end of the current "days" value
    days %= 30; // Ensure that "days" value is less than 30
  } else if (selectedMenu == 1) { // If the "Hours" menu item is selected, update the "hours" value
    hours = hours * 10 + (key - '0'); // Add the new digit to the end of the current "hours" value
    hours %= 24; // Ensure that "hours" value is less than 24
  } else if (selectedMenu == 2) { // If the "Minutes" menu item is selected, update the "minutes" value
    minutes = minutes * 10 + (key - '0'); // Add the new digit to the end of the current "minutes" value
    minutes %= 60; // Ensure that "minutes" value is less than 60
  } else if (selectedMenu == 3) { // If the "Seconds" menu item is selected, update the "seconds" value
    seconds = seconds * 10 + (key - '0'); // Add the new digit to the end of the current "seconds" value
    seconds %= 60; // Ensure that "seconds" value is less than 60
  }
}

// This function resets the days, hours, minutes, or seconds value to zero
void resetTimer() {
  if (selectedMenu == 0) {
    days = 0;
  } else if (selectedMenu == 1) {
    hours = 0;
  } else if (selectedMenu == 2) {
    minutes = 0;
  } else if (selectedMenu == 3) {
    seconds = 0;
  }
}

// This function starts the timer
void startTimer() {
  // Calculate the total time in seconds
  unsigned long totalTime = (days * 86400) + (hours * 3600) + (minutes * 60) + seconds;

  // Loop until the timer reaches zero
  while (totalTime > 0) {
    // Clear the LCD screen and set the cursor to the top left
    lcd.clear();
    lcd.setCursor(0, 0);
    // Print the "Time left:" message on the first line
    lcd.print("Time left: ");
    // Call the formatTime function to format the remaining time and print it on the second line
    lcd.setCursor(0, 1);
    lcd.print(formatTime(totalTime));
    // Wait for one second
    delay(957); // 1000 ms - 42.5 ms (approx. time it takes to complete the residual code loop - calibrated with crono)
    delayMicroseconds(500);
    // Decrement the total time by one second
    totalTime--;
  }

  // Clear the LCD screen and set the cursor to the second line
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Timer Done!");
  lcd.setCursor(0, 1);
  lcd.print("Pumping...");
  // Wait for 1 second
  delay(1000);

  // Turn off the relay
  activatePumps();
}

// This function formats the remaining time as a string
String formatTime(unsigned long totalTime) {
  // Calculate the number of days, hours, minutes, and seconds left
  int daysLeft = totalTime / 86400;
  int hoursLeft = (totalTime % 86400) / 3600;
  int minutesLeft = (totalTime % 3600) / 60;
  int secondsLeft = totalTime % 60;

  // Create an empty string to store the formatted time
  String formattedTime = "";
  // Add the number of days left, followed by "d ", to the formatted time string
  formattedTime += String(daysLeft, DEC);
  formattedTime += "d ";
  // Add the number of hours left, followed by "h ", to the formatted time string
  formattedTime += String(hoursLeft, DEC);
  formattedTime += "h ";
  // Add the number of minutes left, followed by "m ", to the formatted time string
  formattedTime += String(minutesLeft, DEC);
  formattedTime += "m ";
  // Add the number of seconds left, followed by "s", to the formatted time string
  formattedTime += String(secondsLeft, DEC);
  formattedTime += "s";

  // Return the formatted time string
  return formattedTime;
}

void activatePumps() {

  // calculate removed liters
  float pump_time = volume_glass * 60 * 60 / (Qmax_pump);

  float startTime = 0; // variable to store the start time
  float elapsedTime = 0; // variable to store the elapsed time
  bool isTriggered = false; // flag to track if the level sensor has been triggered

  // turn on the relay to activate the pump and record the start time
  digitalWrite(RELAY_PIN, HIGH);
  startTime = millis();

  // run the loop for up to pump_time seconds or until the level sensor is triggered
  while (elapsedTime < pump_time && !isTriggered) {
    // check if the level sensor has been triggered
    if (digitalRead(LEVEL_SENSOR_PIN) == HIGH) {
      isTriggered = true; // set the flag to true
    }
    delay(100); // wait for 100 milliseconds
    elapsedTime = millis() - startTime; // calculate the elapsed time
  }

  // turn off the relay to stop the pump
  digitalWrite(RELAY_PIN, LOW);

  while (true) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Done in: ");
    lcd.setCursor(0, 1);
    lcd.print(elapsedTime/1000);
    lcd.print("s");

    delay(10000); // wait before updating the display again
  }
}
