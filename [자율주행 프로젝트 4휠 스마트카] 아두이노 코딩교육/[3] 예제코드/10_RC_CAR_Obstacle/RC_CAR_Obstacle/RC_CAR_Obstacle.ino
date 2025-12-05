/*******************************************************************************
 * 미로 탈출 스마트카 (Maze Solving Smart Car)
 * 
 * 원본: 에듀이노(Eduino) SmartCar 예제
 * 개선: 미로 탈출 알고리즘 적용 (Wall Following + Multi-angle Scanning)
 * 
 * 알고리즘 설명:
 * 1. 우수법(Right-Hand Rule): 오른쪽 벽을 따라가며 미로 탈출
 * 2. 다중 각도 스캔: 5개 방향(30, 60, 90, 120, 150도)으로 거리 측정
 * 3. 막다른 길 감지: 모든 방향이 막혀있으면 180도 회전
 * 4. 최적 경로 선택: 가장 열린 방향으로 이동
 ******************************************************************************/

#include <Servo.h>

/*******************************************************************************
 * 핀 설정 (Pin Configuration)
 ******************************************************************************/
// 초음파 센서 핀
const int PIN_TRIG = 13;
const int PIN_ECHO = 12;

// 서보모터 핀
const int PIN_SERVO = 2;

// 모터 드라이버 핀
const int PIN_MOTOR_R_EN  = 5;   // 오른쪽 모터 Enable (PWM)
const int PIN_MOTOR_L_EN  = 6;   // 왼쪽 모터 Enable (PWM)
const int PIN_MOTOR_R_IN1 = 8;   // 오른쪽 모터 IN1
const int PIN_MOTOR_R_IN2 = 9;   // 오른쪽 모터 IN2
const int PIN_MOTOR_L_IN3 = 10;  // 왼쪽 모터 IN3
const int PIN_MOTOR_L_IN4 = 11;  // 왼쪽 모터 IN4

/*******************************************************************************
 * 파라미터 설정 (Configurable Parameters)
 ******************************************************************************/
// 거리 임계값 (단위: mm) - 더 멀리서 감지하도록 증가
const int DIST_DANGER  = 250;    // 위험 거리 - 즉시 후진
const int DIST_WARNING = 400;    // 경고 거리 - 감속 및 스캔
const int DIST_SAFE    = 500;    // 안전 거리 - 정상 주행
const int DIST_MAX     = 600;    // 측정 가능 최대 거리

// 벽 따라가기 파라미터
const int WALL_DIST_MIN = 300;   // 벽과의 최소 거리
const int WALL_DIST_MAX = 500;   // 벽과의 최대 거리

const int SPEED = 30;
// 모터 속도 (0-255) - 속도 낮춤
const int SPEED_FAST   = 130 + SPEED;    // 빠른 속도
const int SPEED_NORMAL = 100 + SPEED;    // 보통 속도
const int SPEED_SLOW   = 80 + SPEED;     // 느린 속도
const int SPEED_TURN   = 150;    // 회전 속도

// 회전 시간 (ms) - 크게 회전하도록 증가
const int TIME_TURN_90  = 600;   // 90도 회전 시간 (증가)
const int TIME_TURN_180 = 1200;  // 180도 회전 시간 (증가)
const int TIME_BACKUP   = 1000;  // 후진 시간 (1초로 증가!)

// 서보 각도 설정
const int SERVO_LEFT   = 150;    // 왼쪽 스캔 각도
const int SERVO_CENTER = 90;     // 중앙 각도
const int SERVO_RIGHT  = 30;     // 오른쪽 스캔 각도

// 스캔 각도 배열 (다중 각도 스캔용)
const int SCAN_ANGLES[]   = {30, 60, 90, 120, 150};
const int NUM_SCAN_ANGLES = 5;

// 디버그 모드 (시리얼 출력 활성화)
const bool DEBUG_MODE = 1;

/*******************************************************************************
 * 차량 상태 정의 (Vehicle States)
 ******************************************************************************/
enum car_state_t {
  STATE_FORWARD,     // 전진
  STATE_SCANNING,    // 주변 스캔 중
  STATE_TURN_LEFT,   // 좌회전
  STATE_TURN_RIGHT,  // 우회전
  STATE_BACKUP,      // 후진
  STATE_U_TURN,      // 180도 회전
  STATE_WALL_FOLLOW  // 벽 따라가기
};

/*******************************************************************************
 * 전역 변수 (Global Variables)
 ******************************************************************************/
