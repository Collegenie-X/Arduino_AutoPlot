#include <SoftwareSerial.h>


//  AT
//  AT+NAME[이름]
//  AT+PIN[비밀번호]
//  AT+BAUD[보드레이트 값]

#define BT_RXD 3
#define BT_TXD 4
SoftwareSerial bluetooth(BT_RXD, BT_TXD);

void setup(){
  Serial.begin(9600);
  bluetooth.begin(9600);
}

void loop(){
  if (bluetooth.available()) {
    Serial.write(bluetooth.read());
  }
  if (Serial.available()) {
    bluetooth.write(Serial.read());
  }
}
