#include "LedControlMS.h"  
#include <SoftwareSerial.h>

/* 초음파 센서 */
#define TRIG 7   //초음파를 보내는  D7핀 
#define ECHO A5  //초음파를 받는 D6핀   
      
#define RIGHT_1  5  //motor ONE  PWM 
#define RIGHT_2  6
   
#define LEFT_1  11    //motor TWO
#define LEFT_2  12

#define PIEZO 8

#define SPEED 220  //HIGH (90) , LOW(135) 

     #define  DIN 4
     #define  CS  3
     #define  CLK 2



/* 속도 딜레이 */
#define LONG_SPEED   10 
#define SHORT_SPEED  200   // HIGH 200, LOW 400
int toggle=0;


//DISTANCE 거리 부분 셋팅
#define DISTANCE 30
 
 #define NBR_MTX 1 // 8X8 LED Matrix를 몇개 사용할 지를 정한다. 현재 하나만 셋팅.
 LedControl lc=LedControl(DIN,CLK,CS, NBR_MTX); //8X8 LED Matrix 클래스를 세개의 핀으로 연결한다. 

        byte up[] =   //비행기 이모티콘 모양
         {
             B00011000,   
             B00111100,
             B01111110,
             B11111111,
             B00111100,
             B00111100,
             B00111100,
             B00111101
          };
          byte left[] =   //비행기 이모티콘 모양
         {
             B00011000,   
             B00111000,
             B01111000,
             B11111111,
             B11111111,
             B01111111,
             B00111000,
             B00011001
          };
          byte right[] =   //비행기 이모티콘 모양
         {
             B00011000,   
             B00011100,
             B00011110,
             B11111111,
             B11111111,
             B11111110,
             B00011100,
             B00011001
          };
          byte down[] =   //비행기 이모티콘 모양
         {
             B00011100,   
             B00011100,
             B00011100,
             B00011100,
             B11111111,
             B01111110,
             B00111100,
             B00011000
          };

          byte all[] =   //비행기 이모티콘 모양
         {
             B11111111,   
             B11111111,   
             B11111111,   
             B11111111,   
             B11111111,   
             B11111111,   
             B11111111,   
             B11111111   
            
          };
          



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

  pinMode(PIEZO,OUTPUT);


   pinMode(TRIG,OUTPUT);
   pinMode(ECHO,INPUT);  

    // 8X8 LED Matrix 클래스에  일일히 하나씩 LED 위치 등록하여 초기화 
       for (int i=0; i< NBR_MTX; i++){
        lc.shutdown(i,false);
        lc.setIntensity(i,8);
        lc.clearDisplay(i);
        
        delay(200);
      }


   printByte(all);


   tone(PIEZO,1000,100);

   Forward();
   delay(1000);
   tone(PIEZO,1000,100);

  
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
 
  void Backward() {

      //Motor one
      tone(8,1500,300);
      digitalWrite(RIGHT_1, HIGH);  //125
      digitalWrite(RIGHT_2, LOW);
    
      //Motor Two
      digitalWrite(LEFT_1, HIGH); //125
      digitalWrite(LEFT_2, LOW);
      lc.clearAll();
      printByte(down);
   
      delay(1000);
      tone(8,1500,100);
     
  }

  
  /*
  *  후진으로 이동
  */

    void Forward() {
     // Motor one
      
      digitalWrite(RIGHT_1, LOW);
      digitalWrite(RIGHT_2, SPEED);  
    
      // Motor Two
      digitalWrite(LEFT_1, LOW);
      digitalWrite(LEFT_2, SPEED); 

      lc.clearAll();
      printByte(up);
       
      delay(50);
      
    }


   
    void Rightward() {

      
     //Motor one
      digitalWrite(RIGHT_1, HIGH);
      digitalWrite(RIGHT_2, LOW);
    
      //Motor Two
      digitalWrite(LEFT_1, 0);
      digitalWrite(LEFT_2, LOW);
      lc.clearAll();
      printByte(right);
    
      delay(50);
     
    }


   void Leftward() {

    
        //Motor One
            digitalWrite(RIGHT_1, LOW);
            digitalWrite(RIGHT_2, LOW);

            //Motor Two
            digitalWrite(LEFT_1,HIGH);
            digitalWrite(LEFT_2, LOW);
             lc.clearAll();
            printByte(left);
            delay(50);


   }


     void Stop() {

       // stop Motor
      digitalWrite(RIGHT_1, LOW);
      digitalWrite(RIGHT_2, LOW);  
      digitalWrite(LEFT_1, LOW);
      digitalWrite(LEFT_2, LOW);

      printByte(all);

      tone(PIEZO,1000,200);
  
      delay(50);

     }


      void distance_check(){
      
          int check1 = distance();
          int check2 = distance();
          delay(10);
     
           
              if( (2 < check1 && DISTANCE > check1) && (2< check2 && DISTANCE > check2)) { //체크 될 경우에...
                 Stop();
                 Backward();
                
                 
                     toggle = random(2,4);
                     if(toggle == 2){ 
                             Leftward(); delay(500);
                            }else{ // toggle 다른 값
                              //toggle if 
                              Rightward(); delay(500);
                            }
                                      
               } // if 
               
               
               else{
                toggle = random(1,6);
                
                  if(toggle == 2 ){
                  Leftward();
                  delay(50);
                  
                  } else if(toggle == 4){      
                    Rightward();
                    delay(50);
                    
                  }else{
                    Forward();
                    delay(200);
                  }             
               }          
      }
      
  void printByte(byte character [])
            {
              int i = 0;
              for(i=0;i<8;i++)
              {
                lc.setColumn(0,7-i,character[i]);
              }
           }

