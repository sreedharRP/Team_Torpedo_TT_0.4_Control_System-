#include <util/delay.h>
#include <MPU9250_WE.h>
#include <Wire.h>
#include <Servo.h>
#include "MS5837.h"

// Sensor addressing 
#define MPU9250_ADDR 0x68
#define MS5837_ADDR  0x76
#define ALPHA 0.98

int MAX_SPEED = 2400;
int MIN_SPEED = 1000;

//Declaring the sensor
MPU9250_WE myMPU9250 = MPU9250_WE(MPU9250_ADDR);
MS5837 P_Sensor;
float  perivious_time, current_time, diff_T;
float yaw, f_yaw , est_yaw;
float p_yaw = 0;
float p , r;
float D , P , T, INITIAL_DEPTH , THRESHOLD_DEPTH;


// BMS PIN
uint8_t B_pin = 17;
float Voltage;


//SOFT_KILL_SWITCH
uint8_t Kill_Switch_Pin = 16;
//SOFT CODE CONDITION
int8_t kill_opp;



// LED Indicator 
uint8_t LED_1 = 33;
uint8_t LED_2 = 34;
uint8_t LED_3 = 35;
uint8_t LED_4 = 36;
uint8_t LED_5 = 37;
uint8_t LED_6 = 38;
uint8_t LED_7 = 39;
uint8_t LED_8 = 15;



// Thruster_PIN declaring
uint8_t Thruster_PIN_1 = 2;
uint8_t Thruster_PIN_2 = 3;
uint8_t Thruster_PIN_3 = 4;
uint8_t Thruster_PIN_4 = 5;
uint8_t Thruster_PIN_5 = 7;



// Thruster_initalization
Servo Thruster_1;
Servo Thruster_2;
Servo Thruster_3;
Servo Thruster_4;
Servo Thruster_5;


/*
    Code for the sensor
*/

void initi_IMU(uint8_t address){
    Wire.beginTransmission(address);
    Wire.write(0x00);
    Wire.endTransmission(false);

    myMPU9250.autoOffsets();
  
    xyzFloat aOffs = myMPU9250.getAccOffsets();  // get acceleration offsets
    xyzFloat gOffs = myMPU9250.getGyrOffsets();  // get gyroscope offsets 
    char buffer[35];
    sprintf(buffer, "{%d.0, %d.0, %d.0}", (int)aOffs.x, (int)aOffs.y, (int)aOffs.z);    
    sprintf(buffer, "{%d.0, %d.0, %d.0}", (int)gOffs.x, (int)gOffs.y, (int)gOffs.z);  
   

    myMPU9250.enableGyrDLPF();
    myMPU9250.setGyrDLPF(MPU9250_DLPF_6);
    myMPU9250.setSampleRateDivider(5);
    myMPU9250.setGyrRange(MPU9250_GYRO_RANGE_250);
    myMPU9250.setAccRange(MPU9250_ACC_RANGE_2G);
    myMPU9250.enableAccDLPF(true);
    myMPU9250.setAccDLPF(MPU9250_DLPF_6);
    delay(200);

    perivious_time = millis();


}

void initi_P_sensor(uint8_t address){
    Wire.beginTransmission(address);
    Wire.write(0x00);
    Wire.endTransmission(false);

    if (!P_Sensor.init()) {
       Serial.println("MS5837 initialization failed!");
        while (1); // Stop if sensor isn't initialized properly
    }

    P_Sensor.setFluidDensity(997);   
}

float D_time(){
  current_time = millis();
  diff_T = current_time - perivious_time;
  perivious_time = current_time;
  return diff_T;
}

float yaw_loop(float a){
  float t = D_time();
  float yaw =+ a * t; 
  return yaw;
}


float compli_filter(float a , float b, float c){
float filter ;
  filter = (ALPHA)*(c+b)+(1-ALPHA)*a;
  return filter;
}


void IMU_data(){
  xyzFloat gValue = myMPU9250.getGValues();
  xyzFloat gyr = myMPU9250.getGyrValues();

  float z2 = gyr.z;
  float z1 = gValue.z;
  
  est_yaw = compli_filter(z1 , z2 , p_yaw);
  
  p_yaw = est_yaw; 

  yaw = yaw_loop(est_yaw);
  f_yaw = yaw*(M_PI/180.0);
  f_yaw = f_yaw*0.000625;

   p = myMPU9250.getPitch();
   r = myMPU9250.getRoll();   
}

void P_Sensor_data(){
    P_Sensor.read();
    D = P_Sensor.depth();
    T = P_Sensor.temperature();
    P = P_Sensor.pressure();
}

/*
    Code for the sensor
*/



void blink(uint8_t a){
    digitalWrite(a,HIGH);
    _delay_ms(200);
    digitalWrite(a,LOW);
    _delay_ms(200);

}

void Stop_LED(){
    digitalWrite(LED_1,LOW);
    digitalWrite(LED_2,LOW);
    digitalWrite(LED_3,LOW);
    digitalWrite(LED_4,LOW);
    digitalWrite(LED_5,LOW);
    digitalWrite(LED_6,LOW);
    digitalWrite(LED_7,LOW);
    digitalWrite(LED_8,LOW);

}



