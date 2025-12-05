/*
 * RC CAR Simple Version
 * 서보모터, 기어모터, 블루투스, 초음파센서만 동작
 * LCD, 8X8 LED Matrix 제외
 * 
 * 핀 배치 (예제코드 기준):
 * - 서보모터: 2번 핀
 * - 블루투스: RX=3, TX=4
 * - 모터드라이버: 오른쪽(E=5, IN1=8, IN2=9), 왼쪽(IN3=10, IN4=11, E=6)
 * - 초음파센서: TRIG=13, ECHO=12
 * - 피에조: 7번 핀
 */

#include <SoftwareSerial.h>   // 블루투스 통신 라이브러리
#include <Servo.h>            // 서보 모터 라이브러리

/* 초음파 센서 핀 정의 (2_Ultrasonic_sensor 기준) */
#define TRIG 13
#define ECHO 12

/* 모터 드라이버 핀 정의 (6_Motor_driver 기준) */
#define RightMotor_E_pin  5   // 오른쪽 모터 Enable & PWM
#define RightMotor_1_pin  8   // 오른쪽 모터 IN1
#define RightMotor_2_pin  9   // 오른쪽 모터 IN2
#define LeftMotor_3_pin   10  // 왼쪽 모터 IN3
#define LeftMotor_4_pin   11  // 왼쪽 모터 IN4
#define LeftMotor_E_pin   6   // 왼쪽 모터 Enable & PWM

/* 블루투스 핀 정의 (11_Bluetooth 기준) */
#define BT_RXD 3
#define BT_TXD 4

/* 피에조 부저 */
#define PIEZO 7

/* 서보모터 핀 (1_Servo_motor 기준) */
#define SERVO_PIN 2

/* 모터 속도 설정 (0~255) */
int L_MotorSpeed = 153;   // 왼쪽 모터 속도
int R_MotorSpeed = 153;   // 오른쪽 모터 속도

/* 모터 동작 시간 (ms) */
int move_delay = 40;

/* 장애물 회피 모드 설정 */
bool obstacleAvoidMode = true;   // true: 장애물 회피 ON, false: OFF

/* 서보모터 객체 */
Servo myservo; 

/* 블루투스 통신 변수 */
char inBTChar;
String inputBTString;
char inSLChar;
String inputSLString;

/* 블루투스 시리얼 객체 */
SoftwareSerial bluetooth(BT_RXD, BT_TXD);

/*
 * 전진 동작
 */
void SmartCar_Go() {
  Serial.println("Forward");
  
  digitalWrite(RightMotor_1_pin, HIGH);
  digitalWrite(RightMotor_2_pin, LOW);
  digitalWrite(LeftMotor_3_pin, HIGH);
  digitalWrite(LeftMotor_4_pin, LOW);
  
  analogWrite(RightMotor_E_pin, R_MotorSpeed);
  analogWrite(LeftMotor_E_pin, L_MotorSpeed);
  
  delay(move_delay);
}

/*
 * 후진 동작
 */
void SmartCar_Back() {
  Serial.println("Backward");
  
  digitalWrite(RightMotor_1_pin, LOW);
  digitalWrite(RightMotor_2_pin, HIGH);
  digitalWrite(LeftMotor_3_pin, LOW);
  digitalWrite(LeftMotor_4_pin, HIGH);
  
  analogWrite(RightMotor_E_pin, R_MotorSpeed);
  analogWrite(LeftMotor_E_pin, L_MotorSpeed);
  
  delay(move_delay);
}

/*
 * 왼쪽 회전 (오른쪽 모터만 동작)
 */
void SmartCar_Left() {
  Serial.println("Left");
  
  digitalWrite(RightMotor_1_pin, HIGH);
  digitalWrite(RightMotor_2_pin, LOW);
  digitalWrite(LeftMotor_3_pin, LOW);
  digitalWrite(LeftMotor_4_pin, LOW);
  
  analogWrite(RightMotor_E_pin, R_MotorSpeed);
  analogWrite(LeftMotor_E_pin, 0);
  
  delay(move_delay);
}

/*
 * 오른쪽 회전 (왼쪽 모터만 동작)
 */
