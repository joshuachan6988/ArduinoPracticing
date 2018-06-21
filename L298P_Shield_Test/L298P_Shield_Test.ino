int E1 = 10; // Speed (PWM)
int M1 = 12; // Direction
int E2 = 11; // Speed (PWM)
int M2 = 13; // Direction

void setup()
{
  pinMode(M1, OUTPUT);
  pinMode(M1, OUTPUT);
}

void loop()
{
  int value;
  digitalWrite(M1,HIGH);
  digitalWrite(M2,HIGH);
  
  for(value = 0 ; value <= 255; value+=5)
  {  
    //digitalWrite(E1, HIGH);
    //digitalWrite(E2, HIGH);
    analogWrite(E1, value);   //PWM調速
    analogWrite(E2, value);   //PWM調速
    delay(300);
  }
}
