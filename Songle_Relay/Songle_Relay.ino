#define RELAY 8
void setup() {
Serial.begin(9600);
pinMode(RELAY, OUTPUT);
digitalWrite(RELAY,LOW); 
}

void loop() {
digitalWrite(RELAY, HIGH); 
Serial.println("Relay ON");
delay(10000);
digitalWrite(RELAY, LOW);
Serial.println("Relay OFF"); 
delay(1000);
}
