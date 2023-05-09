/*
  Welcome to Project Iron Lock v1.00!
          made by Rayes
  ░░░░░░░██████████████████░░░░░░░
  ░░░░████▓▓▓█▓▓▓▓▓▓▓▓█▓▓▓███░░░░░
  ░░░██▓▓█▓▓▓█▓▓▓▓▓▓▓▓█▓▓▓█▓▓█░░░░
  ░░██████████▓▓▓▓▓▓▓▓██████████░░
  ░░██──────███████████───────██░░
  ░███───────██▓▓▓▓▓▓█────────███░
  ░████───────█▓▓▓▓▓▓█───────████░
  ░█▓██───────█▓▓▓▓▓▓█───────██▓█░
  ░██▓█───────█▓▓▓▓▓▓█───────█▓██░
  ████▓█──────█▓▓▓▓▓▓█──────█▓████
  █▓██▓█──────▀██████▀──────█▓██▓█
  █▓██▓█────────────────────█▓██▓█
  █▓████────────────────────████▓█
  █▓██▀──────────────────────▀██▓█
  █▓██──█▀▀▀▀▄▄──────▄▄▀▀▀▀█──██▓█
  ███───█─────▀██▄▄██▀─────█───███
  ░██───▀█▄▄▄▄█▀────▀█▄▄▄▄█▀───██░
  ░███────────────────────────███░
  ░░█▓█──────────────────────█▓█░░
  ░░█▓▓█────────────────────█▓▓█░░
  ░░█▓▓▓█──────────────────█▓▓▓█░░
  ░░█▓▓▓█──────────────────█▓▓▓█░░
  ░░█▓▓▓▓█▄──────────────▄█▓▓▓▓█░░
  ░░░█▓▓█▀█──▄▀▀▀▀▀▀▀▀▄──█▀█▓▓█░░░
  ░░░░█▓█─▀▄▄▀────────▀▄▄▀─█▓█░░░░
  ░░░░░█▓█─────▄▄▄▄▄▄─────█▓█░░░░░
  ░░░░░░█▓█▄▄▄██▓▓▓▓██▄▄▄█▓█░░░░░░
  ░░░░░░░█▓▓▓█▓▓▓▓▓▓▓▓█▓▓▓█░░░░░░░
  ░░░░░░░░████████████████░░░░░░░░
  ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
*/

#define USE_ARDUINO_INTERRUPTS true    // Set-up low-level interrupts for most acurate BPM math.

//#include <Keypad.h>
#include <Keypad_I2C.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Adafruit_Fingerprint.h>
#include <EEPROM.h>

#define I2C_ADDR 0x27                   //I2C adress, you should use the code to scan the adress first (0x27) here
#define I2CADDR 0x20                    // Set the Address of the PCF8574

char keys [4] [4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

char keypressed;                 //Where the keys are stored it changes very often

char code[] = {'1', '2', '3', '4', '5', '6'}; //The default code, you can change it or make it a 'n' digits one

char code_buff1[sizeof(code)];  //Where the new key is stored

short a = 0, i = 0, j = 0, s = 0;  //Variables used later
int Signal;
int count;

// define active Pin (4x4)
byte rowPins [4] = {0, 1, 2, 3}; // Connect to Keyboard Row Pin
byte colPins [4] = {4, 5, 6, 7}; // Connect to Pin column of keypad.

LiquidCrystal_I2C lcd(0x27, 20, 4);
Keypad_I2C keypad (makeKeymap (keys), rowPins, colPins, 4, 4, I2CADDR, PCF8574);
Servo servo;

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
SoftwareSerial mySerial(2, 3);

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial1

#endif


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;

void setup()
{
  Serial.begin (9600);
  //Serial.println(sizeof(code));
  Wire .begin (); // Call the connection Wire
  keypad.begin (makeKeymap (keys)); // Call the connection
  servo.attach(8);
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println(F("Found fingerprint sensor!"));
  } else {
    Serial.println(F("Did not find fingerprint sensor :("));
  }

  //FIRST UPLOAD UNCOMMENTED, THEN COMMENTED UPLOAD
  //for (i = 0 ; i < sizeof(code) ; i++) {
  //  EEPROM.write(i, code[i]);
  //}

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(F("*                  *"));
  lcd.setCursor(0, 1);
  lcd.print(F("*     Booting      *"));      //What's written on the LCD you can change
  lcd.setCursor(0, 2);
  lcd.print(F("*   IronLock ...   *"));
  lcd.setCursor(0, 3);
  lcd.print(F("*                  *"));
  Lock();
  finger.getParameters();
  finger.getTemplateCount();
  delay(2000);
  UserInterface();
}


