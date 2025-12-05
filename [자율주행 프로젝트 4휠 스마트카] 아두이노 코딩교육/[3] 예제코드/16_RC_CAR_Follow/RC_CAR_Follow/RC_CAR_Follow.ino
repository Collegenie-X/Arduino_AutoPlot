/*******************************************************************************
 * 앞차 따라가기 스마트카 (Car Following Smart Car)
 * 
 * 기능:
 * 1. 전방 스캔 → 물체 있으면 따라가기
 * 2. 물체 없으면 → 서보 좌/우 스캔하여 찾기
 * 3. 모든 방향 없으면 → 정지
 * 4. 노이즈 필터링 (3회 측정 중앙값)
 ******************************************************************************/

#include <Servo.h>

/*******************************************************************************
 * 핀 설정
 ******************************************************************************/
// 초음파 센서
int trigPin = 13;
int echoPin = 12;

// 서보모터
int servoPin = 2;

// 피에조 부저
int buzzerPin = 3;

// 모터 드라이버
int RightMotor_E_pin = 5;
int RightMotor_1_pin = 8;
int RightMotor_2_pin = 9;
int LeftMotor_3_pin = 10;
int LeftMotor_4_pin = 11;
int LeftMotor_E_pin = 6;

// 모터 속도
int L_MotorSpeed = 153;
int R_MotorSpeed = 153;

/*******************************************************************************
 * 서보 각도 설정
 ******************************************************************************/
int SERVO_LEFT   = 150;   // 왼쪽
int SERVO_CENTER = 90;    // 중앙
int SERVO_RIGHT  = 30;    // 오른쪽

/*******************************************************************************
 * 거리 파라미터 (단위: cm)
 ******************************************************************************/
int DIST_TOO_CLOSE = 20;     // 20cm 이하 - 후진
int DIST_TARGET    = 35;     // 35cm - 목표 거리  
int DIST_FAR       = 60;     // 60cm 이상 - 빠른 전진
int DIST_MAX       = 150;    // 150cm 이상 - 감지 안됨

/*******************************************************************************
 * 전역 변수
 ******************************************************************************/
Servo myServo;
int currentServoPos = SERVO_CENTER;

// 각 방향 거리
long distLeft = 999;
long distCenter = 999;
long distRight = 999;

// 상태
bool isTracking = false;
int lostCount = 0;

/*******************************************************************************
 * 함수 선언
 ******************************************************************************/
long getDistance();
long getDistanceFiltered();
void servoMove(int angle);
void scanAllDirections();
int findTarget();

void SmartCar_Go(int speed);
void SmartCar_Back(int speed);
void SmartCar_Stop();
void SmartCar_Left(int speed);
void SmartCar_Right(int speed);

void beep(int freq, int dur);

/*******************************************************************************
 * Setup
 ******************************************************************************/
void setup() {
  Serial.begin(9600);
  Serial.println("=== Car Following v2 ===");
  
  // 초음파 센서
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);
  delay(50);
  Serial.println("[OK] Ultrasonic");
  
  // 서보모터
  myServo.attach(servoPin);
  myServo.write(SERVO_CENTER);
  currentServoPos = SERVO_CENTER;
  delay(500);
  Serial.println("[OK] Servo at 90");
  
  // 부저
  pinMode(buzzerPin, OUTPUT);
  Serial.println("[OK] Buzzer");
  
  // 모터
  pinMode(RightMotor_E_pin, OUTPUT);
  pinMode(RightMotor_1_pin, OUTPUT);
  pinMode(RightMotor_2_pin, OUTPUT);
  pinMode(LeftMotor_3_pin, OUTPUT);
  pinMode(LeftMotor_4_pin, OUTPUT);
  pinMode(LeftMotor_E_pin, OUTPUT);
  SmartCar_Stop();
  Serial.println("[OK] Motor");
  
  // 시작음
  beep(1000, 100);
  delay(100);
  beep(1500, 100);
  
  Serial.println("");
  Serial.println("Scanning...");
}

/*******************************************************************************
 * Main Loop
 ******************************************************************************/
void loop() {
  // 1. 서보 중앙으로 (전방 스캔)
  servoMove(SERVO_CENTER);
  delay(100);
  
  // 2. 전방 거리 측정 (필터링)
  distCenter = getDistanceFiltered();
  
  Serial.print("CENTER: ");
  Serial.print(distCenter);
  Serial.print(" cm");
  
  // 3. 전방에 물체 있는지 확인
  if (distCenter > 0 && distCenter < DIST_MAX) {
    // 물체 감지됨!
    if (!isTracking) {
      beep(1000, 100);
      isTracking = true;
      Serial.print(" [FOUND!]");
    }
    lostCount = 0;
    
    // 거리에 따라 동작
    if (distCenter < DIST_TOO_CLOSE) {
      Serial.println(" => BACK");
      beep(2000, 30);
      SmartCar_Back(100);
      delay(200);
      SmartCar_Stop();
    }
    else if (distCenter < DIST_TARGET) {
      Serial.println(" => OK STOP");
      SmartCar_Stop();
    }
    else if (distCenter < DIST_FAR) {
      Serial.println(" => SLOW");
      SmartCar_Go(100);
    }
    else {
      Serial.println(" => FAST");
      SmartCar_Go(150);
    }
  }
  else {
    // 전방에 없음 → 좌우 스캔
    Serial.println(" => NO TARGET, Scanning L/R...");
    
    lostCount++;
    
    // 좌우 스캔
    scanAllDirections();
    
    // 가장 가까운 방향 찾기
    int dir = findTarget();
    
    if (dir == -1) {
      // 왼쪽에 물체
      Serial.println("  >> Found LEFT! Turning...");
      beep(800, 50);
      SmartCar_Left(120);
      delay(300);
      SmartCar_Stop();
    }
    else if (dir == 1) {
      // 오른쪽에 물체
      Serial.println("  >> Found RIGHT! Turning...");
      beep(1200, 50);
      SmartCar_Right(120);
      delay(300);
      SmartCar_Stop();
    }
    else {
      // 모든 방향 없음
      Serial.println("  >> NO TARGET anywhere");
      if (isTracking) {
        beep(400, 200);
        isTracking = false;
      }
      SmartCar_Stop();
    }
  }
  
  delay(100);
}

