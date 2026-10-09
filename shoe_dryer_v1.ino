#include <VarSpeedServo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <SPI.h>
#include <MFRC522.h>

// -------------------- RFID --------------------
#define RST_PIN 9   // Reset pin
#define SS_PIN  4   // Slave Select pin
MFRC522 rfid(SS_PIN, RST_PIN);

// Replace with your authorized RFID card UID bytes
byte authorizedUID[] = {0xDE, 0xAD, 0xBE, 0xEF}; 
bool isAuthenticated = false; // Security flag

// -------------------- LCD --------------------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// -------------------- KEYPAD --------------------
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {A0, A1, A2, A3};
byte colPins[COLS] = {A4, A5, 2, 3};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// -------------------- MOTOR + PELTIER --------------------
// NOTE: motorPin2 moved from 12 to 5 to keep pin 12 free for RFID MISO!
const int motorPin  = 6;
const int motorPin1 = 7;
const int motorPin2 = 5;  
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
unsigned long selectedTime = 0;   
unsigned long startTime = 0;      
bool motorRunning = false;        

String inputTime = "";

// -------------------- SETUP --------------------
void setup() {
  // Initialize Serial (optional, for debugging UIDs)
  Serial.begin(9600);

  // Initialize SPI and RFID
  SPI.begin();
  rfid.PCD_Init();

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

  lcd.print("Scan RFID Tag");
}

// -------------------- MAIN LOOP --------------------
void loop() {

  // -------- STEP 1: SECURITY CHECK (RFID) --------
  if (!isAuthenticated && !motorRunning) {
    
    // Look for new RFID card
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      
      // Check if scanned card matches authorized UID
      if (checkCardUID()) {
        isAuthenticated = true;
        lcd.clear();
        lcd.print("Access Granted!");
        delay(1500);
        
        lcd.clear();
        lcd.print("Enter Time (min)");
      } else {
        lcd.clear();
        lcd.print("Access Denied!");
        delay(1500);
        
        lcd.clear();
        lcd.print("Scan RFID Tag");
      }
      
      // Halt PICC and stop crypto
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
    return; // Block further execution until authenticated
  }

  // -------- STEP 2: KEYPAD INPUT MODE --------
  char key = keypad.getKey();

  if (!motorRunning && isAuthenticated) {
    if (key) {
      if (key >= '0' && key <= '9') {
        inputTime += key;

        lcd.clear();
        lcd.print("Time: ");
        lcd.print(inputTime);
        lcd.print(" min");
      }
      else if (key == '*') {
        inputTime = "";
        lcd.clear();
        lcd.print("Cleared");
        delay(500);

        lcd.clear();
        lcd.print("Enter Time");
      }
      else if (key == '#') {
        if (inputTime.length() > 0) {
          int minutes = inputTime.toInt();
          selectedTime = (unsigned long)minutes * 60UL * 1000UL;

          lcd.clear();
          lcd.print("Set: ");
          lcd.print(minutes);
          lcd.print(" min");

          delay(1000);
          startMotor();
        }
      }
    }
  }

  // -------- STEP 3: RUNNING MODE --------
  if (motorRunning) {
    unsigned long remaining = (selectedTime - (millis() - startTime)) / 1000;

    lcd.setCursor(0, 1);
    lcd.print("Left: ");
    lcd.print(remaining);
    lcd.print("s   ");

    if (millis() - startTime >= selectedTime) {
      stopMotor();
    }
  }
}

// -------------------- CHECK RFID UID --------------------
bool checkCardUID() {
  if (rfid.uid.size != 4) return false; // Assuming a standard 4-byte UID tag
  
  for (byte i = 0; i < 4; i++) {
    if (rfid.uid.uidByte[i] != authorizedUID[i]) {
      return false;
    }
  }
  return true;
}

// -------------------- START FUNCTION --------------------
void startMotor() {
  motorRunning = true;
  startTime = millis();

  digitalWrite(motorPin, HIGH);
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, HIGH);
  digitalWrite(motorPin3, LOW);

  digitalWrite(ledRun, HIGH);
  digitalWrite(ledDone, LOW);

  myservo1.write(90, 50);
  myservo2.write(90, 50);

  lcd.clear();
  lcd.print("Motor Running");
  lcd.setCursor(0, 1);
  lcd.print("Peltier ON"); 
}

// -------------------- STOP FUNCTION --------------------
void stopMotor() {
  digitalWrite(motorPin, LOW);
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);

  digitalWrite(ledRun, LOW);
  digitalWrite(ledDone, HIGH);

  myservo1.write(0, 50);
  myservo2.write(0, 50);

  motorRunning = false;
  isAuthenticated = false; // Require RFID scan again for next cycle!
  inputTime = "";

  lcd.clear();
  lcd.print("Finished");

  delay(2000);

  lcd.clear();
  lcd.print("Scan RFID Tag");
}
