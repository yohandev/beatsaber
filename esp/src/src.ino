#include <mpu6050_esp32.h>
#include <math.h>
#include <Wire.h>

MPU6050 imu;

float ang_vel  = 0;  //used for holding the magnitude of acceleration
float ang_pos = 0;

const int DT = 50;
uint32_t primary_timer;
uint32_t swipe_timer;

const int BUTTON1 = 45;
uint8_t button1state;

const int BUTTON2 = 39;
uint8_t button2state;

float y_drift,z_drift, ang_drift;

struct vector {
  float y, z;
};

struct vector gravity;
struct vector accel_real;
struct vector vel_real;
struct vector pos_real;

float start_altitude;

char direction[5];

char output[100];

void setup() {
  //set up serial monitor
  Serial.begin(115200);
  while(!Serial);


  //set up imu
  if (imu.setupIMU(1)) {
    Serial.println("IMU Connected!");
  } else {
    Serial.println("IMU Not Connected :/");
    Serial.println("Restarting");
    ESP.restart(); // restart the ESP (proper way)
  }

  //buttons
  delay(100);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  
  // remote should be flat for this
  initializeOrientation();
  calculateDrift();
  
  Serial.println("initialization complete");
}

void loop() {

  getAngle();
  ang_pos-=ang_drift;

  calcAccel();
  updateButton1();
  if (button1state ==0){
    imu.readAccelData(imu.accelCount);
    float x = imu.accelCount[0] * imu.aRes; 
    float y = imu.accelCount[1] * imu.aRes; 
    float z = imu.accelCount[2] * imu.aRes;
    if (abs(sqrt(x*x+y*y+z*z)-1.08)<0.01){
      resetOrientation();
    }
  }
  else if (button1state==1){
    pos_real.y=0;
    pos_real.z=0;
    vel_real.y=0;
    vel_real.z=0;
    //start_altitude = bmp.readAltitude();
    swipe_timer=millis();
  } else if(button1state==2){
    calcPos();
    pos_real.y-=y_drift;
    pos_real.z-=z_drift;
  } else if (button1state==3){
    chooseDirection();
  }

  updateButton2();
  if(button2state == 3){
    Serial.println("reinitializing");
    initializeOrientation();
    calculateDrift();
  }
   
  while(millis()-primary_timer<DT);
  primary_timer = millis();
  
}


void updateButton1(){
  int value = digitalRead(BUTTON1);
  if (button1state==0 && !value){
    button1state = 1;
  } else if (button1state==1){
    button1state = 2;
  } else if (button1state==2 && value){
    button1state =3;
  }else if (button1state == 3){
    button1state =0;
  }
}

void updateButton2(){
  int value = digitalRead(BUTTON2);
  if (button2state==0 && !value){
    button2state = 1;
  } else if (button2state==1){
    button2state = 2;
  } else if (button2state==2 && value){
    button2state =3;
  }else if (button2state == 3){
    button2state =0;
  }
}

