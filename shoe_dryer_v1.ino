#include <VarSpeedServo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// -------------------- LCD --------------------
// Initialize LCD (I2C address 0x27, 16x2)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// -------------------- KEYPAD --------------------
const byte ROWS = 4;
const byte COLS = 4;

// Keypad layout
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Keypad pin connections
byte rowPins[ROWS] = {A0, A1, A2, A3};
byte colPins[COLS] = {A4, A5, 2, 3};

// Create keypad object
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// -------------------- MOTOR + PELTIER --------------------
// These pins control the motor driver (H-bridge)
// IMPORTANT: Peltier modules are connected to the motor output,
// so when the motor is powered, Peltiers are also activated
const int motorPin = 6;
const int motorPin1 = 7;
const int motorPin2 = 12;
const int motorPin3 = 13;

// -------------------- LED INDICATORS --------------------
const int ledRun = 10;   // Turns ON when system is running
const int ledDone = 11;  // Turns ON when process is finished

// -------------------- SERVO --------------------
VarSpeedServo myservo1;
VarSpeedServo myservo2;

const int servoPin1 = 8;
const int servoPin2 = 9;

// -------------------- TIME CONTROL --------------------
unsigned long selectedTime = 0;   // Stores selected duration (ms)
unsigned long startTime = 0;      // Stores start time
bool motorRunning = false;        // System state flag

// Stores user input from keypad
String inputTime = "";

// -------------------- SETUP --------------------
void setup() {

  // Set motor control pins as outputs
  pinMode(motorPin, OUTPUT);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);

  // Set LED pins
  pinMode(ledRun, OUTPUT);
  pinMode(ledDone, OUTPUT);

  // Attach servos
  myservo1.attach(servoPin1);
  myservo2.attach(servoPin2);

  // Set initial servo position
  myservo1.write(0, 100);
  myservo2.write(0, 100);

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  lcd.print("Enter Time (min)");
}

// -------------------- MAIN LOOP --------------------
void loop() {

  // Read keypad input
  char key = keypad.getKey();

  // -------- INPUT MODE --------
  // Only allow input when system is not running
  if (!motorRunning) {

    if (key) {

      // If number key is pressed → build time input
      if (key >= '0' && key <= '9') {
        inputTime += key;

        lcd.clear();
        lcd.print("Time: ");
        lcd.print(inputTime);
        lcd.print(" min");
      }

      // '*' key clears input
      else if (key == '*') {
        inputTime = "";
        lcd.clear();
        lcd.print("Cleared");
        delay(500);

        lcd.clear();
        lcd.print("Enter Time");
      }

      // '#' key confirms input and starts system
      else if (key == '#') {

        if (inputTime.length() > 0) {

          // Convert minutes to milliseconds
          int minutes = inputTime.toInt();
          selectedTime = (unsigned long)minutes * 60UL * 1000UL;

          lcd.clear();
          lcd.print("Set: ");
          lcd.print(minutes);
          lcd.print(" min");

          delay(1000);

          // Start motor + Peltier system
          startMotor();
        }
      }
    }
  }

  // -------- RUNNING MODE --------
  if (motorRunning) {

    // Calculate remaining time in seconds
    unsigned long remaining = (selectedTime - (millis() - startTime)) / 1000;

    // Display countdown
    lcd.setCursor(0, 1);
    lcd.print("Left: ");
    lcd.print(remaining);
    lcd.print("s   ");

    // Stop system when time is completed
    if (millis() - startTime >= selectedTime) {
      stopMotor();
    }
  }
}

// -------------------- START FUNCTION --------------------
void startMotor() {

  motorRunning = true;
  startTime = millis();

  // Motor forward direction using H-bridge
  // This also powers the Peltier modules connected to motor output
  digitalWrite(motorPin, HIGH);
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, HIGH);
  digitalWrite(motorPin3, LOW);

  // Turn ON running LED
  digitalWrite(ledRun, HIGH);
  digitalWrite(ledDone, LOW);

  // Move servos to active position
  myservo1.write(90, 50);
  myservo2.write(90, 50);

  lcd.clear();
  lcd.print("Motor Running");
  lcd.setCursor(0, 1);
  lcd.print("Peltier ON"); // Indicate thermal system active
}

// -------------------- STOP FUNCTION --------------------
void stopMotor() {

  // Stop motor and Peltier modules
  digitalWrite(motorPin, LOW);
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);

  // Update LEDs
  digitalWrite(ledRun, LOW);
  digitalWrite(ledDone, HIGH);

  // Reset servos
  myservo1.write(0, 50);
  myservo2.write(0, 50);

  motorRunning = false;
  inputTime = "";

  lcd.clear();
  lcd.print("Finished");

  delay(2000);

  lcd.clear();
  lcd.print("Enter Time");
}