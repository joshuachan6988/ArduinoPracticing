#include <dht.h>   
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino_FreeRTOS.h>

#define DS3231_I2C_ADDRESS 0x68
#define LCD1602_I2C_ADDRESS 0x27
#define LCD_EN 2
#define LCD_RW 1
#define LCD_RS 0
#define LCD_D4 4
#define LCD_D5 5
#define LCD_D6 6
#define LCD_D7 7
#define LCD_BL 3
#define LCD_BLPOL POSITIVE

LiquidCrystal_I2C lcd(LCD1602_I2C_ADDRESS, LCD_EN, LCD_RW, LCD_RS, LCD_D4,  LCD_D5, LCD_D6, LCD_D7, LCD_BL, LCD_BLPOL);

/* DHT data input read from Pin 2*/
#define dht_dpin 2
dht DHT;

/* Relay switch on by Pin 8 */
#define RELAY 8

/*---------- Define functions for DS3231 -----------------*/
byte decToBcd(byte val);
byte bcdToDec(byte val);
void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year);
void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year);
void displayTime(void);

/*---------- Define functions for DHT11 -----------------*/
void displayTemperHumid(void);

/*---------- Define functions for  Relay ---------------*/
void switchonBump(void);

/*---------- define Tasks -----------------*/
void TaskAlarm( void *pvParameters );
void TaskDisplay( void *pvParameters );




void setup() 
{
  /* Optional Choice: Set the initial time
   * Example: DS3231 seconds, minutes, hours, day, date, month, year
   * setDS3231time(30,42,21,4,26,11,14);
  */
  Serial.begin(9600);
  // Initialize LCD
  lcd.begin(16, 2);
  //lcd.backlight();
  lcd.noBacklight();

  // Set up GPIO for Relay
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);

  // Now set up two tasks to run independently.
  xTaskCreate(
    TaskAlarm
    ,  (const portCHAR *) "Alarm"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TaskDisplay
    ,  (const portCHAR *) "Display"
    ,  512  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL );
    
}



void loop()
{
}

/*------------------------------------------------------*/
/*---------------------- Functions ---------------------*/
/*------------------------------------------------------*/

/*----------------- Fuctions for DS3231 ----------------*/
/* Convert normal decimal numbers to binary coded decimal */
byte decToBcd(byte val)
{
  return( (val/10*16) + (val%10) );
}

/* Convert binary coded decimal to normal decimal numbers */
byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}

/* Set time to DS3231 */
void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
{
  // Sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  // Set next input to start at the seconds register
  Wire.write(0); 
  // Set seconds
  Wire.write(decToBcd(second)); 
  // Set minutes
  Wire.write(decToBcd(minute));
  // Set hours 
  Wire.write(decToBcd(hour)); 
  // Set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfWeek)); 
  // Set date (1 to 31)
  Wire.write(decToBcd(dayOfMonth));
  // Set month 
  Wire.write(decToBcd(month));
  // Set year (0 to 99) 
  Wire.write(decToBcd(year)); 
  Wire.endTransmission();
}

/* Read time from DS3231 */
void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year)
{                      
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  // Set DS3231 register pointer to 00h
  Wire.write(0); 
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // Request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

/* Dispaly Time */
void displayTime(void)
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // Retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  // Set LCD cursor to the left up side
  lcd.setCursor(0, 0);
  // Send it to the serial monitor
  lcd.print(hour, DEC);
  // Convert the byte variable to a decimal number when displayed
  lcd.print(":");
  if (minute<10)
  {
    lcd.print("0");
  }
  lcd.print(minute, DEC);
  lcd.print(":");
  if (second<10)
  {
    lcd.print("0");
  }
  lcd.print(second, DEC);
  lcd.print(" ");
  lcd.print(dayOfMonth, DEC);
  lcd.print("/");
  lcd.print(month, DEC);
  lcd.print("/");
  lcd.print(year, DEC);
  /* Optional Choice: Show the Day of Week
   
  // Set LCD cursor to the left up side
  lcd.setCursor(0, 1);
  // Print daty of week
  lcd.print(" Day of week: ");
  switch(dayOfWeek){
  case 1:
    lcd.println("Sunday");
    break;
  case 2:
    lcd.println("Monday");
    break;
  case 3:
    lcd.println("Tuesday");
    break;
  case 4:
    lcd.println("Wednesday");
    break;
  case 5:
    lcd.println("Thursday");
    break;
  case 6:
    lcd.println("Friday");
    break;
  case 7:
    lcd.println("Saturday");
    break;
  }
  */
}

/*----------------- Fuctions for DHT11 ------------------*/
/* Dispaly Temperature & Humidity */
void displayTemperHumid(void)
{
  /* Read DHT11 infromation */
  DHT.read11(dht_dpin);
  
  int Temperature = (int) DHT.temperature;
  int Humidity = (int) DHT.humidity;
  
  // Set LCD cursor to the left up side
  lcd.setCursor(0, 1);
  // Print Temperature
  lcd.print("T = ");
  lcd.print(Temperature);
  lcd.print("C  ");
  // Print Humidity
  lcd.print("H = ");
  lcd.print(Humidity);
  lcd.print("%"); 
}

/*----------------- Fuctions for Relay ------------------*/
/* Switch on  Bump */
void switchonBump(void)
{
  digitalWrite(RELAY, HIGH); 
  vTaskDelay( 1000 / portTICK_PERIOD_MS ); 
  digitalWrite(RELAY, LOW);
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

/* Alarm Task */
void TaskAlarm(void *pvParameters)
{
  (void) pvParameters;
  
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  
  for (;;) // A Task shall never return or exit.
  {
      // Retrieve data from DS3231
      readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
      if(hour == 13 && minute == 31 )
      {
        switchonBump();
      }
      else
      {
        vTaskDelay( 1000 / portTICK_PERIOD_MS );
      }
  }
}

/* Display Information Task */
void TaskDisplay(void *pvParameters)
{
  (void) pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    // Dispaly Time
    displayTime();
    // Dispaly Temperature & Humidity
    displayTemperHumid();
    // Wait for one second
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); 
    lcd.clear();
  }
}


