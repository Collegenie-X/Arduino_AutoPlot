/*******************************************************************************
 * 적외선 리모컨 RC카 (IR Remote Control Car)
 * 
 * 원본: 에듀이노(Eduino) SmartCar 예제
 * 개선: 스네이크 케이스 + 모듈화 + 가독성 향상
 * 
 * 리모컨 조작:
 *   [2] 전진    [+] 속도 증가
 *   [4] 좌회전  [5] 정지  [6] 우회전
 *   [8] 후진    [-] 속도 감소
 ******************************************************************************/

#include <IRremote.h>

// 리피트 코드 (버튼을 누르고 있을 때 반복 전송됨)
#define IR_REPEAT_CODE  0xFFFFFFFF

/*******************************************************************************
 * IR 리모컨 버튼 코드 (16진수 / 10진수)
 ******************************************************************************/
// 숫자 버튼
#define IR_BTN_0      16738455   // 0xFF6897
#define IR_BTN_1      16724175   // 0xFF30CF
#define IR_BTN_2      1033561079   // 0xFF18E7 - 전진
#define IR_BTN_3      16743045   // 0xFF7A85
#define IR_BTN_4      2351064443   // 0xFF10EF - 좌회전
#define IR_BTN_5      2427583254   // 0xFF38C7 - 정지
#define IR_BTN_6      71952287   // 0xFF5AA5 - 우회전
#define IR_BTN_7      16728765   // 0xFF42BD
#define IR_BTN_8      4136307364   // 0xFF4AB5 - 후진
#define IR_BTN_9      16732845   // 0xFF52AD

// 기능 버튼
#define IR_BTN_PLUS   16754775   // 0xFFA857 - 속도 증가
#define IR_BTN_MINUS  2535091102   // 0xFFE01F - 속도 감소

/*******************************************************************************
 * 핀 설정
 ******************************************************************************/
// IR 수신 핀
const int PIN_IR_RECV = A0;

// 모터 드라이버 핀
const int PIN_MOTOR_R_EN  = 5;    // 오른쪽 모터 Enable (PWM)
const int PIN_MOTOR_L_EN  = 6;    // 왼쪽 모터 Enable (PWM)
const int PIN_MOTOR_R_IN1 = 8;    // 오른쪽 모터 IN1
const int PIN_MOTOR_R_IN2 = 9;    // 오른쪽 모터 IN2
const int PIN_MOTOR_L_IN3 = 10;   // 왼쪽 모터 IN3
const int PIN_MOTOR_L_IN4 = 11;   // 왼쪽 모터 IN4

/*******************************************************************************
 * 파라미터 설정
 ******************************************************************************/
const int SPEED_MIN     = 50;     // 최소 속도
const int SPEED_MAX     = 255;    // 최대 속도
const int SPEED_DEFAULT = 150;    // 기본 속도 (60%)
const int SPEED_STEP    = 20;     // 속도 증감 단위

// 회전 시 속도 비율
const float TURN_FAST_RATIO = 1.4;  // 바깥쪽 바퀴
const float TURN_SLOW_RATIO = 0.4;  // 안쪽 바퀴

/*******************************************************************************
 * 차량 상태 정의
 ******************************************************************************/
enum car_mode_t {
  MODE_STOP,        // 정지
  MODE_FORWARD,     // 전진
  MODE_BACKWARD,    // 후진
  MODE_TURN_LEFT,   // 좌회전
  MODE_TURN_RIGHT   // 우회전
};

/*******************************************************************************
 * 전역 변수
 ******************************************************************************/
IRrecv ir_receiver(PIN_IR_RECV);
decode_results ir_signal;

int current_speed = SPEED_DEFAULT;
car_mode_t current_mode = MODE_STOP;

// 마지막으로 수신된 유효한 IR 코드 (리피트 코드 처리용)
unsigned long last_valid_code = 0;

/*******************************************************************************
 * 함수 선언
 ******************************************************************************/
// IR 처리 함수
void process_ir_command(unsigned long ir_code);
void print_ir_debug(unsigned long ir_code);

// 모터 제어 함수
void motor_forward(int speed);
void motor_backward(int speed);
void motor_turn_left(int speed);
void motor_turn_right(int speed);
void motor_stop(void);
void set_motor_speed(int right_speed, int left_speed);
void set_motor_direction(int right_dir, int left_dir);

// 속도 제어 함수
void speed_up(void);
void speed_down(void);

// 상태 업데이트 함수
void update_motor_output(void);
void print_status(const char* action);

/*******************************************************************************
 * 초기화
 ******************************************************************************/
void setup() {
  // 시리얼 통신 초기화
  Serial.begin(9600);
  Serial.println("=================================");
  Serial.println("   IR Remote Control Car");
  Serial.println("=================================");
  Serial.println("");
  Serial.println("Controls:");
  Serial.println("  [2] Forward    [+] Speed Up");
  Serial.println("  [4] Left  [5] Stop  [6] Right");
  Serial.println("  [8] Backward   [-] Speed Down");
  Serial.println("");
  
  // 모터 핀 초기화
  pinMode(PIN_MOTOR_R_EN, OUTPUT);
  pinMode(PIN_MOTOR_L_EN, OUTPUT);
  pinMode(PIN_MOTOR_R_IN1, OUTPUT);
  pinMode(PIN_MOTOR_R_IN2, OUTPUT);
  pinMode(PIN_MOTOR_L_IN3, OUTPUT);
  pinMode(PIN_MOTOR_L_IN4, OUTPUT);
  
  // IR 수신 시작
  ir_receiver.enableIRIn();
  
  // 초기 상태: 정지
  motor_stop();
  
  Serial.print("Ready! Speed: ");
  Serial.println(current_speed);
}

