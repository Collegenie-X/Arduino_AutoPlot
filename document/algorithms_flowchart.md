# 📊 자율주행 스마트카 알고리즘 상세 순서도

> **목적**: 실행자와 기획자 모두 개발 프로세스를 이해하고 테스트할 수 있도록 알고리즘을 시각화

---

## 1. 시스템 전체 아키텍처

```mermaid
flowchart TB
    subgraph 입력장치
        BT[블루투스 HC-06]
        US[초음파 센서]
        IR[IR 라인센서 x3]
    end
    
    subgraph 제어장치
        MCU[Arduino UNO]
    end
    
    subgraph 출력장치
        M1[DC모터: 오른쪽]
        M2[DC모터: 왼쪽]
        SV[서보모터]
        BZ[피에조 부저]
    end
    
    subgraph 전원부
        BAT[배터리 팩]
    end
    
    BT -->|시리얼 통신| MCU
    US -->|거리 데이터| MCU
    IR -->|아날로그 값| MCU
    
    MCU -->|PWM 신호| M1
    MCU -->|PWM 신호| M2
    MCU -->|각도 제어| SV
    MCU -->|톤 출력| BZ
    
    BAT -->|5V/12V| MCU
    BAT -->|12V| M1
    BAT -->|12V| M2
```

---

## 2. 블루투스 명령 처리 알고리즘

### 2.1 문자열 파싱 순서도

```mermaid
flowchart TD
    A[시작: loop 함수] --> B{BTSerial.available?}
    B -->|No| A
    B -->|Yes| C[문자 1개 읽기: inBTChar]
    C --> D{구분자인가?<br>'_' or '\n' or '\r'}
    
    D -->|No| E[문자열 누적<br>inputBTString += inBTChar]
    E --> F[30ms 대기]
    F --> B
    
    D -->|Yes| G[명령어 해석]
    G --> H{inputBTString == ?}
    
    H -->|"UP"| I[Forward 함수 호출]
    H -->|"DOWN"| J[Backward 함수 호출]
    H -->|"LEFT"| K[Left 함수 호출]
    H -->|"RIGHT"| L[Right 함수 호출]
    H -->|"STOP"| M[Stop 함수 호출]
    H -->|"LINETRACK"| N[모드 전환: LINE_MODE]
    H -->|"AUTO"| O[모드 전환: AUTO_MODE]
    H -->|기타| P[무시: 버퍼 초기화]
    
    I --> Q[응답: BTSerial.print OK]
    J --> Q
    K --> Q
    L --> Q
    M --> Q
    N --> Q
    O --> Q
    P --> R[inputBTString = 빈문자열]
    Q --> R
    R --> B
```

### 2.2 통신 프로토콜 명세

```mermaid
flowchart LR
    subgraph 앱_to_아두이노
        A1["UP_"] --> B1[전진]
        A2["DOWN_"] --> B2[후진]
        A3["LEFT_"] --> B3[좌회전]
        A4["RIGHT_"] --> B4[우회전]
        A5["STOP_"] --> B5[정지]
        A6["SERVOUP_"] --> B6[서보 180도]
        A7["SERVODOWN_"] --> B7[서보 0도]
        A8["LINETRACK_"] --> B8[라인모드]
        A9["AUTO_"] --> B9[자율주행]
    end
    
    subgraph 아두이노_to_앱
        C1[OK] --> D1[명령 성공]
        C2[ERROR] --> D2[명령 실패]
        C3[거리값] --> D3[센서 데이터]
    end
```

---

## 3. 모터 제어 알고리즘

### 3.1 DC 모터 제어 원리

```mermaid
flowchart TB
    subgraph 모터_드라이버_L298N
        IN1[IN1: 오른쪽+]
        IN2[IN2: 오른쪽-]
        IN3[IN3: 왼쪽+]
        IN4[IN4: 왼쪽-]
    end
    
    subgraph 전진
        F1[IN1=HIGH, IN2=LOW]
        F2[IN3=HIGH, IN4=LOW]
    end
    
    subgraph 후진
        B1[IN1=LOW, IN2=HIGH]
        B2[IN3=LOW, IN4=HIGH]
    end
    
    subgraph 좌회전
        L1[IN1=HIGH, IN2=LOW]
        L2[IN3=LOW, IN4=LOW]
    end
    
    subgraph 우회전
        R1[IN1=LOW, IN2=LOW]
        R2[IN3=HIGH, IN4=LOW]
    end
    
    IN1 -.-> F1
    IN2 -.-> F1
    IN3 -.-> F2
    IN4 -.-> F2
```

