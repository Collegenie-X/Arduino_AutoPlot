/*******************************************************************************
 * 단순 따라가기 자동차 (Simple Chase Car)
 * 
 * 동작:
 * - 물체 보이면 → 무조건 직진! (쫓아감)
 * - 물체 없으면 → 정지 + 가끔 서보 좌우 탐색
 ******************************************************************************/

#include <Servo.h>

/*******************************************************************************
 * 핀 설정
 ******************************************************************************/
int trigPin = 13;           // 초음파 Trig
int echoPin = 12;           // 초음파 Echo
int servoPin = 2;           // 서보모터
int buzzerPin = 3;          // 피에조 부저

// 모터 드라이버
int RightMotor_E_pin = 5;   // 오른쪽 PWM
int RightMotor_1_pin = 8;   // 오른쪽 IN1
int RightMotor_2_pin = 9;   // 오른쪽 IN2
int LeftMotor_3_pin = 10;   // 왼쪽 IN3
int LeftMotor_4_pin = 11;   // 왼쪽 IN4
int LeftMotor_E_pin = 6;    // 왼쪽 PWM

/*******************************************************************************
 * 파라미터 설정
 ******************************************************************************/
int MAX_DIST = 150;         // 최대 감지 거리 (cm)
int MOTOR_SPEED = 150;      // 모터 속도
int SERVO_INTERVAL = 2000;  // 서보 탐색 간격 (2초)

/*******************************************************************************
 * 전역 변수
 ******************************************************************************/
Servo myServo;
long distance = 0;
unsigned long lastServoTime = 0;
int noTargetCount = 0;

/*******************************************************************************
 * Setup
 ******************************************************************************/
void setup() {
  Serial.begin(9600);
  Serial.println("=== Simple Chase Car ===");
  
  // 초음파 센서
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);
  
  // 서보모터 (90도 = 정면)
  myServo.attach(servoPin);
  myServo.write(90);
  delay(500);
  
  // 부저
  pinMode(buzzerPin, OUTPUT);
  
  // 모터
  pinMode(RightMotor_E_pin, OUTPUT);
  pinMode(RightMotor_1_pin, OUTPUT);
  pinMode(RightMotor_2_pin, OUTPUT);
  pinMode(LeftMotor_3_pin, OUTPUT);
  pinMode(LeftMotor_4_pin, OUTPUT);
  pinMode(LeftMotor_E_pin, OUTPUT);
  
  // 정지
  stopCar();
  
  // 시작음
  tone(buzzerPin, 1000, 200);
  delay(300);
  tone(buzzerPin, 1500, 200);
  
  Serial.println("Ready! Chase mode!");
  Serial.println("");
}

/*******************************************************************************
 * Main Loop
 ******************************************************************************/
void loop() {
  // 거리 측정
  distance = getDistance();
  
  // 시리얼 출력
  Serial.print("Dist: ");
  Serial.print(distance);
  Serial.print(" cm -> ");
  
  // 물체 감지 확인
  if (distance > 0 && distance < MAX_DIST) {
    // 물체 감지됨! → 무조건 직진!
    Serial.println("GO!");
    tone(buzzerPin, 1000, 20);  // 추적 비프음
    goForward();
    noTargetCount = 0;
    lastServoTime = millis();  // 타이머 리셋
  }
  else {
    // 물체 없음 → 정지
    Serial.println("STOP");
    stopCar();
    noTargetCount++;
    
    // 5초마다 서보 좌우 탐색
    if (millis() - lastServoTime > SERVO_INTERVAL) {
      doServoSearch();
      lastServoTime = millis();
    }
  }
  
  delay(100);
}

/*******************************************************************************
 * 서보 좌우 탐색 (가끔만 실행)
 ******************************************************************************/
void doServoSearch() {
  Serial.println(">> Servo search...");
  tone(buzzerPin, 500, 100);
  
  // 왼쪽 보기
  myServo.write(140);
  delay(400);
  
  // 오른쪽 보기
  myServo.write(40);
  delay(400);
  
  // 다시 정면
  myServo.write(90);
  delay(300);
}

/*******************************************************************************
 * 거리 측정
 ******************************************************************************/
long getDistance() {
  long duration;
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH, 30000);
  
  if (duration == 0) {
    return 999;  // 감지 안됨
  }
  
  return duration / 58;
}

/*******************************************************************************
 * 모터 제어
 ******************************************************************************/
void goForward() {
  digitalWrite(RightMotor_1_pin, HIGH);
  digitalWrite(RightMotor_2_pin, LOW);
  digitalWrite(LeftMotor_3_pin, HIGH);
  digitalWrite(LeftMotor_4_pin, LOW);
  analogWrite(RightMotor_E_pin, MOTOR_SPEED);
  analogWrite(LeftMotor_E_pin, MOTOR_SPEED);
}

void stopCar() {
  analogWrite(RightMotor_E_pin, 0);
  analogWrite(LeftMotor_E_pin, 0);
}
