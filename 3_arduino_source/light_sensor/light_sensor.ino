
#define LIGHT_SENSOR A0      //조도 센서 PIN 설정

int light = 0; 
void setup() {

    Serial.begin(9600);
}

void loop() {

        light= analogRead(LIGHT_SENSOR);  // 0~ 1023
        light = map(light, 100, 900, 0, 100); 
        light = constrain(light,0,100); 
        Serial.println("light:" +String(light));
        delay(50);

        

}