void SmartCar_Right() {
  Serial.println("Right");
  
  digitalWrite(RightMotor_1_pin, LOW);
  digitalWrite(RightMotor_2_pin, LOW);
  digitalWrite(LeftMotor_3_pin, HIGH);
  digitalWrite(LeftMotor_4_pin, LOW);
  
  analogWrite(RightMotor_E_pin, 0);
  analogWrite(LeftMotor_E_pin, L_MotorSpeed);
  
  delay(move_delay);
}

/*
 * 정지 동작
 */
void SmartCar_Stop() {
  Serial.println("Stop");
  
  digitalWrite(RightMotor_1_pin, LOW);
  digitalWrite(RightMotor_2_pin, LOW);
  digitalWrite(LeftMotor_3_pin, LOW);
  digitalWrite(LeftMotor_4_pin, LOW);
  
  analogWrite(RightMotor_E_pin, 0);
  analogWrite(LeftMotor_E_pin, 0);
  
  tone(PIEZO, 1000, 200);
  
  delay(10);
}

/*
 * 초음파 센서로 거리 측정
 */
long getDistance() {
  long duration, distance;
  
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  duration = pulseIn(ECHO, HIGH);
  distance = ((float)(340 * duration) / 10000) / 2;
  
  Serial.print("distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  return distance;
}

/*
 * 초기 설정
 */
void setup() {
  // 시리얼 통신 초기화
  Serial.begin(9600);
  bluetooth.begin(9600);
  
  // 문자열 초기화
  inputBTString = "";
  inputSLString = "";
  
  // 모터 드라이버 핀 설정
  pinMode(RightMotor_E_pin, OUTPUT);
  pinMode(RightMotor_1_pin, OUTPUT);
  pinMode(RightMotor_2_pin, OUTPUT);
  pinMode(LeftMotor_3_pin, OUTPUT);
  pinMode(LeftMotor_4_pin, OUTPUT);
  pinMode(LeftMotor_E_pin, OUTPUT);
  
  // 초음파 센서 핀 설정
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  
  // 피에조 핀 설정
  pinMode(PIEZO, OUTPUT);
  
  // 시작 알림
  tone(PIEZO, 1000, 100);
  
  // 서보모터 초기 테스트
  myservo.attach(SERVO_PIN);
  myservo.write(90);    // 중앙 위치
  delay(500);
  myservo.write(0);     // 0도
  delay(500);
  myservo.write(180);   // 180도
  delay(500);
  myservo.write(90);    // 다시 중앙
  delay(500);
  myservo.detach();
  
  tone(PIEZO, 1000, 100);
  
  Serial.println("RC CAR Ready!");
}

/*
 * 메인 루프
 */
void loop() {
  // 블루투스 또는 시리얼 통신 처리
  while (bluetooth.available() || Serial.available()) {
    if (bluetooth.available()) {
      inBTChar = (char)bluetooth.read();
    }
    if (Serial.available()) {
      inSLChar = (char)Serial.read();
    }
    
    if (inBTChar == '\n' || inBTChar == '\r' || inBTChar == '_' || inSLChar == '_') {
      
      if (inputBTString.equals("STOP") || inputSLString.equals("STOP")) {
        SmartCar_Stop();
        tone(PIEZO, 1000, 200);
        bluetooth.print("OK");
        inputBTString = "";
        inputSLString = "";
        
      } else if (inputBTString.equals("LEFT") || inputSLString.equals("LEFT")) {
        SmartCar_Left();
        bluetooth.print("OK");
        inputBTString = "";
        inputSLString = "";
        
      } else if (inputBTString.equals("RIGHT") || inputSLString.equals("RIGHT")) {
        SmartCar_Right();
        bluetooth.print("OK");
        inputBTString = "";
        inputSLString = "";
        
      } else if (inputBTString.equals("UP") || inputSLString.equals("UP")) {
        SmartCar_Go();
        bluetooth.print("OK");
        inputBTString = "";
        inputSLString = "";
        
      } else if (inputBTString.equals("DOWN") || inputSLString.equals("DOWN")) {
        SmartCar_Back();
        bluetooth.print("OK");
        inputBTString = "";
        inputSLString = "";
        
      } else if (inputBTString.equals("SERVOUP") || inputSLString.equals("SERVOUP")) {
        myservo.attach(SERVO_PIN);
        myservo.write(180);
        delay(500);
        myservo.detach();
        bluetooth.print("OK");
        inputBTString = "";
        inputSLString = "";
        
      } else if (inputBTString.equals("SERVODOWN") || inputSLString.equals("SERVODOWN")) {
        myservo.attach(SERVO_PIN);
        myservo.write(0);
        delay(500);
        myservo.detach();
        bluetooth.print("OK");
        inputBTString = "";
        inputSLString = "";
        
      } else if (inputBTString.equals("SERVOMID") || inputSLString.equals("SERVOMID")) {
        myservo.attach(SERVO_PIN);
        myservo.write(90);
        delay(500);
        myservo.detach();
        bluetooth.print("OK");
        inputBTString = "";
        inputSLString = "";
        
      } else if (inputBTString.startsWith("SPEED") || inputSLString.startsWith("SPEED")) {
        // 속도 조절 명령 (예: SPEED150)
        String speedStr = inputBTString.length() > 0 ? inputBTString.substring(5) : inputSLString.substring(5);
        int newSpeed = speedStr.toInt();
        if (newSpeed >= 0 && newSpeed <= 255) {
          L_MotorSpeed = newSpeed;
          R_MotorSpeed = newSpeed;
          Serial.print("Speed set to: ");
          Serial.println(newSpeed);
        }
        bluetooth.print("OK");
        inputBTString = "";
        inputSLString = "";
        
      } else if (inputBTString.equals("AVOIDON") || inputSLString.equals("AVOIDON")) {
        // 장애물 회피 모드 ON
        obstacleAvoidMode = true;
        Serial.println("Obstacle Avoidance: ON");
        tone(PIEZO, 1500, 100);
        delay(150);
        tone(PIEZO, 1500, 100);
        bluetooth.print("OK");
        inputBTString = "";
        inputSLString = "";
        
      } else if (inputBTString.equals("AVOIDOFF") || inputSLString.equals("AVOIDOFF")) {
        // 장애물 회피 모드 OFF
        obstacleAvoidMode = false;
        Serial.println("Obstacle Avoidance: OFF");
        tone(PIEZO, 500, 200);
        bluetooth.print("OK");
        inputBTString = "";
        inputSLString = "";
        
      } else if (inputBTString.equals("STATUS") || inputSLString.equals("STATUS")) {
        // 현재 상태 확인
        Serial.println("=== STATUS ===");
        Serial.print("Obstacle Avoidance: ");
        Serial.println(obstacleAvoidMode ? "ON" : "OFF");
        Serial.print("Motor Speed: ");
        Serial.println(L_MotorSpeed);
        bluetooth.print(obstacleAvoidMode ? "AVOID:ON" : "AVOID:OFF");
        inputBTString = "";
        inputSLString = "";
        
      } else {
        // 알 수 없는 명령
        Serial.print("Unknown command: ");
        Serial.println(inputBTString);
        tone(PIEZO, 500, 100);
        inputBTString = "";
        inputSLString = "";
      }
      
      // 문자 초기화
      inBTChar = 0;
      inSLChar = 0;
      
    } else {
      if (inBTChar != 0 && inBTChar != -1) {
        inputBTString += inBTChar;
      }
      if (inSLChar != 0 && inSLChar != -1) {
        inputSLString += inSLChar;
      }
      delay(30);
    }
  }
  
  // 장애물 회피 모드가 ON일 때만 초음파 센서로 장애물 감지
  if (obstacleAvoidMode) {
    long dist = getDistance();
    
    if (dist > 0 && dist < 10) {
      // 장애물 감지됨
      tone(PIEZO, 1000, 100);
      Serial.println("Obstacle detected!");
      
      SmartCar_Stop();
      SmartCar_Back();
      delay(500);
      
      // 랜덤하게 좌/우 회전
      int toggle = random(1, 3);
      if (toggle == 2) {
        SmartCar_Left();
        delay(500);
      } else {
        SmartCar_Right();
        delay(500);
      }
      SmartCar_Stop();
    }
  }
}
