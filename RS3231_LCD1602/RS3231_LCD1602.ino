#include <Wire.h>
#include <LiquidCrystal_I2C.h>

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

LiquidCrystal_I2C lcd(LCD1602_I2C_ADDRESS, LCD_EN, LCD_RW, LCD_RS, LCD_D4
                    ,  LCD_D5, LCD_D6, LCD_D7, LCD_BL, LCD_BLPOL);

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}

void setup() {
  lcd.begin(16, 2);
  // put your setup code here, to run once:
}

void readDS3231time(byte *second,
byte *minute,
byte *hour,
byte *dayOfWeek,
byte *dayOfMonth,
byte *month,
byte *year)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}
void displayTime()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);
  // send it to the serial monitor
  lcd.print(hour, DEC);
  // convert the byte variable to a decimal number when displayed
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
  /*lcd.print(" Day of week: ");
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
  }*/
}
void loop()
{
  
  displayTime(); // display the real-time clock data on the Serial Monitor,
  delay(1000); // every second
  lcd.clear();
}
