#include <Servo.h>

int Ldetect = 2;  // 왼쪽 적외선 감지 센서(D2)
int Rdetect = 3;  // 오른쪽 적외선 감지 센서(D3)

int in1 = 9;
int in2 = 10;  // 모터 연결 핀(D9, D10)

int servo = 6;  // 서보 모터 PWM 연결 핀(D6)
int Speed = 255;  // 모터 속도 셋팅

Servo dir;

void setup() {
  Serial.begin(9600); 
  
  dir.attach(servo);  // 서보모터 연결 설정
  dir.write(92);  // 바퀴를 가운데로 설정
}

void loop() {
// 적외선 감지 센서 값을 각각 L과 R에 저장
  int L = digitalRead(Ldetect);
  int R = digitalRead(Rdetect);

  Serial.print("L:"+String(L));
  Serial.println(",R:"+String(R));
  

// 적외선 감지 센서 각각의 값에 따라 RC카의 방향 설정
  if (R == LOW && L == HIGH) {  //RIGHT
    dir.write(132);
    analogWrite(in1, 0);
    analogWrite(in2, Speed);
  }

  if (R == HIGH && L == LOW) { //LEFT
    dir.write(52);
    analogWrite(in1, 0);
    analogWrite(in2, Speed);
  }

  if (R == HIGH && L == HIGH) {
    dir.write(92);
    analogWrite(in1, 0);
    analogWrite(in2, Speed);
  }

  if (R == LOW && L == LOW) {
    dir.write(92);
    analogWrite(in1, 0);
    analogWrite(in2, 0);
  }
}
