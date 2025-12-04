    #include <SoftwareSerial.h>
  
    /* 피에조 스피커 */
    #define PIEZO 8  //PIEZO
    
    #define RX 10
    #define TX 12
    
    #define RIGHT_1  5  //motor ONE  PWM 
    #define RIGHT_2  6
    
    #define LEFT_1  11    //motor TWO
    #define LEFT_2  9
    
    
    #define LEFT     A2
    #define CENTER   A1
    #define RIGHT    A0
    
    #define SPEED 80       //150 (LOW) ,80 (HIGH)
    #define LEFT_SPEED 100  // 220 (LOW),95 (HIGH)
    #define LINE_SENSOR  850    
    
    
    
    int rccar_onoff = 0;
    int long_speed  = 5;
      
    char inBTChar;      //blueTooth 통신에서 한 글자씩 받기위한 char형 변수로 선언
    String inputBTString ;  //up,down,left,right... 등등 블루투스에 보낸  명령어 받기 위한 문자열 저장 공간
    String inputSLString ;
    
    SoftwareSerial BTSerial(TX, RX);
        
        void setup()
        {
          Serial.begin(9600);
           
           BTSerial.begin(9600);  //블루투스 통신 속도 9600byte 1초당
           inputBTString = ""; 
      
           
          pinMode(RIGHT_1, OUTPUT);
          pinMode(RIGHT_2, OUTPUT);
          
          pinMode(LEFT_1, OUTPUT);
          pinMode(LEFT_2, OUTPUT);
         
        }


        
        void loop() {
        
         while (BTSerial.available()) {   //블루투스 통신이 연결되었을 경우에만 무한 루프로 실행한다.  무한이 일을 하면서 자료들을 처리하겠다는 의도다.
          inBTChar = (char)BTSerial.read();
        
        
        if (inBTChar == '\n' || inBTChar == '\r' || inBTChar == '_') {
          if (inputBTString.equals("rccar_")) {
            rccar_onoff = 0;
   
          } else if (inputBTString.equals("linetrack_")) {
    
            rccar_onoff = 1;
          }


          
          if ( rccar_onoff = 0) {
            if (inputBTString.equals("stop")) { //앱에서 "stop" 메시지를 보냈을 경우에
            //inputBTString 문자열값과 비교하여 같을 경우에 모터 동작을 멈춘다.

            Stop();                              // 모터 동작 멈춤
            BTSerial.print("OK");                //실행이 성공적이었다면 앱에 "OK"확인 메시지 보냄.
    
          } else if (inputBTString.equals("left")) { //앱에서 "left" 메시지를 보냈을 경우에
            //inputBTString 문자열값과 비교하여 같을 경우에
            // 오른쪽 모터만 정방향으로 동작시킨다.
    
            Leftward();           //오른쪽 모터만 동작하는 함수 호출한다. 그래서 오른쪽 모터만 움직인다.
            BTSerial.print("OK");           //실행이 성공적이었다면 앱에 "OK"확인 메시지 보냄.
    
          } else if (inputBTString.equals("leftup")) { //앱에서 "leftup" 메시지를 보냈을 경우에
            //inputBTString 문자열값과 비교하여 같을 경우에
    
    
    
            Stop();                          //모터 동작 멈춤
            BTSerial.print("OK");            //실행이 성공적이었다면 앱에 "OK"확인 메시지 보냄.
    
    
          } else if (inputBTString.equals("right")) { //앱에서 "right" 메시지를 보냈을 경우에
            //inputBTString 문자열값과 비교하여 같을 경우에
    
    
            Rightward();                          //왼쪽 모터만 동작하는 함수 호출한다.왼쪽 모터만 움직인다.
            BTSerial.print("OK");            //실행이 성공적이었다면 앱에 "OK"확인 메시지 보냄.
    
          } else if (inputBTString.equals("rightup")) { //앱에서 "rightup" 메시지를 보냈을 경우에
            //inputBTString 문자열값과 비교하여 같을 경우에
    
    
            Stop();          //모터 동작 멈춤
            BTSerial.print("OK");       //실행이 성공적이었다면 앱에 "OK"확인 메시지 보냄.
          }
    
          else if (inputBTString.equals("up")) {   //앱에서 "up" 메시지를 보냈을 경우에
            //inputBTString 문자열값과 비교하여 같을 경우에
            // 정면 방향,모터로 동작
    
            Forward();//up 함수 호출
            BTSerial.print("OK");   //실행이 성공적이었다면 앱에 "OK"확인 메시지 보냄.
    
    
          } else if (inputBTString.equals("down")) { //앱에서 "down" 메시지를 보냈을 경우에
            //inputBTString 문자열값과 비교하여 같을 경우에
            // 후진 방향, 모터로 동작
    
            Backward(); //down 함후 호출
            BTSerial.print("OK");  //실행이 성공적이었다면 앱에 "OK"확인 메시지 보냄.
    
          } else {
            inputBTString = "";   //블루루스 송수신 문자열이 up/down/left/right/leftup/rightup 아닐경우에
            //블루투스 송수진 문자열 빈 공간으로 초기화
          }
          }
        
        else  inputBTString += inBTChar;   // 블루투스 송수신 문자열에 앱에서 보낸 메시지 문자 하나씩 저장
        delay(30);
        }
        
      
      if ( rccar_onoff = 1 ){
        if(analogRead(RIGHT) < LINE_SENSOR  && analogRead(CENTER) < LINE_SENSOR && analogRead(LEFT) < LINE_SENSOR){
          // 3개의 센서가 모두 인식이 됬을 경우는 멈춰있습니다.
           //Backward();
               Stop();   
          }
          else if(analogRead(RIGHT) > LINE_SENSOR  && analogRead(CENTER) > LINE_SENSOR && analogRead(LEFT) > LINE_SENSOR){
          // 3개의 센서가 모두 인식이 안됬을 경우는 멈춰있습니다.
           //Backward();
               Stop();   
          }
          else if(analogRead(LEFT) > LINE_SENSOR && analogRead(CENTER) < LINE_SENSOR && analogRead(RIGHT) < LINE_SENSOR){
          // 왼쪽 센서만 인식했을 경우 왼쪽으로 이동합니다.
            
            Leftward();
          }
            else if(analogRead(LEFT) > LINE_SENSOR && analogRead(CENTER) > LINE_SENSOR && analogRead(RIGHT) < LINE_SENSOR){
          // 왼쪽 센서만 인식했을 경우 왼쪽으로 이동합니다.
            
            Leftward();
          }
          else if(analogRead(RIGHT) > LINE_SENSOR && analogRead(CENTER) < LINE_SENSOR &&analogRead(LEFT) < LINE_SENSOR){
          // 오른쪽 센서만 인식했을 경우 오른쪽으로 이동합니다,
            
            Rightward();
          }
         else if(analogRead(RIGHT) > LINE_SENSOR && analogRead(CENTER) > LINE_SENSOR &&analogRead(LEFT) < LINE_SENSOR){
          // 오른쪽 센서만 인식했을 경우 오른쪽으로 이동합니다,
            
            Rightward();
          }
          
          else if (analogRead(CENTER) > LINE_SENSOR){
          // 가운데 센서가 인식 했을 경우 앞으로 이동합니다.
            Forward();   
           
          }
         }
         //delay(10);    
      }
     } //loop closed                     
        /*
         *    앞으로 이동    
         */
        
          void Forward() {
        
              //Motor one
              analogWrite(RIGHT_1, SPEED + 25);  //125
              analogWrite(RIGHT_2, 0);
            
              //Motor Two
              analogWrite(LEFT_1, SPEED + 25); //125
              analogWrite(LEFT_2, 0);
           
              delay(long_speed);
             
          }
        
          
          /*
          *  후진으로 이동
          */
            void Backward() {
             // Motor one
              
              analogWrite(RIGHT_1, 0);
              analogWrite(RIGHT_2, 150);  
            
              // Motor Two
              analogWrite(LEFT_1, 0);
              analogWrite(LEFT_2, 150); 
         
              delay(long_speed);
              
            }
        
        
           
            void Rightward() {
        
              
             //Motor one
              analogWrite(RIGHT_1, LEFT_SPEED);
              analogWrite(RIGHT_2, 0);
            
              //Motor Two
              analogWrite(LEFT_1, 0);
              analogWrite(LEFT_2, 0);
            
            delay(long_speed);
             
            }
        
        
           void Leftward() {
        
            
                //Motor One
                    analogWrite(RIGHT_1, 0);
                    analogWrite(RIGHT_2, 0);
        
                    //Motor Two
                    analogWrite(LEFT_1, LEFT_SPEED);
                    analogWrite(LEFT_2, 0);
        
                  delay(long_speed);
        
           }
        
        
             void Stop() {
        
               // stop Motor
              digitalWrite(RIGHT_1, LOW);
              digitalWrite(RIGHT_2, LOW);  
              digitalWrite(LEFT_1, LOW);
              digitalWrite(LEFT_2, LOW);
          
              delay(long_speed);
        
             }
         
    
   
   
  