void loop()
{
  keypressed = keypad.getKey();               //Constantly waiting for a key to be pressed
  if (keypressed == '2') {                    // * to open the lock
    //Serial.println(code);
    //Serial.println(code_buff1);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Enter Pincode"));            //Message to show
    GetCode();                          //Getting code function
    if (a == sizeof(code)) {       //The GetCode function assign a value to a (it's correct when it has the size of the code array)
      lcd.clear();                   //Open lock function if code is correct
      lcd.setCursor(0, 1);
      lcd.print(F("     Correct Pin    "));
      delay(2000);
      isHuman();
      Lock();
    } else {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("     Wrong Pin!     "));          //Message to print when the code is wrong
      count++;
      EEPROM.write(8, count);
      Lock();
      delay(2000);
      UserInterface();
    }
  } else if (keypressed == '1') {
    //Serial.println(finger.templateCount);
    if (finger.templateCount == 0) {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("     No fingers     "));
      lcd.setCursor(0, 2);
      lcd.print(F("     registered!    "));
      delay(5000);
      UserInterface();
    } else {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("  Place Finger on   "));
      lcd.setCursor(0, 2);
      lcd.print(F("      Reader! "));
      delay(5000);
      uint32_t startTime = millis();
      bool running = true;
      while (running) {
        uint8_t f = getFingerprintID();
        if (f == FINGERPRINT_OK) {
          running = false;
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print(F("     Unlocked!"));
          count = 0;
          EEPROM.write(8, count);
          Unlock();
          delay(15000);
          Lock();
          UserInterface();
        } else if ((millis() - startTime >= 7000)) {
          if (f == FINGERPRINT_NOTFOUND) {
            lcd.clear();
            lcd.setCursor(0, 1);
            lcd.print(F("Finger not a match!"));
            count++;
            EEPROM.write(8, count);
            Lock();
            delay(5000);
            UserInterface();
            running = false;
          } else {
            lcd.clear();
            lcd.setCursor(0, 1);
            lcd.print(F("No Finger detected!"));
            count++;
            EEPROM.write(8, count);
            Lock();
            delay(5000);
            UserInterface();
            running = false;
          }
        }
      }
    }
  } else if (keypressed == '3') {
    SettingsInterface1();
  } else if (keypressed == '4') {
    SettingsInterface3();
  } else if (keypressed == '5') {
    SettingsInterface2();
  } else if (keypressed == '6') {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Enter Pincode"));            //Message to show
    GetCode();                          //Getting code function
    if (a == sizeof(code)) {       //The GetCode function assign a value to a (it's correct when it has the size of the code array)
      lcd.clear();                   //Open lock function if code is correct
      lcd.setCursor(0, 1);
      lcd.print(F("    Correct Pin     "));
      count = 0;
      EEPROM.write(8, count);
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Enter Print ID:  "));
      id = 0;
      GetCode();
      id = a;
      while (!  getFingerprintEnroll() );
    } else {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("     Wrong Pin!     "));          //Message to print when the code is wrong
      count++;
      EEPROM.write(8, count);
      Lock();
      delay(2000);
      SettingsInterface2();
    }
  } else if (keypressed == '7') {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Enter Pincode: "));            //Message to show
    GetCode();                          //Getting code function
    if (a == sizeof(code)) {       //The GetCode function assign a value to a (it's correct when it has the size of the code array)
      lcd.clear();                   //Open lock function if code is correct
      lcd.setCursor(0, 1);
      lcd.print(F("    Correct Pin     "));
      count = 0;
      EEPROM.write(8, count);
      delay(2000);
      finger.emptyDatabase();
      finger.templateCount = 0;
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("    Fingerprints    "));
      lcd.setCursor(0, 2);
      lcd.print(F("       Deleted!     "));
      delay(2000);
      SettingsInterface2();
    } else {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("     Wrong Pin!     "));          //Message to print when the code is wrong
      count++;
      EEPROM.write(8, count);
      Lock();
      delay(2000);
      SettingsInterface2();
    }
  } else if (keypressed == '8') {
    ChangeCode();
    delay(2000);
    SettingsInterface3();
  } else if (keypressed == '9') {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Enter Pincode"));            //Message to show
    GetCode();                          //Getting code function
    if (a == sizeof(code)) {       //The GetCode function assign a value to a (it's correct when it has the size of the code array)
      count = 0;
      EEPROM.write(8, count);
      char defcode[] = {'0', '0', '0', '0', '0', '0'};
      for ( int i = 0; i < sizeof(code);  ++i ) {
        code[i] = (char)0;
      }
      for (i = 0 ; i < sizeof(code) ; i++) {
        code[i] = defcode[i];       //the code array now receives the new code
        EEPROM.write(i, code[i]);
      }
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("     Pin reset!"));
      delay(2000);
      SettingsInterface3();
    } else {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("     Wrong Pin!     "));          //Message to print when the code is wrong
      count++;
      EEPROM.write(8, count);
      Lock();
      delay(2000);
      SettingsInterface3();
    }
  } else if (keypressed == '*') {
    lcd.clear();
    UserInterface();
  } else if (keypressed == '#') {
    lcd.clear();
    SettingsInterface1();
  } else if (keypressed == 'D') {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   IronLock v1.00   ");
    lcd.setCursor(0, 1);
    lcd.print("Created by the Team ");
    lcd.setCursor(0, 2);
    lcd.print("    RJH-Security    ");
    lcd.setCursor(0, 3);
    lcd.print("at Stark Industries ");
    delay(8000);
    UserInterface();
  }
}

