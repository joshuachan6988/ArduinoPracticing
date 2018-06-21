#include <Arduino_FreeRTOS.h>

// RFID Read
/*
* Read a card using a mfrc522 reader on your SPI interface
* Pin layout should be as follows (on Arduino Uno):
* MOSI: Pin 11 / ICSP-4
* MISO: Pin 12 / ICSP-1
* SCK: Pin 13 / ISCP-3
* SS: Pin 10
* RST: Pin 9
*/

#include <SPI.h>
#include <RFID.h>

#define SS_PIN 10
#define RST_PIN 9

RFID rfid(SS_PIN,RST_PIN);

int serNum[5];

// LCD Print
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 

// define two tasks for LCDAd & RFIDRead
void TaskLCDAd( void *pvParameters );
void TaskRFIDRead( void *pvParameters );

// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // initialize SPI & RFID
  SPI.begin();
  rfid.init();
  // initialize LCD
  lcd.begin(16, 2);
  for(int i = 0; i < 3; i++) 
  {
    lcd.backlight(); // Open Backlight
    delay(250);
    lcd.noBacklight(); // Close Backlight
    delay(250);
  }
  lcd.backlight();

  // System Bigin
  lcd.setCursor(0, 0);
  lcd.print("System Bigin!!");
  //lcd.print("Welcome to IAC!!");
  delay(1000);
  lcd.setCursor(0, 1); // 設定游標位置在第二行行首
  lcd.print("Start Print");
  delay(2000);
  lcd.clear();
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  // Now set up two tasks to run independently.
  xTaskCreate(
    TaskLCDAd
    ,  (const portCHAR *)"LCDAd"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TaskRFIDRead
    ,  (const portCHAR *) "RFIDRead"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskLCDAd(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  
  for (;;) // A Task shall never return or exit.
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Welcome to IAC!!");
    vTaskDelay( 1000 / portTICK_PERIOD_MS );
    lcd.setCursor(0, 1);
    lcd.print("Have a nice day!");
    vTaskDelay( 1000 / portTICK_PERIOD_MS );
  }
}

void TaskRFIDRead(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  int count;
  int pass;
  int checkNum[5] = {243, 220, 113, 154, 196};
  
  for(;;)
  {
    pass = 0;
    if(rfid.isCard())
    {
      if(rfid.readCardSerial())
      {
        for(count = 0; count < 5; count ++)
        {
          Serial.print(rfid.serNum[count],DEC);
          Serial.print(" ");
          if(rfid.serNum[count] == checkNum[count])
          {
            pass = pass + 1;
          }
        }
        Serial.println("");

        /* If I use lcd print here,
         * it might be a problem that though the messages are printed,
         * they covered each other soon, never being completed.  
         * Therefore, I need Interrupt.
         */
        if(pass == 5)
        {
          Serial.println("Welcome Joshua");
        }
      } 
    }
    rfid.halt();
    //vTaskDelay( 1000 / portTICK_PERIOD_MS );
  }
}
