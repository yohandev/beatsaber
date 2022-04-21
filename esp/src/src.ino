#include <mpu6050_esp32.h>
#include <math.h>
#include <Wire.h>

MPU6050 imu;

//angle of rotation around x axis
float ang_velx  = 0;  
float ang_vely = 0;
float ang_x = 0;
float ang_y = 0;

//timer stuff, DT is total loop time, must be set for functions to work properly
const int DT = 50;
uint32_t primary_timer;

//buttons
const int BUTTON1 = 45;
uint8_t button1state;
const int BUTTON2 = 39;
uint8_t button2state;

//variable to hold drift values
float y_drift,z_drift, ang_driftx, ang_drifty;

//simple struct
struct vector {
  float x, y, z;
};

//vectors
struct vector gravity;
struct vector accel_real;
struct vector vel_real;
struct vector pos_real;

//will contain direction of most recent motion "0100" 
char direction[5];

//for printing stuff
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
  ang_x-=ang_driftx;
  ang_y-= ang_drifty;

  calcAccel();
  updateButton1();
  if (button1state ==0){
    //if essentially still, recalculate angle
    imu.readAccelData(imu.accelCount);
    float x = imu.accelCount[0] * imu.aRes; 
    float y = imu.accelCount[1] * imu.aRes; 
    float z = imu.accelCount[2] * imu.aRes;
    if (abs(sqrt(x*x+y*y+z*z)-1.08)<0.01){
      resetOrientation();
    }
  }
  else if (button1state==1){
    //button just pushed, reset pos and vel vectors
    pos_real.y=0;
    pos_real.z=0;
    vel_real.y=0;
    vel_real.z=0;
  } else if(button1state==2){
    //while button held keep updating position
    calcPos();
    pos_real.y-=y_drift;
    pos_real.z-=z_drift;
  } else if (button1state==3){
    //when button released determine direction of motion
    chooseDirection();
  }

  // button 2 can be used to reinitialize
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