void ChangeCode() {                     //Change code sequence
  lcd.clear();
  lcd.print(F("Enter old Pincode: "));
  GetCode();                      //verify the old code first so you can change it

  if (a == sizeof(code)) {  //again verifying the a value
    lcd.clear();                   //Open lock function if code is correct
    lcd.setCursor(0, 1);
    lcd.print(F("    Correct Pin     "));
    delay(2000);
    count = 0;
    EEPROM.write(8, count);
    GetNewCode1();            //Get the new code

    for ( int i = 0; i < sizeof(code);  ++i ) {
      code[i] = (char)0;
    }
    for (i = 0 ; i < sizeof(code) ; i++) {
      code[i] = code_buff1[i];       //the code array now receives the new code
      EEPROM.write(i, code[i]);
    }
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("  Pincode Changed"));
    //Serial.println(code);
    delay(2000);
  }
  else {                    //In case the old code is wrong you can't change it
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("     Wrong Pin"));
    count++;
    EEPROM.write(8, count);
    Lock();
    delay(2000);
  }
}

void GetNewCode1() {
  lcd.clear();
  lcd.print(F("New 6 - digit Pin: "));
  i = 0;
  j = 0;

  while (keypressed != 'A') {          //A to confirm and quits the loop
    //Serial.println(j);
    keypressed = keypad.getKey();
    if (keypressed != NO_KEY) {
      lcd.setCursor(j, 1);
      lcd.print("*");               //On the new code you can show * as I did or change it to keypressed to show the keys
      code_buff1[i] = keypressed;   //Store caracters in the array
      i++;
      j++;
    }
  }

  if (j != 7) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("It must be 6 Digits!"));
    //Serial.println(j);
    delay(2000);
    for ( int i = 0; i < sizeof(code_buff1);  ++i ) {
      code_buff1[i] = (char)0;
    }
    keypressed = NO_KEY;
    GetNewCode1();
  }

  //Serial.println("code_buff1 :");
  //Serial.println(code_buff1);
  keypressed = NO_KEY;
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image taken");
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("     reading..."));
      break;
    case FINGERPRINT_NOFINGER:
      //Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      //Serial.println("Imaging error");
      return p;
    default:
      Serial.println(F("Unknown error"));
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("     reading..."));
      break;
    case FINGERPRINT_IMAGEMESS:
      //Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      //Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      //Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println(F("Unknown error"));
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    //Serial.println("Found a print match!");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("Found a print match!"));
    return p;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    //Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    //Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println(F("Unknown error"));
    return p;
  }
}

