# RC CAR Simple - 소스코드 설명서

## 목차
1. [개요](#1-개요)
2. [하드웨어 구성](#2-하드웨어-구성)
3. [소프트웨어 구조](#3-소프트웨어-구조)
4. [순서도 (Flowchart)](#4-순서도-flowchart)
5. [알고리즘 설명](#5-알고리즘-설명)
6. [명령어 목록](#6-명령어-목록)
7. [함수 설명](#7-함수-설명)

---

## 1. 개요

이 프로젝트는 아두이노 기반의 RC카로, 다음 기능을 포함합니다:

| 기능 | 설명 |
|------|------|
| 🎮 **블루투스 원격 제어** | 스마트폰 앱으로 조종 |
| 💻 **시리얼 통신 제어** | PC에서 시리얼 모니터로 조종 |
| 🚧 **장애물 회피** | 초음파 센서로 장애물 감지 및 자동 회피 |
| 🔄 **서보모터 제어** | 카메라 또는 센서 방향 조절 |

---

## 2. 하드웨어 구성

### 2.1 핀 배치표

| 부품 | 핀 번호 | 기능 | 비고 |
|------|---------|------|------|
| **서보모터** | D2 | 신호선 | PWM |
| **블루투스** | D3 | RX (수신) | SoftwareSerial |
| **블루투스** | D4 | TX (송신) | SoftwareSerial |
| **모터드라이버** | D5 | 오른쪽 Enable | PWM 속도제어 |
| **모터드라이버** | D6 | 왼쪽 Enable | PWM 속도제어 |
| **피에조 부저** | D7 | 알림음 | - |
| **모터드라이버** | D8 | 오른쪽 IN1 | 방향제어 |
| **모터드라이버** | D9 | 오른쪽 IN2 | 방향제어 |
| **모터드라이버** | D10 | 왼쪽 IN3 | 방향제어 |
| **모터드라이버** | D11 | 왼쪽 IN4 | 방향제어 |
| **초음파센서** | D12 | ECHO | 입력 |
| **초음파센서** | D13 | TRIG | 출력 |

### 2.2 부품 목록

| 부품 | 수량 | 모델명 | 설명 |
|------|:----:|--------|------|
| 메인보드 | 1 | Arduino UNO | 메인 컨트롤러 |
| 모터드라이버 | 1 | L298N | DC 모터 제어 |
| DC 기어모터 | 2 | - | 좌/우 바퀴 구동 |
| 블루투스 모듈 | 1 | HC-06 | 무선 통신 |
| 초음파센서 | 1 | HC-SR04 | 거리 측정 |
| 서보모터 | 1 | SG90 | 방향 조절 |
| 피에조 부저 | 1 | - | 알림음 |

### 2.3 회로 연결도

```mermaid
graph LR
    subgraph Arduino UNO
        D2[D2]
        D3[D3]
        D4[D4]
        D5[D5 PWM]
        D6[D6 PWM]
        D7[D7]
        D8[D8]
        D9[D9]
        D10[D10]
        D11[D11]
        D12[D12]
        D13[D13]
        VCC[5V]
        GND[GND]
    end

    subgraph 서보모터
        SERVO[SG90]
    end

    subgraph 블루투스
        BT[HC-06]
    end

    subgraph 모터드라이버
        L298N[L298N]
    end

    subgraph 초음파센서
        ULTRA[HC-SR04]
    end

    subgraph 부저
        PIEZO[피에조]
    end

    D2 --> SERVO
    D3 --> BT
    D4 --> BT
    D5 --> L298N
    D6 --> L298N
    D7 --> PIEZO
    D8 --> L298N
    D9 --> L298N
    D10 --> L298N
    D11 --> L298N
    D12 --> ULTRA
    D13 --> ULTRA
```

---

## 3. 소프트웨어 구조

### 3.1 프로그램 구조도

```mermaid
graph TD
    A[rc_car_simple.ino] --> B[라이브러리]
    A --> C[상수 정의]
    A --> D[전역 변수]
    A --> E[함수]
    A --> F[setup]
    A --> G[loop]

    B --> B1[SoftwareSerial.h]
    B --> B2[Servo.h]

    C --> C1[핀 번호]
    C --> C2[설정값]

    D --> D1[모터 속도]
    D --> D2[통신 버퍼]
    D --> D3[모드 플래그]

    E --> E1[SmartCar_Go]
    E --> E2[SmartCar_Back]
    E --> E3[SmartCar_Left]
    E --> E4[SmartCar_Right]
    E --> E5[SmartCar_Stop]
    E --> E6[getDistance]

    G --> G1[블루투스 명령 처리]
    G --> G2[장애물 회피 처리]
```

### 3.2 변수 설명표

| 변수명 | 타입 | 초기값 | 설명 |
|--------|------|--------|------|
| `L_MotorSpeed` | int | 153 | 왼쪽 모터 속도 (0~255) |
| `R_MotorSpeed` | int | 153 | 오른쪽 모터 속도 (0~255) |
| `obstacleAvoidMode` | bool | true | 장애물 회피 모드 ON/OFF |
| `inBTChar` | char | - | 블루투스 수신 문자 (1글자) |
| `inputBTString` | String | "" | 블루투스 명령어 문자열 |
| `inSLChar` | char | - | 시리얼 수신 문자 (1글자) |
| `inputSLString` | String | "" | 시리얼 명령어 문자열 |
| `move_delay` | int | 40 | 모터 동작 지연 시간 (ms) |

---

## 4. 순서도 (Flowchart)

### 4.1 전체 프로그램 흐름

```mermaid
flowchart TD
    START([🚀 시작]) --> SETUP[⚙️ setup 함수 실행]
    
    SETUP --> INIT1[시리얼 통신 초기화<br/>9600 baud]
    INIT1 --> INIT2[핀 모드 설정<br/>INPUT/OUTPUT]
    INIT2 --> INIT3[서보모터 테스트<br/>90° → 0° → 180° → 90°]
    INIT3 --> INIT4[🔊 시작 알림음]
    INIT4 --> READY[✅ RC CAR Ready!]
    
    READY --> LOOP[🔄 loop 함수 실행]
    
    LOOP --> CHECK_BT{📡 블루투스/시리얼<br/>데이터 있음?}
    
    CHECK_BT -->|YES| PROCESS[📝 명령어 처리]
    CHECK_BT -->|NO| CHECK_AVOID
    
    PROCESS --> CHECK_AVOID{🚧 obstacleAvoidMode<br/>== true?}
    
    CHECK_AVOID -->|YES| MEASURE[📏 거리 측정<br/>getDistance]
    CHECK_AVOID -->|NO| LOOP
    
    MEASURE --> CHECK_DIST{거리 < 10cm?}
    
    CHECK_DIST -->|YES| AVOID[⚠️ 장애물 회피 실행]
    CHECK_DIST -->|NO| LOOP
    
    AVOID --> LOOP

    style START fill:#4CAF50,color:#fff
    style READY fill:#2196F3,color:#fff
    style AVOID fill:#f44336,color:#fff
```

### 4.2 setup() 함수 상세 흐름

```mermaid
flowchart TD
    A([setup 시작]) --> B[Serial.begin 9600]
    B --> C[bluetooth.begin 9600]
    C --> D[문자열 변수 초기화<br/>inputBTString = '']
    
    D --> E[모터 핀 설정]
    E --> E1[RightMotor_E_pin → OUTPUT]
    E1 --> E2[RightMotor_1_pin → OUTPUT]
    E2 --> E3[RightMotor_2_pin → OUTPUT]
    E3 --> E4[LeftMotor_3_pin → OUTPUT]
    E4 --> E5[LeftMotor_4_pin → OUTPUT]
    E5 --> E6[LeftMotor_E_pin → OUTPUT]
    
    E6 --> F[센서 핀 설정]
    F --> F1[TRIG → OUTPUT]
    F1 --> F2[ECHO → INPUT]
    F2 --> F3[PIEZO → OUTPUT]
    
    F3 --> G[🔊 시작 알림음<br/>tone 1000Hz, 100ms]
    
    G --> H[서보모터 테스트]
    H --> H1[attach SERVO_PIN]
    H1 --> H2[write 90° - 중앙]
    H2 --> H3[write 0° - 최소]
    H3 --> H4[write 180° - 최대]
    H4 --> H5[write 90° - 중앙]
    H5 --> H6[detach]
    
    H6 --> I[🔊 완료 알림음]
    I --> J[Serial.println<br/>'RC CAR Ready!']
    J --> K([setup 종료])

    style A fill:#4CAF50,color:#fff
    style K fill:#2196F3,color:#fff
```

### 4.3 명령어 처리 흐름

```mermaid
flowchart LR
    A([명령어 처리 시작]) --> B{bluetooth.available<br/>OR Serial.available?}
    
    B -->|YES| C[문자 읽기<br/>inBTChar = bluetooth.read<br/>inSLChar = Serial.read]
    B -->|NO| Z([처리 종료])
    
    C --> D{종료문자인가?<br/>'\n' '\r' '_'}
    
    D -->|NO| E[문자열에 추가<br/>inputBTString += inBTChar]
    E --> F[delay 30ms]
    F --> B
    
    D -->|YES| G{명령어 판별}
    
    G -->|STOP| H1[🛑 SmartCar_Stop]
    G -->|UP| H2[⬆️ SmartCar_Go]
    G -->|DOWN| H3[⬇️ SmartCar_Back]
    G -->|LEFT| H4[⬅️ SmartCar_Left]
    G -->|RIGHT| H5[➡️ SmartCar_Right]
    G -->|SERVOUP| H6[🔼 Servo 180°]
    G -->|SERVODOWN| H7[🔽 Servo 0°]
    G -->|SERVOMID| H8[⏺️ Servo 90°]
    G -->|SPEED###| H9[⚡ 속도 변경]
    G -->|AVOIDON| H10[✅ 회피모드 ON]
    G -->|AVOIDOFF| H11[❌ 회피모드 OFF]
    G -->|STATUS| H12[📊 상태 출력]
    G -->|기타| H13[❓ 알 수 없는 명령]
    
    H1 & H2 & H3 & H4 & H5 & H6 & H7 & H8 & H9 & H10 & H11 & H12 & H13 --> I[bluetooth.print 'OK']
    I --> J[문자열 초기화<br/>inputBTString = '']
    J --> B

    style A fill:#4CAF50,color:#fff
    style Z fill:#2196F3,color:#fff
    style H1 fill:#f44336,color:#fff
```

### 4.4 장애물 회피 알고리즘

```mermaid
flowchart TD
    A([장애물 회피 시작]) --> B{obstacleAvoidMode<br/>== true?}
    
    B -->|NO| Z([종료 - 회피 안함])
    B -->|YES| C[📏 거리 측정<br/>dist = getDistance]
    
    C --> D{dist > 0 AND<br/>dist < 10cm?}
    
    D -->|NO| Z2([종료 - 안전거리])
    D -->|YES| E[🔊 경고음<br/>tone 1000Hz]
    
    E --> F[📢 Serial.println<br/>'Obstacle detected!']
    F --> G[🛑 SmartCar_Stop<br/>즉시 정지]
    
    G --> H[⬇️ SmartCar_Back<br/>후진]
    H --> I[⏱️ delay 500ms]
    
    I --> J[🎲 랜덤값 생성<br/>toggle = random 1,3]
    
    J --> K{toggle == 2?}
    
    K -->|YES| L[⬅️ SmartCar_Left<br/>좌회전]
    K -->|NO| M[➡️ SmartCar_Right<br/>우회전]
    
    L --> N[⏱️ delay 500ms]
    M --> N
    
    N --> O[🛑 SmartCar_Stop<br/>정지]
    O --> P([회피 완료])

    style A fill:#4CAF50,color:#fff
    style Z fill:#9E9E9E,color:#fff
    style Z2 fill:#4CAF50,color:#fff
    style P fill:#2196F3,color:#fff
    style E fill:#FF9800,color:#fff
    style G fill:#f44336,color:#fff
```

### 4.5 거리 측정 함수 (getDistance)

```mermaid
flowchart TD
    A([getDistance 시작]) --> B[TRIG → HIGH]
    B --> C[⏱️ delayMicroseconds 10μs]
    C --> D[TRIG → LOW]
    D --> E[duration = pulseIn ECHO, HIGH]
    E --> F[거리 계산<br/>distance = 340 × duration / 10000 / 2]
    F --> G[📢 Serial.print distance]
    G --> H[return distance]
    H --> I([종료])

    style A fill:#4CAF50,color:#fff
    style I fill:#2196F3,color:#fff
```

---

## 5. 알고리즘 설명

### 5.1 모터 제어 알고리즘

#### H-Bridge 방향 제어표

| 동작 | IN1 | IN2 | Enable | 결과 |
|:----:|:---:|:---:|:------:|------|
| ▶️ 정방향 | HIGH | LOW | PWM | 모터 정회전 |
| ◀️ 역방향 | LOW | HIGH | PWM | 모터 역회전 |
| ⏹️ 정지 | LOW | LOW | 0 | 모터 정지 |
| 🛑 브레이크 | HIGH | HIGH | - | 급정지 |

#### PWM 속도 제어표

| PWM 값 | 백분율 | 설명 |
|:------:|:------:|------|
| 0 | 0% | 정지 |
| 64 | 25% | 저속 |
| 127 | 50% | 중속 |
| **153** | **60%** | **기본값** |
| 191 | 75% | 고속 |
| 255 | 100% | 최대속도 |

#### 이동 방향별 모터 동작표

| 동작 | 오른쪽 모터 | | 왼쪽 모터 | |
|:----:|:-----------:|:-:|:---------:|:-:|
| | **방향** | **속도** | **방향** | **속도** |
| ⬆️ 전진 | 정방향 | PWM | 정방향 | PWM |
| ⬇️ 후진 | 역방향 | PWM | 역방향 | PWM |
| ⬅️ 좌회전 | 정방향 | PWM | 정지 | 0 |
| ➡️ 우회전 | 정지 | 0 | 정방향 | PWM |
| ⏹️ 정지 | 정지 | 0 | 정지 | 0 |

### 5.2 초음파 센서 알고리즘

#### 거리 측정 원리

```mermaid
sequenceDiagram
    participant A as Arduino
    participant T as TRIG
    participant U as 초음파
    participant O as 물체
    participant E as ECHO

    A->>T: HIGH 신호 (10μs)
    T->>U: 초음파 발사 (40kHz)
    U->>O: 초음파 이동
    O->>U: 반사
    U->>E: 수신
    E->>A: HIGH 유지 시간 반환
    Note over A: 거리 = 시간 × 음속 / 2
```

#### 거리 계산 공식표

| 항목 | 값 | 설명 |
|------|-----|------|
| 음속 | 340 m/s | 상온 기준 |
| 음속 | 0.034 cm/μs | μs당 이동거리 |
| 공식 | `거리 = duration × 340 / 10000 / 2` | cm 단위 |
| /2 이유 | 왕복 거리 | 발사→반사→수신 |

### 5.3 블루투스 통신 알고리즘

#### 명령어 프로토콜

| 구성요소 | 형식 | 예시 |
|----------|------|------|
| 명령어 | 대문자 영문 | UP, DOWN, LEFT |
| 종료문자 | `_` 또는 `\n` | - |
| 전체형식 | `[명령어]_` | `UP_`, `SPEED200_` |

#### 문자열 파싱 과정

```mermaid
sequenceDiagram
    participant App as 📱 앱
    participant BT as 🔵 블루투스
    participant ARD as 🔌 아두이노

    App->>BT: "U" 전송
    BT->>ARD: 'U' 수신
    Note over ARD: inputBTString = "U"
    
    App->>BT: "P" 전송
    BT->>ARD: 'P' 수신
    Note over ARD: inputBTString = "UP"
    
    App->>BT: "_" 전송
    BT->>ARD: '_' 수신
    Note over ARD: 종료문자 감지!
    
    ARD->>ARD: "UP" 명령 실행
    Note over ARD: SmartCar_Go() 호출
    
    ARD->>BT: "OK" 응답
    BT->>App: "OK" 전달
    
    Note over ARD: inputBTString = "" 초기화
```

### 5.4 장애물 회피 알고리즘

#### 회피 전략표

| 단계 | 동작 | 시간 | 목적 |
|:----:|------|:----:|------|
| 1️⃣ | 🛑 즉시 정지 | - | 충돌 방지 |
| 2️⃣ | ⬇️ 후진 | 500ms | 안전거리 확보 |
| 3️⃣ | 🎲 랜덤 방향 선택 | - | 탈출 경로 다양화 |
| 4️⃣ | ↩️ 회전 | 500ms | 방향 전환 |
| 5️⃣ | 🛑 정지 | - | 재판단 대기 |

#### 랜덤 회전 확률표

| random(1,3) 결과 | 확률 | 동작 |
|:----------------:|:----:|------|
| 1 | 50% | ➡️ 우회전 |
| 2 | 50% | ⬅️ 좌회전 |

---

## 6. 명령어 목록

### 6.1 이동 명령어

| 명령어 | 동작 | 함수 | 응답 |
|:------:|------|------|:----:|
| `UP_` | ⬆️ 전진 | SmartCar_Go() | OK |
| `DOWN_` | ⬇️ 후진 | SmartCar_Back() | OK |
| `LEFT_` | ⬅️ 좌회전 | SmartCar_Left() | OK |
| `RIGHT_` | ➡️ 우회전 | SmartCar_Right() | OK |
| `STOP_` | 🛑 정지 | SmartCar_Stop() | OK |

### 6.2 서보모터 명령어

| 명령어 | 동작 | 각도 | 응답 |
|:------:|------|:----:|:----:|
| `SERVOUP_` | 🔼 위로 | 180° | OK |
| `SERVOMID_` | ⏺️ 중앙 | 90° | OK |
| `SERVODOWN_` | 🔽 아래로 | 0° | OK |

### 6.3 설정 명령어

| 명령어 | 동작 | 범위/값 | 응답 |
|:------:|------|---------|:----:|
| `SPEED###_` | ⚡ 속도 설정 | 0~255 | OK |
| `AVOIDON_` | ✅ 회피모드 ON | - | OK |
| `AVOIDOFF_` | ❌ 회피모드 OFF | - | OK |
| `STATUS_` | 📊 상태 확인 | - | AVOID:ON/OFF |

### 6.4 알림음 목록

| 상황 | 주파수 | 지속시간 | 패턴 |
|------|:------:|:--------:|------|
| 🚀 시작 | 1000Hz | 100ms | 단음 |
| 🛑 정지 | 1000Hz | 200ms | 단음 |
| ⚠️ 장애물 감지 | 1000Hz | 100ms | 단음 |
| ✅ 회피모드 ON | 1500Hz | 100ms | 2회 반복 |
| ❌ 회피모드 OFF | 500Hz | 200ms | 단음 (낮은음) |
| ❓ 알 수 없는 명령 | 500Hz | 100ms | 단음 (낮은음) |

---

## 7. 함수 설명

### 7.1 함수 목록 요약표

| 함수명 | 반환타입 | 매개변수 | 설명 |
|--------|:--------:|:--------:|------|
| `setup()` | void | - | 초기화 (1회 실행) |
| `loop()` | void | - | 메인 루프 (무한반복) |
| `SmartCar_Go()` | void | - | 전진 |
| `SmartCar_Back()` | void | - | 후진 |
| `SmartCar_Left()` | void | - | 좌회전 |
| `SmartCar_Right()` | void | - | 우회전 |
| `SmartCar_Stop()` | void | - | 정지 |
| `getDistance()` | long | - | 거리 측정 (cm) |

### 7.2 모터 제어 함수 상세

#### SmartCar_Go() - 전진

```cpp
void SmartCar_Go() {
  Serial.println("Forward");
  
  // 오른쪽 모터: 정방향
  digitalWrite(RightMotor_1_pin, HIGH);
  digitalWrite(RightMotor_2_pin, LOW);
  
  // 왼쪽 모터: 정방향
  digitalWrite(LeftMotor_3_pin, HIGH);
  digitalWrite(LeftMotor_4_pin, LOW);
  
  // PWM 속도 설정
  analogWrite(RightMotor_E_pin, R_MotorSpeed);
  analogWrite(LeftMotor_E_pin, L_MotorSpeed);
  
  delay(move_delay);
}
```

#### 모터 함수 핀 출력 비교표

| 함수 | IN1 | IN2 | IN3 | IN4 | ENA | ENB |
|------|:---:|:---:|:---:|:---:|:---:|:---:|
| SmartCar_Go | H | L | H | L | PWM | PWM |
| SmartCar_Back | L | H | L | H | PWM | PWM |
| SmartCar_Left | H | L | L | L | PWM | 0 |
| SmartCar_Right | L | L | H | L | 0 | PWM |
| SmartCar_Stop | L | L | L | L | 0 | 0 |

> **H** = HIGH, **L** = LOW, **PWM** = 속도값 (153)

### 7.3 getDistance() - 거리 측정

```cpp
long getDistance() {
  long duration, distance;
  
  // 1. 초음파 발사
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  
  // 2. 에코 시간 측정
  duration = pulseIn(ECHO, HIGH);
  
  // 3. 거리 계산 (cm)
  distance = ((float)(340 * duration) / 10000) / 2;
  
  // 4. 시리얼 출력
  Serial.print("distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  return distance;
}
```

---

## 8. 부록

### 8.1 트러블슈팅 가이드

| 증상 | 원인 | 해결방법 |
|------|------|----------|
| 모터가 안 돌아감 | 전원 부족 | 외부 전원 연결 확인 |
| 블루투스 연결 안됨 | TX/RX 반대 | 3번↔4번 핀 교체 |
| 거리 측정 이상 | 센서 방향 | TRIG/ECHO 핀 확인 |
| 한쪽으로 치우침 | 속도 불균형 | L/R_MotorSpeed 조정 |
| 명령 인식 안됨 | 종료문자 누락 | 명령어 끝에 `_` 추가 |

### 8.2 확장 가능 기능

| 기능 | 필요 부품 | 난이도 |
|------|----------|:------:|
| 라인트레이서 | IR 센서 2개 | ⭐⭐ |
| 속도 표시 | LCD 1602 | ⭐⭐ |
| 조명 제어 | LED 모듈 | ⭐ |
| 경적 | 능동 부저 | ⭐ |
| 카메라 | ESP32-CAM | ⭐⭐⭐ |

---

## 문서 정보

| 항목 | 내용 |
|------|------|
| **버전** | 1.1 |
| **작성일** | 2025년 12월 |
| **대상 파일** | `rc_car_simple.ino` |
| **호환 보드** | Arduino UNO, Nano |
| **IDE 버전** | Arduino IDE 1.8.x / 2.x |