### 3.2 PWM 속도 제어

```mermaid
flowchart LR
    A[속도값 0~255] --> B{값 범위}
    B -->|0| C[정지]
    B -->|1~100| D[저속]
    B -->|101~180| E[중속]
    B -->|181~255| F[고속]
    
    C --> G[모터 동작]
    D --> G
    E --> G
    F --> G
```

---

## 4. 초음파 센서 알고리즘

### 4.1 거리 측정 순서도

```mermaid
flowchart TD
    A[시작: measureDistance] --> B[TRIG = LOW]
    B --> C[2μs 대기]
    C --> D[TRIG = HIGH]
    D --> E[10μs 대기]
    E --> F[TRIG = LOW]
    F --> G[pulseIn: ECHO HIGH 시간 측정]
    G --> H[거리 = 시간 × 0.034 / 2]
    H --> I{거리 유효?}
    I -->|Yes| J[거리값 반환]
    I -->|No: 0 또는 음수| K[0 반환: 측정 실패]
    J --> L[종료]
    K --> L
```

### 4.2 거리 기반 의사결정

```mermaid
flowchart TD
    A[거리 측정] --> B{distance < 10cm?}
    B -->|Yes| C[🔴 위험: 즉시 정지 + 경고음]
    B -->|No| D{distance < 20cm?}
    D -->|Yes| E[🟡 주의: 감속 + 스캔 준비]
    D -->|No| F{distance < 40cm?}
    F -->|Yes| G[🟢 안전: 정상 주행]
    F -->|No| H[✅ 여유: 가속 가능]
    
    C --> I[회피 동작]
    E --> J[서보모터 스캔]
    G --> K[현재 속도 유지]
    H --> L[속도 증가]
```

---

## 5. 서보모터 스캔 알고리즘

### 5.1 3방향 스캔

```mermaid
flowchart TD
    A[스캔 시작] --> B[서보 45도: 오른쪽]
    B --> C[300ms 대기]
    C --> D[오른쪽 거리 측정]
    D --> E[서보 90도: 정면]
    E --> F[300ms 대기]
    F --> G[정면 거리 측정]
    G --> H[서보 135도: 왼쪽]
    H --> I[300ms 대기]
    I --> J[왼쪽 거리 측정]
    J --> K[서보 90도: 정면 복귀]
    K --> L{최대 거리 방향?}
    L -->|오른쪽| M[RIGHT 반환]
    L -->|정면| N[FORWARD 반환]
    L -->|왼쪽| O[LEFT 반환]
```

### 5.2 5방향 정밀 스캔 (확장)

```mermaid
flowchart LR
    subgraph 스캔_각도
        A1[30°] --> A2[60°]
        A2 --> A3[90°]
        A3 --> A4[120°]
        A4 --> A5[150°]
    end
    
    subgraph 거리_배열
        D1[dist_0]
        D2[dist_1]
        D3[dist_2]
        D4[dist_3]
        D5[dist_4]
    end
    
    A1 --> D1
    A2 --> D2
    A3 --> D3
    A4 --> D4
    A5 --> D5
```

---

## 6. 장애물 회피 알고리즘

### 6.1 기본 회피 알고리즘

```mermaid
flowchart TD
    A[loop 시작] --> B[전진]
    B --> C[거리 측정]
    C --> D{거리 < 20cm?}
    D -->|No| B
    D -->|Yes| E[정지]
    E --> F[경고음]
    F --> G[후진 500ms]
    G --> H[서보 스캔]
    H --> I{왼쪽 > 오른쪽?}
    I -->|Yes| J[좌회전 500ms]
    I -->|No| K[우회전 500ms]
    J --> L[정지]
    K --> L
    L --> B
```

### 6.2 고급 회피 알고리즘 (막다른 길 처리)