Servo servo_sensor;
car_state_t current_state = STATE_FORWARD;
int scan_distances[NUM_SCAN_ANGLES];  // 각 각도별 거리 저장

/*******************************************************************************
 * 함수 선언 (Function Declarations)
 ******************************************************************************/
// 상태 처리 함수
void handle_forward(int front_dist);
void handle_scanning(void);
void handle_backup(void);
void handle_wall_follow(void);

// 스캔 및 분석 함수
void scan_all_directions(void);
int  analyze_scan_results(void);
void execute_decision(int direction);

// 모터 제어 함수
void motor_forward(int speed);
void motor_backward(int speed);
void motor_stop(void);
void motor_turn(int right_speed, int left_speed);
void turn_left(int duration);
void turn_right(int duration);
void u_turn(void);
void set_motor_direction(int right_dir, int left_dir);
void set_motor_speed(int right_speed, int left_speed);

// 센서 함수
int measure_distance(void);

/*******************************************************************************
 * 초기화 함수 (Setup)
 ******************************************************************************/
void setup() {
  // 시리얼 통신 초기화
  Serial.begin(9600);
  
  // 서보모터 초기화
  servo_sensor.attach(PIN_SERVO);
  servo_sensor.write(SERVO_CENTER);
  
  // 초음파 센서 핀 모드 설정
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_TRIG, OUTPUT);
  
  // 모터 핀 모드 설정
  pinMode(PIN_MOTOR_R_EN, OUTPUT);
  pinMode(PIN_MOTOR_L_EN, OUTPUT);
  pinMode(PIN_MOTOR_R_IN1, OUTPUT);
  pinMode(PIN_MOTOR_R_IN2, OUTPUT);
  pinMode(PIN_MOTOR_L_IN3, OUTPUT);
  pinMode(PIN_MOTOR_L_IN4, OUTPUT);
  
  // 초기 상태
  motor_stop();
  delay(1000);
  
  Serial.println("=== Maze Solving Smart Car Started! ===");
  Serial.println("Wall Following (Right-Hand Rule) Algorithm");
}

/*******************************************************************************
 * 메인 루프 (Main Loop)
 ******************************************************************************/
void loop() {
  // 전방 거리 측정
  int front_dist = measure_distance();
  
  // 상태 머신 기반 동작
  switch (current_state) {
    case STATE_FORWARD:
      handle_forward(front_dist);
      break;
      
    case STATE_SCANNING:
      handle_scanning();
      break;
      
    case STATE_BACKUP:
      handle_backup();
      break;
      
    case STATE_TURN_LEFT:
    case STATE_TURN_RIGHT:
    case STATE_U_TURN:
      // 회전은 해당 함수에서 완료 후 STATE_FORWARD로 전환
      break;
      
    case STATE_WALL_FOLLOW:
      handle_wall_follow();
      break;
  }
  
  delay(100);  // 루프 안정화 (100ms)
}

/*******************************************************************************
 * 상태 처리 함수들 (State Handlers)
 ******************************************************************************/

// 전진 상태 처리
void handle_forward(int front_dist) {
  Serial.print("[DIST] ");
  Serial.print(front_dist);
  Serial.println(" mm");
  
  if (front_dist < DIST_DANGER) {
    // 위험! 즉시 정지 → 1초 후진 → 스캔
    Serial.println("[!!!] DANGER - BACKUP 1 SEC!");
    motor_stop();
    delay(100);
    current_state = STATE_BACKUP;
  }
  else if (front_dist < DIST_WARNING) {
    // 경고 - 정지 후 주변 스캔
    Serial.println("[!] Obstacle - STOP & SCAN");
    motor_stop();
    delay(200);
    current_state = STATE_SCANNING;
  }
  else if (front_dist < DIST_SAFE) {
    // 주의 - 느린 속도로 전진
    Serial.println("[~] Slow Forward");
    motor_forward(SPEED_SLOW);
  }
  else {
    // 안전 - 정상 속도 전진
    motor_forward(SPEED_NORMAL);
  }
}

// 주변 스캔 처리 (다중 각도)
void handle_scanning(void) {
  Serial.println("[>>] Multi-angle Scan Start");
  
  // 모든 각도에서 거리 측정
  scan_all_directions();
  
  // 스캔 결과 분석 및 방향 결정
  int best_direction = analyze_scan_results();
  
  // 서보 중앙 복귀
  servo_sensor.write(SERVO_CENTER);
  delay(200);
  
  // 분석 결과에 따라 행동 결정
  execute_decision(best_direction);
}

