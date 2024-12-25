#include <SD.h>
#include <LiquidCrystal.h>
#include <TM1637Display.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

//LCD:
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

//SD card cs
const int chipSelect = 10;

//7seg display
#define CLK 2
#define DIO 3

TM1637Display segment(CLK, DIO);


byte thermometer[] = {  // thermometer icon for lcd display
  B00100,
  B01010,
  B01010,
  B01010,
  B01010,
  B10001,
  B10001,
  B01110
};



File myFile;

String Line = "";
String lcdMessage = "";

bool running = 0;

int csvTime = 0;
int csvTemp = 0;
int csvDuration = 0;
int realTemp = 20;
int targetTemp = 0;

tmElements_t tm;

int buttonADC;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  lcd.begin(16, 2);

  segment.showNumberDec(-999);

  lcd.setCursor(0, 0);
  lcd.print("  mnux");
  lcd.setCursor(0, 1);
  lcd.print("  2024");


  delay(500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" reflow");
  lcd.setCursor(0, 1);
  lcd.print("  oven");
  delay(500);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SELFTEST");
  lcd.setCursor(0, 1);



  //TESTING RTC
  lcd.setCursor(0, 1);
  lcd.print("        ");
  lcd.setCursor(0, 1);
  lcd.print("RTC:");

  if (RTC.read(tm)) {
    Serial.println("BATT OK");
    lcd.setCursor(5, 1);
    lcd.print("OK");
  } else {
    if (RTC.chipPresent()) {
      lcd.setCursor(5, 1);
      lcd.print("BAD");
    } else {
      lcd.setCursor(5, 1);
      lcd.print("BAD");
    }
  }

  delay(500);

  // wait for Serial Monitor to connect. Needed for native USB port boards only:




  lcd.setCursor(0, 1);
  lcd.print("        ");
  lcd.setCursor(0, 1);
  lcd.print("SD:");

  if (!SD.begin(chipSelect)) {
    lcd.setCursor(4, 1);
    lcd.print("BAD");
    while (true);
  } else {
    lcd.setCursor(4, 1);
    lcd.print("OK");
  }

  Serial.println("initialization done.");

  lcd.clear();

  myFile = SD.open("file.csv");

  display();

  lcd.setCursor(0, 1);
  lcd.print("Ready!");
}

void loop() {

  buttonADC = analogRead(0);

  if (buttonADC < 800) {
    Serial.println("START BUTTON PRESSED ");
    running = 1;
    lcd.setCursor(0, 1);
    lcd.print("Started");
  }

  if (running == 1) {
    getNewLine();
    //Serial.println(Line);
    if (csvTime == csvDuration) {
      lcd.setCursor(0, 1);
      lcd.print("Done!   ");
      Serial.print("done!");
      getNewLine();
      Line = "";
      running == 1;
    }
  }

  delay(1000);
}

char getNewLine() {
  Line = "";

  char currentCharRead = "";

  //Serial.println(currentCharRead);
  //Serial.println("getting new characters:");
  while (1) {
    currentCharRead = myFile.read();

    Line = Line + currentCharRead;

    if (Line == "END;") {  //starts reflow cycle after "START;" appears in input file
      Serial.println("Finished!!");

      break;
    }
    if (Line == "START;") {  //starts reflow cycle after "START;" appears in input file
      Serial.println("Starting reflow cycle!!");

      break;
    }
    if (Line.endsWith("LCD;")) {  //sets LCD message after "LCD;" appears in input file
      Serial.println("LCD: " + Line.substring(0, 8));
      lcd.setCursor(0, 1);
      lcd.print(Line.substring(0, 8));

      break;
    }
    if (Line.endsWith("DURATION;")) {  //sets duration of the reflow cycle after "DURATION;" appears in input file
      csvDuration = Line.substring(0, 4).toInt();
      Serial.print("Duration:");
      Serial.println(csvDuration);

      break;
    }

    else {  //if line isnt a special command it gets csv values


      if (currentCharRead == ',') {  //get 1st csv value
        //Serial.println("comma detected");
        csvTime = Line.toInt();
        Serial.print("NEW TIME:");
        Serial.println(csvTime);
        Line = "";

        while (1) {
          currentCharRead = myFile.read();
          Line = Line + currentCharRead;

          if (currentCharRead == '\n') {  //get 2nd csv value TEMPATURE
            csvTemp = Line.toInt();
            lcd.setCursor(5, 0);
            lcd.print("   ");
            lcd.setCursor(5, 0);
            lcd.print(csvTemp);
            Serial.print("NEW TEMP:");
            Serial.println(csvTemp);
            Line = "";
            break;
          }
        }
      }
    }

    if (currentCharRead == '\n') {  //stop when find new line
      break;
    }

    else {
    }
  }
}


void display() {
  //Serial.print("rinngif display");
  lcd.createChar(0, thermometer);
  lcd.home();
  lcd.write(byte(0));
  lcd.setCursor(2, 0);
  lcd.print(realTemp);
  lcd.setCursor(4, 0);
  lcd.print("/");
  lcd.setCursor(5, 0);
  lcd.print(targetTemp);
}
