    #include <SoftwareSerial.h>
    
    
    /* 피에조 스피커 */
    #define PIEZO 8  //PIEZO
    
    
    #define FRONT_RIGHT_MOTOR  5  //motor ONE  PWM 
    #define BACK_RIGHT_MOTOR  6
    
    #define FRONT_LEFT_MOTOR  11    //motor TWO
    #define BACK_LEFT_MOTOR  12
    
    
    #define LEFT     A0
    #define CENTER   A1
    #define RIGHT    A2
    
    #define SPEED 130       //150 (LOW) ,80 (HIGH)
    #define LEFT_SPEED 150  // 220 (LOW),95 (HIGH)
    
    #define LINE_SENSOR  850    
    
    
    int long_speed  = 5;
      
        
        void setup()
        {
          Serial.begin(9600);
           
          pinMode(FRONT_RIGHT_MOTOR, OUTPUT);
          pinMode(BACK_RIGHT_MOTOR, OUTPUT);
          pinMode(FRONT_LEFT_MOTOR, OUTPUT);
          pinMode(BACK_LEFT_MOTOR, OUTPUT);

         Forward(); 
         delay(1500);
         Leftward();
         delay(1500);
         Rightward();
         delay(1500);
         Stop(); 
         delay(1000);
         
        }
        
        void loop()
        {
        
         if(analogRead(RIGHT) > LINE_SENSOR  && analogRead(CENTER) > LINE_SENSOR && analogRead(LEFT) > LINE_SENSOR){
          // 3개의 센서가 모두 인식이 됬을 경우는 멈춰있습니다.
           //Backward();
               Stop();   
          }
          else if(analogRead(LEFT) > LINE_SENSOR && analogRead(CENTER) < LINE_SENSOR && analogRead(RIGHT) < LINE_SENSOR){
          // 왼쪽 센서만 인식했을 경우 왼쪽으로 이동합니다.
            
            Leftward();
           
          }
          else if(analogRead(RIGHT) > LINE_SENSOR && analogRead(CENTER) < LINE_SENSOR &&analogRead(LEFT) < LINE_SENSOR){
          // 오른쪽 센서만 인식했을 경우 오른쪽으로 이동합니다,
            
            Rightward();
         
          }
          
          else if(analogRead(CENTER) > LINE_SENSOR){
          // 가운데 센서가 인식 했을 경우 앞으로 이동합니다.
            Forward();   
           
          }
         
         //delay(10);    
        }
        
            
        /*
         *    앞으로 이동    
         */
         
          void Forward() {
        
              //Motor one
              analogWrite(FRONT_RIGHT_MOTOR, SPEED);  //125
              analogWrite(BACK_RIGHT_MOTOR, 0);
            
              //Motor Two
              analogWrite(FRONT_LEFT_MOTOR, SPEED); //125
              analogWrite( BACK_LEFT_MOTOR, 0);
           
              delay(long_speed);
             
          }
        
          
          /*
          *  후진으로 이동
          */
            void Backward() {
             // Motor one
              
              analogWrite(FRONT_RIGHT_MOTOR, 0);
              analogWrite(BACK_RIGHT_MOTOR, 150);  
            
              // Motor Two
              analogWrite(FRONT_LEFT_MOTOR, 0);
              analogWrite(BACK_LEFT_MOTOR, 150); 
         
              delay(long_speed);
              
            }
        
        
           
            void Rightward() {
        
              
             //Motor one
              analogWrite(FRONT_RIGHT_MOTOR, LEFT_SPEED);
              analogWrite(BACK_RIGHT_MOTOR, 0);
            
              //Motor Two
              analogWrite(FRONT_LEFT_MOTOR, 0);
              analogWrite(BACK_LEFT_MOTOR, 0);
            
            delay(long_speed);
             
            }
        
        
           void Leftward() {
        
            
                //Motor One
                    analogWrite(FRONT_RIGHT_MOTOR, 0);
                    analogWrite(BACK_RIGHT_MOTOR, 0);
        
                    //Motor Two
                    analogWrite(FRONT_LEFT_MOTOR, LEFT_SPEED);
                    analogWrite(BACK_LEFT_MOTOR, 0);
        
                  delay(long_speed);
        
           }
        
        
             void Stop() {
        
               // stop Motor
              digitalWrite(FRONT_RIGHT_MOTOR, LOW);
              digitalWrite(BACK_RIGHT_MOTOR, LOW);  
              digitalWrite(FRONT_LEFT_MOTOR, LOW);
              digitalWrite(BACK_LEFT_MOTOR, LOW);
          
              delay(long_speed);
        
             }
        
         

