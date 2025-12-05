/* 
 * ============================================================================
 * 에듀이노 SmartCar - 라인 트레이서 (Line Tracer)
 * ============================================================================
 * 기능: 검정색 라인을 따라 자율 주행하는 스마트카
 * 버전: 3.0 (교육용 - 단순화)
 * ============================================================================
 */

#include <Servo.h>

// ============================================================================
// [설정값] - 이 값들만 수정하세요!
// ============================================================================

// ★★★ 가장 중요한 설정 ★★★
int SPEED = 100;              // 기본 속도 (50~150)
int LOOP_DELAY = 300;         // 센서 체크 속도 (낮을수록 빠름, 50~500)
int MOTOR_REVERSE = 1;        // 회전 방향 반전 (1: 반전, 0: 정상)

// ----- 핀 설정 -----
int PIN_MOTOR_RIGHT_EN = 5;   // 오른쪽 모터 PWM
int PIN_MOTOR_LEFT_EN  = 6;   // 왼쪽 모터 PWM
int PIN_MOTOR_RIGHT_1  = 8;   // 오른쪽 모터 방향1
int PIN_MOTOR_RIGHT_2  = 9;   // 오른쪽 모터 방향2
int PIN_MOTOR_LEFT_1   = 10;  // 왼쪽 모터 방향1
int PIN_MOTOR_LEFT_2   = 11;  // 왼쪽 모터 방향2

int PIN_SENSOR_LEFT   = A3;   // 왼쪽 라인 센서
int PIN_SENSOR_CENTER = A4;   // 중앙 라인 센서
int PIN_SENSOR_RIGHT  = A5;   // 오른쪽 라인 센서

int PIN_BUZZER = 3;           // 피에조 부저
int PIN_SERVO  = 2;           // 서보모터

// ----- 속도 세부 설정 -----
int SPEED_MAX       = 180;    // 최대 속도
int SPEED_MIN       = 50;     // 최소 속도 (회전 시 안쪽 바퀴)
int SPEED_TURN_FAST = 150;    // 급회전 바깥 바퀴
int SPEED_TURN_SLOW = 0;      // 급회전 안쪽 바퀴

// ----- PID 설정 -----
float PID_KP = 35.0;          // 비례 게인 (높일수록 회전 크게)
float PID_KI = 0.0;           // 적분 게인
float PID_KD = 20.0;          // 미분 게인

// ----- 시스템 설정 -----
int DEBUG_MODE = 1;           // 1: 시리얼 출력 ON, 0: OFF
int LOST_COUNT_MAX = 30;      // 라인 이탈 허용 횟수 (높일수록 더 오래 직진)
int LOST_SPEED = 80;          // 0,0,0 일 때 직진 속도 (라인 찾기용)

// ----- 서보 설정 -----
int SERVO_CENTER = 90;        // 중앙 각도
int SERVO_LEFT   = 135;       // 왼쪽 각도
int SERVO_RIGHT  = 45;        // 오른쪽 각도
int SERVO_DELAY  = 500;       // 서보 이동 딜레이

// ============================================================================
// [전역 변수]
// ============================================================================

// 센서값 (0 또는 1)
int sensorLeft   = 0;
int sensorCenter = 0;
int sensorRight  = 0;

// 이전 센서값 저장
int prevLeft   = 1;
int prevCenter = 1;
int prevRight  = 1;

// 라인 이탈 카운터
int lostCount = 0;

// PID 변수
int pidLastError = 0;
int pidIntegral  = 0;

// 서보모터
Servo servoMotor;
unsigned long lastServoTime = 0;
int servoSweepState = 0;      // 0:중앙, 1:왼쪽, 2:오른쪽

// ============================================================================
// [setup 함수]
// ============================================================================

void setup() {
  // 모터 핀 설정
  pinMode(PIN_MOTOR_RIGHT_EN, OUTPUT);
  pinMode(PIN_MOTOR_LEFT_EN, OUTPUT);
  pinMode(PIN_MOTOR_RIGHT_1, OUTPUT);
  pinMode(PIN_MOTOR_RIGHT_2, OUTPUT);
  pinMode(PIN_MOTOR_LEFT_1, OUTPUT);
  pinMode(PIN_MOTOR_LEFT_2, OUTPUT);
  
  // 부저 핀 설정
  pinMode(PIN_BUZZER, OUTPUT);
  
  // 서보모터 설정
  servoMotor.attach(PIN_SERVO);
  servoMotor.write(SERVO_CENTER);
  
  // 시리얼 통신 시작
  Serial.begin(9600);
  Serial.println("=== Line Tracer Start ===");
  Serial.print("SPEED: ");
  Serial.println(SPEED);
  
  // 시작음
  playStartSound();
}