/*******************************************************************************
 * 메인 루프
 ******************************************************************************/
void loop() {
  // IR 신호 수신 확인
  if (ir_receiver.decode(&ir_signal)) {
    unsigned long ir_code = ir_signal.value;
    
    // 리피트 코드 처리: 무시
    if (ir_code == IR_REPEAT_CODE) {
      // 리피트 코드는 무시 (모터 상태 유지)
    } else {
      // 새로운 유효 코드
      last_valid_code = ir_code;
      
      // 디버그 출력
      print_ir_debug(ir_code);
      
      // 명령 처리
      process_ir_command(ir_code);
      
      // 모터 출력 업데이트
      update_motor_output();
    }
    
    // 다음 신호 수신 대기
    ir_receiver.resume();
  }
}

/*******************************************************************************
 * IR 명령 처리
 ******************************************************************************/
void process_ir_command(unsigned long ir_code) {
  switch (ir_code) {
    // 속도 제어
    case IR_BTN_PLUS:
      speed_up();
      print_status("Speed Up");
      break;
      
    case IR_BTN_MINUS:
      speed_down();
      print_status("Speed Down");
      break;
    
    // 방향 제어
    case IR_BTN_2:  // 전진
      current_mode = MODE_FORWARD;
      print_status("Forward");
      break;
      
    case IR_BTN_8:  // 후진
      current_mode = MODE_BACKWARD;
      print_status("Backward");
      break;
      
    case IR_BTN_4:  // 좌회전
      current_mode = MODE_TURN_LEFT;
      print_status("Turn Left");
      break;
      
    case IR_BTN_6:  // 우회전
      current_mode = MODE_TURN_RIGHT;
      print_status("Turn Right");
      break;
      
    case IR_BTN_5:  // 정지
      current_mode = MODE_STOP;
      print_status("Stop");
      break;
      
    default:
      // 알 수 없는 버튼
      last_valid_code = 0;
      Serial.print(">> Unknown: 0x");
      Serial.println(ir_code, HEX);
      break;
  }
}

/*******************************************************************************
 * 모터 출력 업데이트
 ******************************************************************************/
void update_motor_output(void) {
  switch (current_mode) {
    case MODE_FORWARD:
      motor_forward(current_speed);
      break;
      
    case MODE_BACKWARD:
      motor_backward(current_speed);
      break;
      
    case MODE_TURN_LEFT:
      motor_turn_left(current_speed);
      break;
      
    case MODE_TURN_RIGHT:
      motor_turn_right(current_speed);
      break;
      
    case MODE_STOP:
    default:
      motor_stop();
      break;
  }
}

/*******************************************************************************
 * 모터 제어 함수들
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

// 좌회전 (왼쪽 바퀴 느리게, 오른쪽 바퀴 빠르게)
void motor_turn_left(int speed) {
  int fast_speed = min((int)(speed * TURN_FAST_RATIO), SPEED_MAX);
  int slow_speed = max((int)(speed * TURN_SLOW_RATIO), SPEED_MIN);
  
  set_motor_direction(HIGH, HIGH);
  set_motor_speed(fast_speed, slow_speed);  // 오른쪽 빠르게, 왼쪽 느리게
}

// 우회전 (오른쪽 바퀴 느리게, 왼쪽 바퀴 빠르게)
void motor_turn_right(int speed) {
  int fast_speed = min((int)(speed * TURN_FAST_RATIO), SPEED_MAX);
  int slow_speed = max((int)(speed * TURN_SLOW_RATIO), SPEED_MIN);
  
  set_motor_direction(HIGH, HIGH);
  set_motor_speed(slow_speed, fast_speed);  // 오른쪽 느리게, 왼쪽 빠르게
}

// 정지
void motor_stop(void) {
  set_motor_speed(0, 0);
}

// 모터 방향 설정
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
 * 속도 제어 함수들
 ******************************************************************************/

// 속도 증가
void speed_up(void) {
  current_speed += SPEED_STEP;
  if (current_speed > SPEED_MAX) {
    current_speed = SPEED_MAX;
  }
}

// 속도 감소
void speed_down(void) {
  current_speed -= SPEED_STEP;
  if (current_speed < SPEED_MIN) {
    current_speed = SPEED_MIN;
  }
}

/*******************************************************************************
 * 디버그/출력 함수들
 ******************************************************************************/

// IR 신호 디버그 출력
void print_ir_debug(unsigned long ir_code) {
  Serial.print("[IR] HEX: 0x");
  Serial.print(ir_code, HEX);
  Serial.print(" / DEC: ");
  Serial.println(ir_code);
}

// 상태 출력
void print_status(const char* action) {
  Serial.print(">> ");
  Serial.print(action);
  Serial.print(" | Mode: ");
  
  switch (current_mode) {
    case MODE_STOP:       Serial.print("STOP");   break;
    case MODE_FORWARD:    Serial.print("FWD");    break;
    case MODE_BACKWARD:   Serial.print("BWD");    break;
    case MODE_TURN_LEFT:  Serial.print("LEFT");   break;
    case MODE_TURN_RIGHT: Serial.print("RIGHT");  break;
  }
  
  Serial.print(" | Speed: ");
  Serial.print(current_speed);
  Serial.print(" (");
  Serial.print((current_speed * 100) / 255);
  Serial.println("%)");
}
