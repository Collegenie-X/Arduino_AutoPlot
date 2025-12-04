  // 장애물 회피 RC카 예제 코드
  
  
  #include <Servo.h>
  
  int MotorF = 9;
  int MotorB = 10;  // 모터 연결 핀(D9, D10)
  
  int trig = 12;
  int echo = 13; // 초음파 센서 핀(Echo - D13, Trig - D12)
  
  int servoPin = 6;  // 서보 모터 PWM 연결 핀(D6)
  int Speed = 250;  // 모터 속도 셋팅
  
  int arc = 92; 
  
  
  Servo servo;
  
  void up(){
  
    analogWrite(MotorF,255); 
    analogWrite(MotorB,LOW); 
    delay(20);
    
  }
  
  
  void down(){
    
    analogWrite(MotorF,LOW); 
    analogWrite(MotorB,255); 
    delay(20);
  
  }
  
  
  
  void Stop(){
    
    analogWrite(MotorF,LOW); 
    analogWrite(MotorB,LOW); 
    delay(100); 
  }
  
  void setup()
  {
      servo.attach(6);
      servo.write(92);
    
      pinMode(trig, OUTPUT);
      pinMode(echo, INPUT);
    
      Serial.begin(9600); // 시리얼 통신 설정
    
    
  }
  
  float distanceCar(){
    digitalWrite(trig, LOW);
    digitalWrite(echo, LOW);
    delayMicroseconds(4);
    digitalWrite(trig, HIGH);
    delayMicroseconds(20);
    digitalWrite(trig, LOW);
  
    unsigned long duration = pulseIn(echo, HIGH);
    float distance = duration / 29.0 / 2.0;
  
    return distance;
    
  }
  
  void loop()
  {
    // 초음파센서 거리 측정
    float distance = distanceCar(); 
    Serial.println(distance);
  
  
    // 장애물과의 거리가 15cm 미만일 경우 장애물을 회피
    if ( 0 < distance && distance <= 10) {  //자동차 거리가 매우 짧을 때. 
      tone(8,1000,50);
      
      servo.write(arc); //center 값 
      down(); 
      delay(100); 
      
    }else if(10 < distance && distance <30) {
  
   
      servo.write(arc); //center 값 
      Stop(); 
      delay(50); 
      up(); 
      delay(100); 
   
      
    }else{
       tone(8,3000,80);
  
      arc = 92; // center 값. 
      Stop();
      delay(50);
       
       servo.write(arc+25); 
       up();
       delay(50); 
       if(1<distanceCar() && distanceCar()< 50){
         arc = arc+20;
       }
  
     Stop();
     delay(50);
       servo.write(arc-25); 
       up(); 
       delay(80); 
       if(1<distanceCar() && distanceCar()< 50) {
        arc = arc-20; 
       }
  
       up();
       delay(100); 
      
    }
  
    delay(10);
  }
  
  
  
  
  
  
  
  
  
  