```mermaid
flowchart TD
    A[장애물 감지] --> B[3방향 스캔]
    B --> C{모든 방향 < 15cm?}
    C -->|Yes| D[180도 회전: 되돌아가기]
    C -->|No| E{정면 > 30cm?}
    E -->|Yes| F[직진 선택]
    E -->|No| G{왼쪽 vs 오른쪽}
    G -->|왼쪽 큼| H[좌회전]
    G -->|오른쪽 큼| I[우회전]
    G -->|같음| J[랜덤 선택]
    
    D --> K[회피 완료]
    F --> K
    H --> K
    I --> K
    J --> K
```

---

## 7. 라인 트레이싱 알고리즘

### 7.1 3센서 기본 알고리즘

```mermaid
flowchart TD
    A[센서 읽기] --> B[LEFT = analogRead A0]
    B --> C[CENTER = analogRead A1]
    C --> D[RIGHT = analogRead A2]
    D --> E[임계값 비교: THRESHOLD=850]
    
    E --> F{L>TH & C>TH & R>TH?}
    F -->|Yes| G[정지: 종료 지점]
    F -->|No| H{L<TH & C<TH & R<TH?}
    H -->|Yes| I[정지: 라인 이탈]
    H -->|No| J{C > TH?}
    J -->|Yes| K[전진]
    J -->|No| L{L > TH?}
    L -->|Yes| M[좌회전]
    L -->|No| N{R > TH?}
    N -->|Yes| O[우회전]
    N -->|No| P[정지: 예외]
    
    K --> A
    M --> A
    O --> A
```

### 7.2 센서 조합별 동작 매트릭스

```mermaid
flowchart TB
    subgraph 센서상태_to_동작
        S1["○●○<br>(중앙만)"] --> A1[전진]
        S2["●○○<br>(왼쪽만)"] --> A2[좌회전]
        S3["○○●<br>(오른쪽만)"] --> A3[우회전]
        S4["●●○<br>(왼쪽+중앙)"] --> A4[약한 좌회전]
        S5["○●●<br>(중앙+오른쪽)"] --> A5[약한 우회전]
        S6["●●●<br>(전체 감지)"] --> A6[정지]
        S7["○○○<br>(전체 미감지)"] --> A7[복구 시도]
    end
```

### 7.3 라인 이탈 복구 알고리즘

```mermaid
flowchart TD
    A[라인 이탈 감지<br>모든 센서 미감지] --> B[마지막 방향 기억]
    B --> C{마지막 = 좌회전?}
    C -->|Yes| D[좌회전 계속]
    C -->|No| E{마지막 = 우회전?}
    E -->|Yes| F[우회전 계속]
    E -->|No| G[후진 후 스캔]
    
    D --> H{센서 감지?}
    F --> H
    G --> H
    H -->|Yes| I[라인 복구 성공]
    H -->|No| J[시도 횟수++]
    J --> K{시도 > 5?}
    K -->|Yes| L[정지: 복구 실패]
    K -->|No| D
    
    I --> M[정상 주행 재개]
```

---

## 8. 자동차 따라가기 알고리즘

### 8.1 목표 거리 유지

```mermaid
flowchart TD
    A[시작] --> B[거리 측정]
    B --> C{거리 = TARGET ± 5cm?}
    C -->|Yes| D[속도 유지]
    C -->|No| E{거리 < TARGET?}
    E -->|Yes| F[감속 또는 후진]
    E -->|No| G{거리 > TARGET?}
    G -->|Yes| H[가속]
    G -->|No| I[정지]
    
    D --> B
    F --> B
    H --> B
    I --> J[대상 사라짐]
    J --> K[탐색 모드]
    K --> B
```

### 8.2 PID 제어 개념 (심화)

```mermaid
flowchart LR
    A[목표 거리] --> B((오차 계산))
    C[현재 거리] --> B
    B --> D[P: 비례 제어]
    B --> E[I: 적분 제어]
    B --> F[D: 미분 제어]
    D --> G((합산))
    E --> G
    F --> G
    G --> H[모터 속도 출력]
    H --> I[실제 거리 변화]
    I --> C
```

---

## 9. 통합 상태 관리

### 9.1 상태 머신 다이어그램