void GetCode() {                 //Getting code sequence
  i = 0;                    //All variables set to 0
  a = 0;
  j = 0;
  for (int i = 0; i < 6; i++) {
    code[i] = EEPROM.read(i);
  }
  Serial.println(F("Code is: ")); Serial.println(code);
  //Serial.println(F("Code Size is: ")); Serial.println(sizeof(code));
  while (keypressed != 'A') {                                   //The user press A to confirm the code otherwise he can keep typing
    keypressed = keypad.getKey();
    if (keypressed != NO_KEY && keypressed != 'A' ) {     //If the char typed isn't A and neither "nothing"
      lcd.setCursor(j, 1);                                 //This to print "*" on the LCD whenever a key is pressed it's position is controlled by j
      lcd.print("*");
      j++;
      if (keypressed == code[i] && i < sizeof(code)) {       //if the char typed is correct a and i increments to verify the next caracter
        a++;                                              //Now I think maybe I should have use only a or i ... too lazy to test it -_-'
        i++;
      }
      else
        a--;                                               //if the character typed is wrong a decrements and cannot equal the size of code []
    }
  }
  keypressed = NO_KEY;
}

void UserInterface() {
  count = EEPROM.read(8);
  //Serial.println("Count:");
  //Serial.println(count);
  if (count >= 3) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("   TOO MANY TRIES   "));      //What's written on the LCD you can change
    lcd.setCursor(0, 2);
    lcd.print(F("   wait 1 minute   "));      //What's written on the LCD you can change
    count = 0;
    EEPROM.write(8, count);
    delay(60000);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("    Unlock with:    "));      //What's written on the LCD you can change
  lcd.setCursor(0, 1);
  lcd.print(F("   1 Fingerprint    "));
  lcd.setCursor(0, 2);
  lcd.print(F("     2 Pincode      "));
  lcd.setCursor(0, 3);
  lcd.print(F(" OR  3 Settings     "));
  lcd.setCursor(19, 3);
  lcd.print(count);
}

void SettingsInterface1() {
  count = EEPROM.read(8);
  if (count >= 3) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("   TOO MANY TRIES   "));      //What's written on the LCD you can change
    lcd.setCursor(0, 2);
    lcd.print(F("   wait 1 minute   "));      //What's written on the LCD you can change
    count = 0;
    EEPROM.write(8, count);
    delay(60000);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("   Choose Setting:  "));
  lcd.setCursor(0, 1);
  lcd.print(F("       * Back  "));
  lcd.setCursor(0, 2);
  lcd.print(F("     4 Pincode      "));
  lcd.setCursor(0, 3);
  lcd.print(F("   5 Fingerprints   "));
}

void SettingsInterface2() {
  count = EEPROM.read(8);
  if (count >= 3) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("   TOO MANY TRIES   "));      //What's written on the LCD you can change
    lcd.setCursor(0, 2);
    lcd.print(F("   wait 1 minute   "));      //What's written on the LCD you can change
    count = 0;
    EEPROM.write(8, count);
    delay(60000);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("   Choose Setting:  "));
  lcd.setCursor(0, 1);
  lcd.print(F("       # Back  "));
  lcd.setCursor(0, 2);
  lcd.print(F("  6 Add Fingerprint  "));
  lcd.setCursor(0, 3);
  lcd.print(F("    7 Delete All     "));
}

void SettingsInterface3() {
  count = EEPROM.read(8);
  if (count >= 3) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("   TOO MANY TRIES   "));      //What's written on the LCD you can change
    lcd.setCursor(0, 2);
    lcd.print(F("   wait 1 minute   "));      //What's written on the LCD you can change
    count = 0;
    EEPROM.write(8, count);
    delay(60000);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("   Choose Setting:   "));
  lcd.setCursor(0, 1);
  lcd.print(F("       # Back  "));
  lcd.setCursor(0, 2);
  lcd.print(F("    8 Change Pin     "));
  lcd.setCursor(0, 3);
  lcd.print(F("     9 Reset Pin     "));
}

void Unlock() {
  servo.write(90); //Position 1 ansteuern mit dem Winkel 90°
}

void Lock() {
  servo.write(0);
}

void isHuman() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("  Place Finger on   "));
  lcd.setCursor(0, 2);
  lcd.print(F("  Heartbeat Sensor! "));
  delay(8000);
  uint32_t startTime = millis();
  bool running = true;
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("     Verifiying     "));
  while (running) {
    Signal = analogRead(0);
    //Serial.println(Signal);                    // Send the Signal value to Serial Plotter.
    if (Signal > 570 && Signal < 750) {                        // If the signal is above "550", then "turn - on" Arduino's on-Board LED.
      //Serial.println(Signal);
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("   HUMAN VERIFIED   "));
      lcd.setCursor(0, 2);
      lcd.print(F("     Unlocked!      "));
      count = 0;
      EEPROM.write(8, count);
      Unlock();
      delay(15000);
      UserInterface();
      running = false;
    } else if ((millis() - startTime >= 2000)) {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("    NOT A HUMAN!    "));
      count++;
      EEPROM.write(8, count);
      Lock();
      delay(5000);
      UserInterface();
      running = false;
    }
  }
}