// 후진 처리 - 1초간 충분히 후진!
void handle_backup(void) {
  Serial.println("[<<] BACKUP 1 SECOND!");
  
  // 1초간 후진
  motor_backward(SPEED_NORMAL);
  delay(TIME_BACKUP);  // 1000ms = 1초
  motor_stop();
  delay(300);
  
  Serial.println("[<<] Backup Done - Now Scanning");
  current_state = STATE_SCANNING;
}

// 벽 따라가기 처리 (우수법)
void handle_wall_follow(void) {
  // 오른쪽 벽 거리 측정
  servo_sensor.write(SERVO_RIGHT);
  delay(150);
  int right_dist = measure_distance();
  
  // 전방 거리 측정
  servo_sensor.write(SERVO_CENTER);
  delay(150);
  int front_dist = measure_distance();
  
  if (front_dist < DIST_WARNING) {
    // 전방 막힘 - 좌회전
    turn_left(TIME_TURN_90);
    current_state = STATE_FORWARD;
  }
  else if (right_dist > WALL_DIST_MAX) {
    // 오른쪽 열림 - 우회전 (새로운 통로)
    Serial.println("[>>] Right Passage Found!");
    motor_forward(SPEED_SLOW);
    delay(300);
    turn_right(TIME_TURN_90);
    current_state = STATE_FORWARD;
  }
  else if (right_dist < WALL_DIST_MIN) {
    // 오른쪽 벽 너무 가까움 - 약간 좌회전
    motor_turn(SPEED_SLOW, SPEED_NORMAL);
    delay(100);
  }
  else {
    // 적정 거리 유지하며 전진
    motor_forward(SPEED_NORMAL);
  }
}

/*******************************************************************************
 * 스캔 및 분석 함수들 (Scanning Functions)
 ******************************************************************************/

// 모든 방향 스캔
void scan_all_directions(void) {
  for (int i = 0; i < NUM_SCAN_ANGLES; i++) {
    servo_sensor.write(SCAN_ANGLES[i]);
    delay(200);
    scan_distances[i] = measure_distance();
    
    if (DEBUG_MODE) {
      Serial.print("  Angle ");
      Serial.print(SCAN_ANGLES[i]);
      Serial.print(" deg: ");
      Serial.print(scan_distances[i]);
      Serial.println(" mm");
    }
  }
}

// 스캔 결과 분석 (반환: -1=좌회전, 0=직진, 1=우회전, 2=U턴)
// 더 명확하게 좌/우 중 넓은 쪽으로 크게 회전!
int analyze_scan_results(void) {
  // 좌측 평균 (150도, 120도)
  int left_avg = (scan_distances[4] + scan_distances[3]) / 2;
  // 중앙 (90도)
  int center_dist = scan_distances[2];
  // 우측 평균 (30도, 60도)
  int right_avg = (scan_distances[0] + scan_distances[1]) / 2;
  
  // 최대 거리 찾기
  int max_dist = 0;
  for (int i = 0; i < NUM_SCAN_ANGLES; i++) {
    if (scan_distances[i] > max_dist) {
      max_dist = scan_distances[i];
    }
  }
  
  Serial.println("========== SCAN RESULT ==========");
  Serial.print("  LEFT:   ");
  Serial.print(left_avg);
  Serial.println(" mm");
  Serial.print("  CENTER: ");
  Serial.print(center_dist);
  Serial.println(" mm");
  Serial.print("  RIGHT:  ");
  Serial.print(right_avg);
  Serial.println(" mm");
  Serial.println("=================================");
  
  // 1. 모든 방향 막힘 → U턴
  if (max_dist < DIST_DANGER) {
    Serial.println(">>> DECISION: U-TURN (Dead End!)");
    return 2;
  }
  
  // 2. 중앙이 충분히 열려있으면 → 직진
  if (center_dist > DIST_SAFE) {
    Serial.println(">>> DECISION: GO STRAIGHT");
    return 0;
  }
  
  // 3. 좌/우 비교해서 더 넓은 쪽으로 크게 회전!
  if (left_avg > right_avg + 50) {
    // 왼쪽이 50mm 이상 더 넓으면 좌회전
    Serial.println(">>> DECISION: TURN LEFT (More Space)");
    return -1;
  }
  else if (right_avg > left_avg + 50) {
    // 오른쪽이 50mm 이상 더 넓으면 우회전
    Serial.println(">>> DECISION: TURN RIGHT (More Space)");
    return 1;
  }
  else {
    // 비슷하면 우수법 적용 (오른쪽 우선)
    Serial.println(">>> DECISION: TURN RIGHT (Right-Hand Rule)");
    return 1;
  }
}