// ============================================================================
// [loop 함수]
// ============================================================================

void loop() {
  // 1. 센서 읽기
  readSensors();
  
  // 2. 센서값 출력 (디버그)
  if (DEBUG_MODE == 1) {
    printSensorValues();
  }
  
  // 3. 라인 이탈 체크 (0,0,0)
  if (sensorLeft == 0 && sensorCenter == 0 && sensorRight == 0) {
    // 라인 이탈!
    lostCount++;
    
    if (lostCount > LOST_COUNT_MAX) {
      // 완전 이탈 - 정지 및 서보 흔들기
      motorStop();
      buzzerWarning();
      servoSearchLine();  // 2초마다 서보 좌우 흔들기
      
      if (DEBUG_MODE == 1) {
        Serial.println("LINE LOST - SEARCHING");
      }
    } else {
      // ★ 사각지대: 조금 직진하면서 라인 찾기 ★
      motorDrive(LOST_SPEED, LOST_SPEED, 1, 1);
      
      if (DEBUG_MODE == 1) {
        Serial.println("0,0,0 - SLOW FORWARD");
      }
    }
  } else {
    // 라인 감지됨 - 정상 주행
    lostCount = 0;
    lastServoTime = 0;
    servoMotor.write(SERVO_CENTER);
    followLine();
  }
  
  // 4. 현재 센서값 저장
  prevLeft   = sensorLeft;
  prevCenter = sensorCenter;
  prevRight  = sensorRight;
  
  delay(LOOP_DELAY);
}

// ============================================================================
// [센서 함수]
// ============================================================================

void readSensors() {
  sensorLeft   = digitalRead(PIN_SENSOR_LEFT);
  sensorCenter = digitalRead(PIN_SENSOR_CENTER);
  sensorRight  = digitalRead(PIN_SENSOR_RIGHT);
}

void printSensorValues() {
  Serial.print("LEFT:");
  Serial.print(sensorLeft);
  Serial.print(" CENTER:");
  Serial.print(sensorCenter);
  Serial.print(" RIGHT:");
  Serial.println(sensorRight);
}

// ============================================================================
// [라인 추적 함수]
// ============================================================================

void followLine() {
  // 에러값 계산
  int error = calculateError();
  
  // 센서 조합별 처리
  if (sensorLeft == 0 && sensorCenter == 1 && sensorRight == 0) {
    // 0 1 0: 직진
    motorPID(error);
    buzzerOff();
  }
  else if (sensorLeft == 0 && sensorCenter == 1 && sensorRight == 1) {
    // 0 1 1: 약간 우회전
    motorPID(error);
    buzzerRight();
  }
  else if (sensorLeft == 0 && sensorCenter == 0 && sensorRight == 1) {
    // 0 0 1: 급우회전
    motorSharpTurn(1);  // 1 = 우회전
    buzzerRight();
  }
  else if (sensorLeft == 1 && sensorCenter == 1 && sensorRight == 0) {
    // 1 1 0: 약간 좌회전
    motorPID(error);
    buzzerLeft();
  }
  else if (sensorLeft == 1 && sensorCenter == 0 && sensorRight == 0) {
    // 1 0 0: 급좌회전
    motorSharpTurn(0);  // 0 = 좌회전
    buzzerLeft();
  }
  else if (sensorLeft == 1 && sensorCenter == 1 && sensorRight == 1) {
    // 1 1 1: 교차로 - 직진
    motorPID(0);
    buzzerOff();
  }
  else if (sensorLeft == 1 && sensorCenter == 0 && sensorRight == 1) {
    // 1 0 1: T자 교차로
    motorPID(0);
    buzzerOff();
  }
}

int calculateError() {
  if (sensorLeft == 0 && sensorCenter == 1 && sensorRight == 0) {
    return 0;   // 정중앙
  }
  else if (sensorLeft == 0 && sensorCenter == 1 && sensorRight == 1) {
    return 1;   // 약간 오른쪽
  }
  else if (sensorLeft == 0 && sensorCenter == 0 && sensorRight == 1) {
    return 2;   // 많이 오른쪽
  }
  else if (sensorLeft == 1 && sensorCenter == 1 && sensorRight == 0) {
    return -1;  // 약간 왼쪽
  }
  else if (sensorLeft == 1 && sensorCenter == 0 && sensorRight == 0) {
    return -2;  // 많이 왼쪽
  }
  return 0;
}

// ============================================================================
// [모터 제어 함수]
// ============================================================================