```mermaid
stateDiagram-v2
    [*] --> BOOT: 전원 ON
    
    BOOT --> SELF_TEST: 초기화 완료
    SELF_TEST --> IDLE: 테스트 통과
    SELF_TEST --> ERROR: 테스트 실패
    
    IDLE --> RC_MODE: BT:RCCAR_
    IDLE --> LINE_MODE: BT:LINETRACK_
    IDLE --> AUTO_MODE: BT:AUTO_
    IDLE --> FOLLOW_MODE: BT:FOLLOW_
    
    RC_MODE --> IDLE: BT:STOP_
    LINE_MODE --> IDLE: BT:STOP_ / 종료 라인
    AUTO_MODE --> IDLE: BT:STOP_
    FOLLOW_MODE --> IDLE: BT:STOP_
    
    RC_MODE --> EMERGENCY: 장애물 <5cm
    LINE_MODE --> EMERGENCY: 완전 이탈
    AUTO_MODE --> EMERGENCY: 막다른 길
    
    EMERGENCY --> IDLE: 해제
    
    ERROR --> [*]: 전원 OFF
    
    state RC_MODE {
        [*] --> 대기중
        대기중 --> 동작중: 명령 수신
        동작중 --> 대기중: 동작 완료
    }
    
    state LINE_MODE {
        [*] --> 라인추적
        라인추적 --> 복구중: 이탈
        복구중 --> 라인추적: 복구 성공
    }
    
    state AUTO_MODE {
        [*] --> 직진
        직진 --> 스캔중: 장애물
        스캔중 --> 회피중: 방향 결정
        회피중 --> 직진: 회피 완료
    }
```

---

## 10. 테스트 케이스 설계

### 10.1 기능별 테스트 매트릭스

```mermaid
flowchart TB
    subgraph 테스트_항목
        T1[TC-01: 블루투스 연결]
        T2[TC-02: 전진 명령]
        T3[TC-03: 후진 명령]
        T4[TC-04: 좌회전 명령]
        T5[TC-05: 우회전 명령]
        T6[TC-06: 정지 명령]
        T7[TC-07: 초음파 거리 측정]
        T8[TC-08: 서보 각도 제어]
        T9[TC-09: 장애물 회피]
        T10[TC-10: 라인 트레이싱]
    end
    
    subgraph 예상결과
        R1[페어링 성공]
        R2[앞으로 이동]
        R3[뒤로 이동]
        R4[왼쪽 회전]
        R5[오른쪽 회전]
        R6[즉시 정지]
        R7[거리값 출력]
        R8[지정 각도 이동]
        R9[장애물 우회]
        R10[라인 따라 이동]
    end
    
    T1 --> R1
    T2 --> R2
    T3 --> R3
    T4 --> R4
    T5 --> R5
    T6 --> R6
    T7 --> R7
    T8 --> R8
    T9 --> R9
    T10 --> R10
```

### 10.2 에러 시나리오 테스트

```mermaid
flowchart TD
    A[에러 시나리오] --> B[블루투스 연결 끊김]
    A --> C[배터리 부족]
    A --> D[센서 오작동]
    A --> E[모터 응답 없음]
    
    B --> B1[자동 정지 확인]
    C --> C1[저전압 경고음 확인]
    D --> D1[기본값 사용 확인]
    E --> E1[타임아웃 처리 확인]
    
    B1 --> F[테스트 결과 기록]
    C1 --> F
    D1 --> F
    E1 --> F
```

---

## 📋 알고리즘 체크리스트

| 알고리즘 | PRIMM 단계 | 완료 |
|----------|-----------|:----:|
| 블루투스 통신 | P-R-I-M-M | ☐ |
| DC 모터 제어 | P-R-I-M-M | ☐ |
| 초음파 거리 측정 | P-R-I-M-M | ☐ |
| 서보모터 제어 | P-R-I-M-M | ☐ |
| 장애물 회피 | P-R-I-M-M | ☐ |
| 라인 트레이싱 | P-R-I-M-M | ☐ |
| 통합 상태 관리 | P-R-I-M-M | ☐ |

---

*이 문서의 모든 Mermaid 다이어그램은 학습자가 알고리즘을 시각적으로 이해하고, 테스트 케이스를 직접 설계할 수 있도록 구성되었습니다.*


