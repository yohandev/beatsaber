
#include <math.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <Arduino.h>
#include "timer.h"
#include "peer.h"
#include "imu.h"
#include "num.h"
#include "message.h"

// #define RECV
int isRight = 0;
//using int because will be padded anyways

#if defined RECV
void setup() {
    // Website is configured for 115200 bps
    Serial.begin(115200);

    // Await serial
    while (!Serial);

    // Place this in ADDR below
    Serial.println(Peer::addr());

    // Relay every message received
    Peer::recv(+[](const u8* mac, const u8* buf, int len) {
        Serial.write(buf, len);

    });
}

void loop() { Serial.println("getting a response");Serial.println(Serial.read());/* It's empty here... */ }

#else

const u8 ADDR[] = { 0x7C, 0xDF, 0xA1, 0x0F, 0x07, 0x66 };

const int DT = 50;
Timer timer(DT);    // Serial write timer
Imu imu;            // MPU6050

Peer peer(ADDR);    // Peer networking


//MPU6050 imu;
TFT_eSPI tft = TFT_eSPI();

//buttons
const int BUTTON1 = 45;
uint8_t button1state = 0;
const int BUTTON2 = 39;
uint8_t button2state = 0;

//drawing stuff
int draw_animationNumber = 0;
int draw_counter = 0;

//audio stuff
uint8_t AUDIO_TRANSDUCER = 14;
uint8_t AUDIO_PWM = 0;
int old_note = 0;

//score
int score = 0;

vec3 ang;           // current angle (integrated)
vec3 acc;           // acceleration rotated with gravity removed
vec3 gravity;       // gravity vector
vec3 vel;           // velocity integrated from acc
vec3 pos;           // position integrated from vel

int direction = 0;





//for printing stuff
char output[100];

void setup() {
  //set up serial monitor
  Serial.begin(115200);
  while(!Serial);

  //setup tft 
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);

  //set up audio
  pinMode(AUDIO_TRANSDUCER, OUTPUT);
  ledcSetup(AUDIO_PWM, 200, 12);//12 bits of PWM precision
  ledcWrite(AUDIO_PWM, 0); //0 is a 0% duty cycle for the NFET
  ledcAttachPin(AUDIO_TRANSDUCER, AUDIO_PWM);

  // start peer
  peer.begin();

  //set up imu
  imu.begin();
  gravity = imu.poll().get_acc();
  //sprintf(output, "%f, %f, %f", gravity.x, gravity.y, gravity.z);
  //Serial.println(output);
  imu.calibrate(10, DT, gravity);

  //buttons
  delay(100);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  
  //Serial.println("initialization complete");
}

void loop() {
  if(timer.poll()){
    //integrate for angle
    getAngle();
    //rotates accelation based on angle
    calcAccel();
    updateButton1();
    if (button1state ==0){
      //if essentially still, recalculate angle
      vec3 curr_acc = imu.accel();
      if (abs(sqrt(curr_acc.x*curr_acc.x+curr_acc.y*curr_acc.y+curr_acc.z*curr_acc.z)-10.5)<0.1){
        resetOrientation();
        ang.z = 0;
      }
    }
    else if (button1state==1){
      //button just pushed, reset pos and vel vectors
      pos = {.x=0, .y = 0, .z = 0};
      vel = {.x=0, .y = 0, .z = 0};
    } else if(button1state==2){
      //while button held keep updating position
      calcPos();
    } else if (button1state==3){
      //when button released determine direction of motion
      chooseDirection();
      message mess;
      mess.ang.x = -ang.x-3.14*0.5;
      mess.ang.y = ang.y;
      mess.ang.z = ang.z;
      mess.direction = direction;
      mess.isRight = isRight;
      peer.send((u8*)&mess, sizeof(message));
 
      Serial.write((u8*)&mess, sizeof(message));
      delay(10);
      
    }

    // button 2 can be used to reinitialize
    updateButton2();
    if(button2state == 3){
      Serial.println("reinitializing");
      gravity = imu.poll().get_acc();
      imu.calibrate(10, DT, gravity);
      ang = {.x=0, .y = 0, .z = 0};
      sprintf(output, "%f, %f, %f", gravity.x, gravity.y, gravity.z);
      Serial.println(output);
      
    }
    int incoming = Serial.read();
    if(incoming!=-1){
      tft.fillScreen(TFT_BLACK);
      if(incoming>0){
        draw_animationNumber = 1;
        
        score+=incoming;
      }else{
        draw_animationNumber = 2;
      }
    //   incoming = -1;
    }

    //response animation and sounds
    make_sound();
    draw_miss();
    draw_hit();
    draw_score();
    
    message mess;
    mess.ang.x = -ang.x-3.14*0.5;
    mess.ang.y = ang.y;
    mess.ang.z = ang.z;
    mess.direction = -1;
    mess.isRight = isRight;
    peer.send((u8*)&mess, sizeof(message));
    Serial.write((u8*)&mess, sizeof(message));
  }
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
#endif