void motorPID(int error) {
  // PID 계산
  pidIntegral += error;
  pidIntegral = constrain(pidIntegral, -50, 50);
  
  int derivative = error - pidLastError;
  int correction = (PID_KP * error) + (PID_KI * pidIntegral) + (PID_KD * derivative);
  
  pidLastError = error;
  
  // 속도 계산 (MOTOR_REVERSE로 방향 조절)
  int leftSpeed;
  int rightSpeed;
  
  if (MOTOR_REVERSE == 1) {
    // 반전 모드
    leftSpeed  = SPEED + correction;
    rightSpeed = SPEED - correction;
  } else {
    // 정상 모드
    leftSpeed  = SPEED - correction;
    rightSpeed = SPEED + correction;
  }
  
  // 속도 제한
  leftSpeed  = constrain(leftSpeed, SPEED_MIN, SPEED_MAX);
  rightSpeed = constrain(rightSpeed, SPEED_MIN, SPEED_MAX);
  
  // 모터 구동
  motorDrive(leftSpeed, rightSpeed, 1, 1);
}

void motorSharpTurn(int turnRight) {
  if (MOTOR_REVERSE == 1) {
    // 반전 모드
    if (turnRight == 1) {
      motorDrive(SPEED_TURN_SLOW, SPEED_TURN_FAST, 1, 1);  // 우회전
    } else {
      motorDrive(SPEED_TURN_FAST, SPEED_TURN_SLOW, 1, 1);  // 좌회전
    }
  } else {
    // 정상 모드
    if (turnRight == 1) {
      motorDrive(SPEED_TURN_FAST, SPEED_TURN_SLOW, 1, 1);  // 우회전
    } else {
      motorDrive(SPEED_TURN_SLOW, SPEED_TURN_FAST, 1, 1);  // 좌회전
    }
  }
}

void motorStop() {
  analogWrite(PIN_MOTOR_LEFT_EN, 0);
  analogWrite(PIN_MOTOR_RIGHT_EN, 0);
}

void motorDrive(int leftSpeed, int rightSpeed, int leftForward, int rightForward) {
  // 왼쪽 모터 방향
  if (leftForward == 1) {
    digitalWrite(PIN_MOTOR_LEFT_1, HIGH);
    digitalWrite(PIN_MOTOR_LEFT_2, LOW);
  } else {
    digitalWrite(PIN_MOTOR_LEFT_1, LOW);
    digitalWrite(PIN_MOTOR_LEFT_2, HIGH);
  }
  
  // 오른쪽 모터 방향
  if (rightForward == 1) {
    digitalWrite(PIN_MOTOR_RIGHT_1, HIGH);
    digitalWrite(PIN_MOTOR_RIGHT_2, LOW);
  } else {
    digitalWrite(PIN_MOTOR_RIGHT_1, LOW);
    digitalWrite(PIN_MOTOR_RIGHT_2, HIGH);
  }
  
  // 속도 설정
  analogWrite(PIN_MOTOR_LEFT_EN, leftSpeed);
  analogWrite(PIN_MOTOR_RIGHT_EN, rightSpeed);
}

// ============================================================================
// [서보모터 함수]
// ============================================================================

void servoSearchLine() {
  unsigned long currentTime = millis();
  
  // 2초(2000ms)마다 서보 흔들기
  if (currentTime - lastServoTime >= 2000 || lastServoTime == 0) {
    lastServoTime = currentTime;
    
    // 빠르게 좌우로 흔들기
    servoMotor.write(SERVO_LEFT);
    delay(SERVO_DELAY);
    servoMotor.write(SERVO_CENTER);
    delay(SERVO_DELAY);
    servoMotor.write(SERVO_RIGHT);
    delay(SERVO_DELAY);
    servoMotor.write(SERVO_CENTER);
    
    if (DEBUG_MODE == 1) {
      Serial.println("SERVO SWEEP!");
    }
  }
}

// ============================================================================
// [부저 함수]
// ============================================================================

void buzzerLeft() {
  tone(PIN_BUZZER, 500, 50);   // 낮은 음
}

void buzzerRight() {
  tone(PIN_BUZZER, 1000, 50);  // 높은 음
}

void buzzerWarning() {
  tone(PIN_BUZZER, 2000, 200); // 경고음
}

void buzzerOff() {
  noTone(PIN_BUZZER);
}

void playStartSound() {
  tone(PIN_BUZZER, 523, 100);  // 도
  delay(120);
  tone(PIN_BUZZER, 659, 100);  // 미
  delay(120);
  tone(PIN_BUZZER, 784, 150);  // 솔
  delay(200);
  noTone(PIN_BUZZER);
}
