/*
  SD card read/write

  This example shows how to read and write data to and from an SD card file
  The circuit. Pin numbers reflect the default
  SPI pins for Uno and Nano models:
   SD card attached to SPI bus as follows:
 ** SDO - pin 11
 ** SDI - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (For For Uno, Nano: pin 10. For MKR Zero SD: SDCARD_SS_PIN)

  created   Nov 2010
  by David A. Mellis
  modified  24 July 2020
  by Tom Igoe

  This example code is in the public domain.

*/
#include <SD.h>
#include <LiquidCrystal.h>

// inicializace LCD displeje
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
// nastavení čísla propojovacího pinu
// pro osvětlení LCD displeje
const int chipSelect = 10;

byte thermometer[] = { // thermometer
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


int buttonADC;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  lcd.begin(16, 2);

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

  // wait for Serial Monitor to connect. Needed for native USB port boards only:
  while (!Serial);

  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR:");
    lcd.setCursor(0, 1);
    lcd.print("no SD");
    while (true);
  }

  Serial.println("initialization done.");

  lcd.clear();

  myFile = SD.open("file.csv");
  
  display();

  lcd.setCursor(0, 1);
  lcd.print("Ready!");
}

void loop() {

  buttonADC = analogRead (0);

  if (buttonADC < 800) {
    Serial.println ("START BUTTON PRESSED ");
    running=1;
    lcd.setCursor(0, 1);
    lcd.print("Started");

  }
  
  if(running==1){
    getNewLine();
    Line = "";
    Serial.print(csvTime);
    Serial.print(",");
    Serial.println(csvTemp);
    
    if(csvTime == csvDuration){
      lcd.setCursor(0, 1);
      lcd.print("Done!   ");
      Serial.print("done!");
      getNewLine();
      Line = "";
      running==0;
    }
  } 
 
  delay(1000);

}

char getNewLine(){
  
  char currentCharRead = "";

  //Serial.println(currentCharRead);
  //Serial.println("getting new characters:");
  while(1){
    currentCharRead = myFile.read();

    Line = Line + currentCharRead;

    if(Line == "START;"){ //starts reflow cycle after "START;" appears in input file
      Serial.println("Starting reflow cycle!!");
      break;
    }

    if(Line.endsWith("LCD;")){ //sets LCD message after "LCD;" appears in input file
      Serial.println("LCD: " + Line.substring(0,8));
      lcd.setCursor(0, 1);
      lcd.print(Line.substring(0,8));
      
    }

    if(Line.endsWith("DURATION;")){ //sets duration of the reflow cycle after "DURATION;" appears in input file
      csvDuration = Line.substring(0,4).toInt();
      Serial.print("Duration:");
      Serial.println(csvDuration);
      break;
    }
    

    if(currentCharRead == ','){//get 1st csv value
      //Serial.println("comma detected");
      csvTime = Line.toInt();
      Line = "";

      

      while(1){
          currentCharRead = myFile.read();
          Line = Line + currentCharRead;
          
          if(currentCharRead == ','){//get 2nd csv value TEMPATURE
            csvTemp = Line.toInt();
            lcd.setCursor(5, 0);
            lcd.print("   ");
            lcd.setCursor(5, 0);
            lcd.print(csvTemp);
            Line = "";
            break;
          }
        }

      
    }

    if(currentCharRead == '\n'){ //stop when find new line
      break;
    }

    else{
      
    }
    
  }


  //return Line;
}


void display(){
  Serial.print("rinngif display");
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