int Kill_Switch(){
    uint8_t a = digitalRead(Kill_Switch_Pin);
    if (a==1){
        return a;
    }
    else{
        return a;
    }   
}

float BMS(){

    float v , percentage;
    int a = analogRead(B_pin);
    v = (3.33/(1023)*a);
    percentage = (100/3.33)*v; 
    return percentage;
}


void print_statment(float a , int b){
   Serial1.print("Battery level in voltage: ");
   Serial1.println(a);
   Serial1.print("Kill Swtich Status: ");
   Serial1.println(b);
}


void initial(){
    digitalWrite(Thruster_PIN_1,LOW);
    digitalWrite(Thruster_PIN_2,LOW);
    digitalWrite(Thruster_PIN_3,LOW);
    digitalWrite(Thruster_PIN_4,LOW);
    digitalWrite(Thruster_PIN_5,LOW);

}


void Motor_origin(){
    Thruster_1.write(0);
    Thruster_2.write(0);
    Thruster_3.write(0);
    Thruster_4.write(0);
    Thruster_5.write(0);
}


int8_t Soft_Reset(){
    digitalWrite(LED_5,HIGH);
    return -1;

}

void sensor_values(){
    IMU_data();
    P_Sensor_data();

  Serial1.print(p);
  Serial1.print("/");
  Serial1.print(r);
  Serial1.print("/");
  Serial1.print(f_yaw);
  Serial1.print("/");
  Serial1.print(D);
  Serial1.print("/");
  Serial1.print(P);
  Serial1.print("/");
  Serial1.print(T);
  Serial1.print("/");
  Serial1.print(INITIAL_DEPTH);
  Serial1.println("");

}


void Kill_Operation(){
    Motor_origin();
    kill_opp = Soft_Reset();
   Serial.println(kill_opp);
}


void forward_motion(int a){
    Thruster_4.writeMicroseconds(a);
    Thruster_5.writeMicroseconds(a);
    sensor_values();
    //`Stop_LED();

}

void Heave_motion(int a){
    Thruster_1.writeMicroseconds(a);
    Thruster_2.writeMicroseconds(a);
    Thruster_3.writeMicroseconds(a);
    sensor_values();
    //Stop_LED();
}

void GoDeep(){
    Serial1.println("Going Deep");
    blink(LED_1);
}


// void test_mostion(){
//     Thruster_4.writeMicroseconds(1500);
//     Thruster_5.writeMicroseconds(1500);
//     digitalWrite(LED_7,HIGH);
// }


void setup(){
    
    // Enable INPUT PINS
    Serial.begin(115200);
    Serial1.begin(115200);
    pinMode(B_pin,INPUT);
    pinMode(Kill_Switch_Pin,INPUT);

    pinMode(LED_1,OUTPUT);
    pinMode(LED_2,OUTPUT);
    pinMode(LED_3,OUTPUT);
    pinMode(LED_4,OUTPUT);
    pinMode(LED_5,OUTPUT);
    pinMode(LED_6,OUTPUT);
    pinMode(LED_7,OUTPUT);
    pinMode(LED_8,OUTPUT);

    // Setting the Sensors
    Wire.begin();
    _delay_ms(100);
    initi_IMU(MPU9250_ADDR);
    _delay_ms(500);
    initi_P_sensor(MS5837_ADDR);

    attachInterrupt(digitalPinToInterrupt(Kill_Switch_Pin),Kill_Operation,HIGH);

    //Setting_up thrusters
    pinMode(Thruster_PIN_1,OUTPUT);
    Thruster_1.attach(Thruster_PIN_1,1000,2000);
    pinMode(Thruster_PIN_2,OUTPUT);
    Thruster_2.attach(Thruster_PIN_2,1000,2000);
    pinMode(Thruster_PIN_3,OUTPUT);
    Thruster_3.attach(Thruster_PIN_3,1000,2000);
    pinMode(Thruster_PIN_4,OUTPUT);
    Thruster_4.attach(Thruster_PIN_4,1000,2000);
    pinMode(Thruster_PIN_5,OUTPUT);
    Thruster_5.attach(Thruster_PIN_5,1000,2000);


    initial();
    _delay_ms(5000);
   Serial1.println("Initialisation Over!");
   Serial1.println("Starting Over!");

   P_Sensor.read();
   INITIAL_DEPTH = P_Sensor.pressure();
   Motor_origin();



}

void loop(){


    uint8_t Kill = Kill_Switch();
    Voltage = BMS();

    digitalWrite(LED_5,LOW);

    print_statment(Voltage,Kill);
    _delay_ms(500);

    if(P<100){
        Heave_motion(MAX_SPEED);
        GoDeep();
    }
    else{
        Heave_motion(MIN_SPEED);
        Stop_LED();
    }

    forward_motion(MAX_SPEED);
    
    // test_mostion();

}