/*******************************************************************************
 * 서보 이동 (충분한 대기)
 ******************************************************************************/
void servoMove(int angle) {
  if (currentServoPos == angle) return;
  
  int diff = abs(angle - currentServoPos);
  myServo.write(angle);
  currentServoPos = angle;
  
  // 각도 차이에 따른 대기 (60도당 150ms)
  int waitTime = (diff * 150) / 60;
  if (waitTime < 100) waitTime = 100;
  delay(waitTime);
}

/*******************************************************************************
 * 모든 방향 스캔
 ******************************************************************************/
void scanAllDirections() {
  // 왼쪽 스캔
  servoMove(SERVO_LEFT);
  delay(150);
  distLeft = getDistanceFiltered();
  Serial.print("  L:");
  Serial.print(distLeft);
  
  // 오른쪽 스캔
  servoMove(SERVO_RIGHT);
  delay(150);
  distRight = getDistanceFiltered();
  Serial.print(" R:");
  Serial.println(distRight);
  
  // 중앙 복귀
  servoMove(SERVO_CENTER);
}

/*******************************************************************************
 * 타겟 방향 찾기 (반환: -1=왼쪽, 0=없음, 1=오른쪽)
 ******************************************************************************/
int findTarget() {
  int minDist = DIST_MAX;
  int dir = 0;
  
  // 왼쪽 체크
  if (distLeft > 0 && distLeft < DIST_MAX && distLeft < minDist) {
    minDist = distLeft;
    dir = -1;
  }
  
  // 오른쪽 체크
  if (distRight > 0 && distRight < DIST_MAX && distRight < minDist) {
    minDist = distRight;
    dir = 1;
  }
  
  return dir;
}

/*******************************************************************************
 * 초음파 거리 측정 (단일)
 ******************************************************************************/
long getDistance() {
  long duration, dist;
  
  // 트리거 초기화
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // 트리거 펄스
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // 에코 측정 (타임아웃 30ms)
  duration = pulseIn(echoPin, HIGH, 30000);
  
  if (duration == 0) {
    return 999;
  }
  
  // cm 계산
  dist = (duration * 0.034) / 2;
  
  if (dist < 2) dist = 2;
  if (dist > 400) dist = 999;
  
  return dist;
}

/*******************************************************************************
 * 필터링된 거리 측정 (3회 측정, 중앙값)
 ******************************************************************************/
long getDistanceFiltered() {
  long d1, d2, d3, temp;
  
  d1 = getDistance();
  delay(30);
  d2 = getDistance();
  delay(30);
  d3 = getDistance();
  
  // 정렬 (버블)
  if (d1 > d2) { temp = d1; d1 = d2; d2 = temp; }
  if (d2 > d3) { temp = d2; d2 = d3; d3 = temp; }
  if (d1 > d2) { temp = d1; d1 = d2; d2 = temp; }
  
  // 중앙값 반환
  return d2;
}

/*******************************************************************************
 * 모터 제어
 ******************************************************************************/
void SmartCar_Go(int speed) {
  digitalWrite(RightMotor_1_pin, HIGH);
  digitalWrite(RightMotor_2_pin, LOW);
  digitalWrite(LeftMotor_3_pin, HIGH);
  digitalWrite(LeftMotor_4_pin, LOW);
  analogWrite(RightMotor_E_pin, speed);
  analogWrite(LeftMotor_E_pin, speed);
}

void SmartCar_Back(int speed) {
  digitalWrite(RightMotor_1_pin, LOW);
  digitalWrite(RightMotor_2_pin, HIGH);
  digitalWrite(LeftMotor_3_pin, LOW);
  digitalWrite(LeftMotor_4_pin, HIGH);
  analogWrite(RightMotor_E_pin, speed);
  analogWrite(LeftMotor_E_pin, speed);
}

void SmartCar_Stop() {
  analogWrite(RightMotor_E_pin, 0);
  analogWrite(LeftMotor_E_pin, 0);
}

void SmartCar_Left(int speed) {
  digitalWrite(RightMotor_1_pin, HIGH);
  digitalWrite(RightMotor_2_pin, LOW);
  digitalWrite(LeftMotor_3_pin, LOW);
  digitalWrite(LeftMotor_4_pin, HIGH);
  analogWrite(RightMotor_E_pin, speed);
  analogWrite(LeftMotor_E_pin, speed);
}

void SmartCar_Right(int speed) {
  digitalWrite(RightMotor_1_pin, LOW);
  digitalWrite(RightMotor_2_pin, HIGH);
  digitalWrite(LeftMotor_3_pin, HIGH);
  digitalWrite(LeftMotor_4_pin, LOW);
  analogWrite(RightMotor_E_pin, speed);
  analogWrite(LeftMotor_E_pin, speed);
}

/*******************************************************************************
 * 피에조 부저
 ******************************************************************************/
void beep(int freq, int dur) {
  tone(buzzerPin, freq, dur);
}
