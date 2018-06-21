#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <task.h>

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

// define two tasks for Blink & AnalogRead
// Why static functions? 
static void vSenderTask( void *pvParameters );
static void TaskRFIDRead( void *pvParameters );
static void TaskLCDReceiver( void *pvParameters );

struct message{
  char* Topline;
  char* Bottomline;
};

QueueHandle_t xQueue;

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

  message Message[2] = { {"Welcome to IAC!!", "Have a nice day!"}
                        ,{"How are you!", "Joshua Jan"}};
                        
  xQueue = xQueueCreate(2, sizeof(message));
  
  if( xQueue != NULL)
  {
    /* When FreeRTOS xTaskCreate API Function faces unreasonable memory allocating,  
     * it will stop unless there are enough space or assign property size of memory.
     * Try assign 1000 bytes for each task.
     */
    xTaskCreate(TaskLCDReceiver, "Receiver", 128, NULL, 2, NULL);
    xTaskCreate(vSenderTask, "General Message", 128, (void *) &Message[0], 1, NULL);
    xTaskCreate(TaskRFIDRead, "RFID Message", 128, (void *) &Message[1], 1, NULL);
    //vTaskStartScheduler();
    Serial.println("YES");
    
  }
  else
  {
    Serial.println("ERROR: QUEUE CREATING FAIL!!");
  }
}

void loop()
{
  // Empty. Things are done in Tasks.
}

static void vSenderTask( void *pvParameters )
{
  message * MessageToSend;
  portBASE_TYPE xStatus;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  MessageToSend = (message *) pvParameters;

  for( ;; )
  {
    xStatus = xQueueSendToBack(xQueue, &MessageToSend, 0);
    if( xStatus != pdPASS)
    {
      Serial.println("Could not sent to the queue.");
    }
    vTaskDelayUntil( &xLastWakeTime, ( 5000 / portTICK_PERIOD_MS ) );
    taskYIELD();
  }
}

static void TaskRFIDRead(void *pvParameters)  // This is a task.
{
  message * MessageToSend;
  portBASE_TYPE xStatus;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  MessageToSend = (message *) pvParameters;

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
        if(pass == 5)
        {
          xStatus = xQueueSendToBack(xQueue, &MessageToSend, 0);
          vTaskDelayUntil( &xLastWakeTime, ( 2000 / portTICK_PERIOD_MS ) );
          taskYIELD();
          if( xStatus != pdPASS)
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Couldn't send");
            lcd.setCursor(0, 1);
            lcd.print("to the queue!");
          }
        }
      } 
    }
    else
    {
      vTaskDelayUntil( &xLastWakeTime, ( 1000 / portTICK_PERIOD_MS ) );
      rfid.halt();
      taskYIELD();
    }
  }
}

static void TaskLCDReceiver( void *pvParameters )
{
  message * ReceiveMessage;
  portBASE_TYPE xStatus;
  const TickType_t xTicksToWait = portMAX_DELAY;
  //const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS;

  for( ;; )
  {
    if(uxQueueMessagesWaiting(xQueue) != 0)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Queue should");
      lcd.setCursor(0, 1);
      lcd.print("have been empty.");
      vTaskDelay( 500 / portTICK_PERIOD_MS );
    }
    xStatus = xQueueReceive(xQueue, &ReceiveMessage, xTicksToWait);
    if(xStatus == pdPASS)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print((* ReceiveMessage).Topline);
      lcd.setCursor(0, 1);
      lcd.print((* ReceiveMessage).Bottomline);
      vTaskDelay( 500 / portTICK_PERIOD_MS );
    }
    else
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Couldn't receive");
      lcd.setCursor(0, 1);
      lcd.print("from the queue.");
      vTaskDelay( 500 / portTICK_PERIOD_MS );
    }
  }
}