uint8_t getFingerprintEnroll() {
  int p = -1;
  //Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("    Place finger"));
  lcd.setCursor(0, 2);
  lcd.print(F("     on sensor!"));
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        //Serial.println("Image taken");
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(F("    Image taken"));
        break;
      case FINGERPRINT_NOFINGER:
        //Serial.println(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println(F("Communication error"));
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(F("Error! Try again"));
        delay(2000);
        SettingsInterface2();
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println(F("Imaging error"));
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(F("Error! Try again"));
        delay(2000);
        SettingsInterface2();
        break;
      default:
        Serial.println(F("Unknown error"));
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(F("Error! Try again"));
        delay(2000);
        SettingsInterface2();
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("  Image converted"));
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println(F("Image too messy"));
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("Error! Try again"));
      delay(2000);
      SettingsInterface2();
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println(F("Communication error"));
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("Error! Try again"));
      delay(2000);
      SettingsInterface2();
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println(F("Could not find fingerprint features"));
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("Error! Try again"));
      delay(2000);
      SettingsInterface2();
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println(F("Could not find fingerprint features"));
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("Error! Try again"));
      delay(2000);
      SettingsInterface2();
      return p;
    default:
      Serial.println(F("Unknown error"));
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("Error! Try again"));
      delay(2000);
      SettingsInterface2();
      return p;
  }

  //Serial.println("Remove finger");
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("   Remove Finger"));
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  //Serial.print("ID "); Serial.println(id);
  p = -1;
  //Serial.println("Place same finger again");
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F(" Place same finger"));
  lcd.setCursor(0, 2);
  lcd.print(F("       again"));
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        //Serial.println("Image taken");
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(F("    Image taken"));
        break;
      case FINGERPRINT_NOFINGER:
        //Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println(F("Communication error"));
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(F("Error! Try again"));
        delay(2000);
        SettingsInterface2();
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println(F("Imaging error"));
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(F("Error! Try again"));
        delay(2000);
        SettingsInterface2();
        break;
      default:
        Serial.println(F("Unknown error"));
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(F("Error! Try again"));
        delay(2000);
        SettingsInterface2();
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("  Image converted"));
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println(F("Image too messy"));
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("Error! Try again"));
      delay(2000);
      SettingsInterface2();
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println(F("Communication error"));
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("Error! Try again"));
      delay(2000);
      SettingsInterface2();
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println(F("Could not find fingerprint features"));
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("Error! Try again"));
      delay(2000);
      SettingsInterface2();
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println(F("Could not find fingerprint features"));
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("Error! Try again"));
      delay(2000);
      SettingsInterface2();
      return p;
    default:
      Serial.println(F("Unknown error"));
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(F("Error! Try again"));
      delay(2000);
      SettingsInterface2();
      return p;
  }

  // OK converted!
  //Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    //Serial.println("Prints matched!");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("  Prints matched!"));
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println(F("Communication error"));
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("Error! Try again"));
    delay(2000);
    SettingsInterface2();
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println(F("Fingerprints did not match"));
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("  Fingerprints did"  ));
    lcd.setCursor(0, 2);
    lcd.print(F("     not match"      ));
    delay(2000);
    SettingsInterface2();
    return p;
  } else {
    Serial.println(F("Unknown error"));
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("Error! Try again"));
    delay(2000);
    SettingsInterface2();
    return p;
  }

  //Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    //Serial.println("Stored!");
    finger.templateCount++;
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("      Stored!"));
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println(F("Communication error"));
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("Error! Try again"));
    delay(2000);
    SettingsInterface2();
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println(F("Could not store in that location"));
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("Error! Try again"));
    delay(2000);
    SettingsInterface2();
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println(F("Error writing to flash"));
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("Error! Try again"));
    delay(2000);
    SettingsInterface2();
    return p;
  } else {
    //Serial.println(F("Unknown error"));
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("Error! Try again"));
    delay(2000);
    SettingsInterface2();
    return p;
  }
  delay(2000);
  UserInterface();
  return true;
}