// 결정 실행 - 크게 회전!
void execute_decision(int direction) {
  switch (direction) {
    case -1:  // 좌회전 - 크게!
      Serial.println("[<<<] TURN LEFT!");
      turn_left(TIME_TURN_90);
      delay(100);
      // 회전 후 잠시 전진
      motor_forward(SPEED_SLOW);
      delay(300);
      break;
      
    case 0:   // 직진
      Serial.println("[^^^] GO FORWARD!");
      motor_forward(SPEED_NORMAL);
      break;
      
    case 1:   // 우회전 - 크게!
      Serial.println("[>>>] TURN RIGHT!");
      turn_right(TIME_TURN_90);
      delay(100);
      // 회전 후 잠시 전진
      motor_forward(SPEED_SLOW);
      delay(300);
      break;
      
    case 2:   // U턴
      Serial.println("[@@@] U-TURN!");
      u_turn();
      break;
  }
  
  motor_stop();
  delay(100);
  current_state = STATE_FORWARD;
}

/*******************************************************************************
 * 모터 제어 함수들 (Motor Control Functions)
 ******************************************************************************/

// 전진
void motor_forward(int speed) {
  set_motor_direction(HIGH, HIGH);
  set_motor_speed(speed, speed);
}

// 후진
void motor_backward(int speed) {
  set_motor_direction(LOW, LOW);
  set_motor_speed(speed, speed);
}

// 정지
void motor_stop(void) {
  set_motor_speed(0, 0);
}

// 좌회전 (제자리)
void turn_left(int duration) {
  Serial.println("[<<<] Turn Left");
  set_motor_direction(HIGH, LOW);  // 오른쪽 전진, 왼쪽 후진
  set_motor_speed(SPEED_TURN, SPEED_TURN);
  delay(duration);
  motor_stop();
  delay(100);
}

// 우회전 (제자리)
void turn_right(int duration) {
  Serial.println("[>>>] Turn Right");
  set_motor_direction(LOW, HIGH);  // 오른쪽 후진, 왼쪽 전진
  set_motor_speed(SPEED_TURN, SPEED_TURN);
  delay(duration);
  motor_stop();
  delay(100);
}

// U턴 (180도 회전)
void u_turn(void) {
  Serial.println("[@@@] U-Turn (180 deg)");
  motor_backward(SPEED_SLOW);
  delay(TIME_BACKUP);
  turn_right(TIME_TURN_180);
}

// 곡선 주행 (좌우 속도 다르게)
void motor_turn(int right_speed, int left_speed) {
  set_motor_direction(HIGH, HIGH);
  set_motor_speed(right_speed, left_speed);
}

// 모터 방향 설정 (LOW=후진, HIGH=전진)
void set_motor_direction(int right_dir, int left_dir) {
  digitalWrite(PIN_MOTOR_R_IN1, right_dir);
  digitalWrite(PIN_MOTOR_R_IN2, !right_dir);
  digitalWrite(PIN_MOTOR_L_IN3, left_dir);
  digitalWrite(PIN_MOTOR_L_IN4, !left_dir);
}

// 모터 속도 설정
void set_motor_speed(int right_speed, int left_speed) {
  analogWrite(PIN_MOTOR_R_EN, right_speed);
  analogWrite(PIN_MOTOR_L_EN, left_speed);
}

/*******************************************************************************
 * 센서 함수들 (Sensor Functions)
 ******************************************************************************/

// 초음파 거리 측정 (단위: mm)
int measure_distance(void) {
  long duration, distance;
  
  // 트리거 펄스 발생
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  
  // 에코 시간 측정
  duration = pulseIn(PIN_ECHO, HIGH, 30000);  // 타임아웃 30ms
  
  // 거리 계산 (음속 340m/s)
  if (duration == 0) {
    distance = DIST_MAX;  // 타임아웃 시 최대값
  } else {
    distance = (duration * 340) / 2000;
  }
  
  // 범위 제한
  if (distance > DIST_MAX) distance = DIST_MAX;
  if (distance < 0) distance = 0;
  
  return distance;
}
