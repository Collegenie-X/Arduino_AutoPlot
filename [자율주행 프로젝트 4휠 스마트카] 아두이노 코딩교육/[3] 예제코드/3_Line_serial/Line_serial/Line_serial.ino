void setup() {
  Serial.begin(9600);
  pinMode(A3,INPUT);
  pinMode(A4,INPUT);
  pinMode(A5,INPUT);


}
void loop() {
  int val_1 = digitalRead(A3);
  int val_2 = digitalRead(A4);
  int val_3 = digitalRead(A5);
  
  Serial.print(" LEFT:");
  Serial.print(val_3);
  Serial.print("  CENTER:");
  Serial.print(val_2);

  Serial.print(" RIGHT:");
  Serial.println(val_1);

 
  delay(100);
}
