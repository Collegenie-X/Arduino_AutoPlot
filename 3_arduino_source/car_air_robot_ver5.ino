
#include <SoftwareSerial.h>

/* 초음파 센서 */
#define ECHO 3
#define TRIG 4


#define RIGHT_1  5  //motor ONE  PWM 
#define RIGHT_2  6

#define LEFT_1  11    //motor TWO
#define LEFT_2  12

#define SPEED 120  //HIGH (90) , LOW(135) 



/* 속도 딜레이 */
#define LONG_SPEED   10 
#define SHORT_SPEED  200   // HIGH 200, LOW 400
int toggle=0;


//DISTANCE 거리 부분 셋팅
#define DISTANCE 12
//SoftwareSerial BTSerial(RX,TX);  //bluetooth TX,RX 클로스 연결


void Forward();
void Backward();
long distance();
void Rightward();
void Leftward();
void Stop();


void setup()
{
  Serial.begin(9600);
   
  pinMode(RIGHT_1, OUTPUT);
  pinMode(RIGHT_2, OUTPUT);
  pinMode(LEFT_1, OUTPUT);
  pinMode(LEFT_2, OUTPUT);


   pinMode(TRIG,OUTPUT);
   pinMode(ECHO,INPUT);  


   Forward();
   delay(1000);
   Backward();
   delay(1000);
   Rightward();
   delay(1000);
   Stop();
   

  
}

void loop()
{
      distance_check();
      delay(20);    
}


long distance(){
        // put your main code here, to run repeatedly:
         digitalWrite(TRIG,LOW);
         delayMicroseconds(20);
         digitalWrite(TRIG,HIGH);
         delayMicroseconds(50);
         digitalWrite(TRIG,LOW);
      
         long distance = pulseIn(ECHO,HIGH,4800)/58;  //1m
      
         Serial.print("distance:");
         Serial.println(distance);
      
         return distance;
       }

    
/*
 *    앞으로 이동    
 */
 
  void Forward() {

      //Motor one
      analogWrite(RIGHT_1, SPEED);  //125
      analogWrite(RIGHT_2, LOW);
    
      //Motor Two
      analogWrite(LEFT_1, SPEED); //125
      analogWrite(LEFT_2, LOW);
   
      delay(LONG_SPEED);
     
  }

  
  /*
  *  후진으로 이동
  */

    void Backward() {
     // Motor one
      
      digitalWrite(RIGHT_1, LOW);
      digitalWrite(RIGHT_2, SPEED);  
    
      // Motor Two
      digitalWrite(LEFT_1, LOW);
      digitalWrite(LEFT_2, SPEED); 
 
     delay(500);
      
    }


   
    void Rightward() {

      
     //Motor one
      analogWrite(RIGHT_1, 180);
      digitalWrite(RIGHT_2, LOW);
    
      //Motor Two
      analogWrite(LEFT_1, 0);
      digitalWrite(LEFT_2, LOW);
    
      delay(SHORT_SPEED);
     
    }


   void Leftward() {

    
        //Motor One
            analogWrite(RIGHT_1, 0);
            digitalWrite(RIGHT_2, LOW);

            //Motor Two
            analogWrite(LEFT_1,180);
            digitalWrite(LEFT_2, LOW);

            delay(SHORT_SPEED);

   }


     void Stop() {

       // stop Motor
      digitalWrite(RIGHT_1, LOW);
      digitalWrite(RIGHT_2, LOW);  
      digitalWrite(LEFT_1, LOW);
      digitalWrite(LEFT_2, LOW);
  
      delay(10);

     }


      void distance_check(){
      
          int check1 = distance();
          int check2 = distance();
          delay(5);
     
           
              if( (2 < check1 && DISTANCE > check1) && (2< check2 && DISTANCE > check2)) { //체크 될 경우에...

                 tone(8,1500,300);
                 Backward();
                 Backward();
                 Backward();
                 
               toggle = random(2,5);
                    if(toggle == 2){ 
                      for(int i=0; i<4; i++){ Leftward(); }
                      }else{ // toggle 다른 값
                        //toggle if 
                        for(int i=0; i<4; i++){ Rightward(); }
                      }
                                
               } // if 
               else{
                toggle = random(1,20);
                
                  if(toggle == 2 ){
                  Leftward();
                  
                  } else if(toggle == 4){      
                    Rightward();
                    Rightward();
                  }else{
                    Forward();
                  }             
               }          
      }
      
 

