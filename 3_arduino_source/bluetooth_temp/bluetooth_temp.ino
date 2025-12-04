    #include <SoftwareSerial.h>

    SoftwareSerial BTSerial(4, 5); //TX 4 , RX 5 
   
    
    void setup() {
          Serial.begin(9600);
          BTSerial.begin(9600); 

    Serial.print("START");
    }
    
 void loop() {

     
                
   if (BTSerial.available()) {   //블루투스 연결이 되었을 때.. 
    Serial.print("GoGO");
    char data = (char)BTSerial.read(); 
    
 
    Serial.write(data); 
  
    
    if (data == 'o') {  // 앱에서 'o' 메시지 전달 -> 받기 
     Serial.println("on"); 
      
    }else if(data == 'f') {
     Serial.println("off");  
      
    }else if(data == 's')  {
     Serial.println("sensor"); 
    }
    
    } // bluetooth available close
 }  
