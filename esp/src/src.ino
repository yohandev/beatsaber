
#include <math.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "timer.h"
#include "peer.h"
#include "imu.h"
#include "num.h"
#include "message.h"

bool select_changed = false;

uint8_t b1_press;
uint8_t b2_press;

//enum for button states
enum button_state {S0, S1, S2, S3, S4};

class Button{
  public:
  uint32_t S2_start_time;
  uint32_t button_change_time;    
  uint32_t debounce_duration;
  uint32_t long_press_duration;
  uint8_t pin;
  uint8_t flag;
  uint8_t button_pressed;
  button_state state; // This is public for the sake of convenience
  Button(int p) {
  flag = 0;  
    state = S0;
    pin = p;
    S2_start_time = millis(); //init
    button_change_time = millis(); //init
    debounce_duration = 10;
    long_press_duration = 1000;
    button_pressed = 0;
  }
  void read() {
    uint8_t button_val = digitalRead(pin);  
    button_pressed = !button_val; //invert button
  }
  int update() {
    read();
    flag = 0;
    //State 0
    if (state==S0) {
      if (button_pressed) {
        button_change_time = millis();
        state = S1;
      }
    }
    //State 1
    else if (state==S1) {
      if (button_pressed && millis()-button_change_time >= debounce_duration){
        state = S2;
        S2_start_time = millis();
      }
      else if (not button_pressed){
        button_change_time = millis();
        state = S0;
      }
    } 
    //State 2
    else if (state==S2) {
      if (button_pressed && millis()-S2_start_time >= long_press_duration){
        state = S3;
      }
      else if (not button_pressed){
        button_change_time = millis();
        state = S4;
      }
    } 
    //State 3
    else if (state==S3) {
      if (not button_pressed){
        button_change_time = millis();
        state = S4;
      }
    }
    //State 4
    else if (state==S4) {      	
      if (not button_pressed && millis() - button_change_time >= debounce_duration) {
        if (millis() - S2_start_time >= long_press_duration+debounce_duration) {
          flag = 2;
        } else {
          flag = 1;
        }
        state = S0;
      } else if (button_pressed) {
        if (millis() - S2_start_time >= long_press_duration) {
          state = S3;
        } else {
          state = S2;
        }
        button_change_time = millis();
      }
    }
    return flag;
  }
};

// #define RECV
int isLeft = 1;
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

void loop() { /* It's empty here... */ }

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

int in_game = 0;

const char POST_URL[] = "POST https://608dev-2.net/sandbox/sc/team27/user_esp_server.py HTTP/1.1\r\n";
const char GET_URL[] = "GET https://608dev-2.net/sandbox/sc/team27/user_esp_server.py HTTP/1.1\r\n";

char network[] = "MIT";
char password[] = "";

uint8_t channel = 1; //network channel on 2.4 GHz
byte bssid[] = { 0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41 };  //6 byte MAC address of AP you're targeting.

uint32_t posting_timer = 0;

//Some constants and some resources:
const int RESPONSE_TIMEOUT = 6000;  //ms to wait for response from host
const int POSTING_PERIOD = 6000;    //ms to wait between posting step

const uint16_t IN_BUFFER_SIZE = 1000;   //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000;  //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE];    //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE];  //char array buffer to hold HTTP response

Button b1(BUTTON1);
Button b2(BUTTON2);


//for printing stuff
char output[100];

void setup() {
  //set up serial monitor
  Serial.begin(115200);
  while(!Serial);

  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      Serial.printf("%d: %s, Ch:%d (%ddBm) %s ", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "");
      uint8_t* cc = WiFi.BSSID(i);
      for (int k = 0; k < 6; k++) {
        Serial.print(*cc, HEX);
        if (k != 5) Serial.print(":");
        cc++;
      }
      Serial.println("");
    }
  }
  delay(100);  //wait a bit (100 ms)

  //if using regular connection use line below:
  WiFi.begin(network, password);


  uint8_t count = 0;  //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count < 12) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) {  //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.println(WiFi.localIP().toString() + " (" + WiFi.macAddress() + ") (" + WiFi.SSID() + ")");
    delay(500);
  } else {  //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart();  // restart the ESP (proper way)
  }
  
  //setup tft 
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);

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
  sprintf(output, "%f, %f, %f", gravity.x, gravity.y, gravity.z);
  Serial.println(output);
  imu.calibrate(10, DT, gravity);

  //buttons
  delay(100);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  
  Serial.println("initialization complete");
}

void loop() {
  if (in_game == 2){
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
        mess.isLeft = isLeft;
        peer.send((u8*)&mess, sizeof(message));
        Serial.write((u8*)&mess, sizeof(message));
        //this is just for demo purposes,  should actually be determined by the response from server
        draw_animationNumber = rand()%2+1;
        tft.fillScreen(TFT_BLACK);
        if (draw_animationNumber==1){
          score+=1;
        }
      }

      // button 2 can be used to reinitialize
      updateButton2();
      if(button2state == 3){
        Serial.println("reinitializing");
        gravity = imu.poll().get_acc();
        imu.calibrate(10, DT, gravity);
        sprintf(output, "%f, %f, %f", gravity.x, gravity.y, gravity.z);
        Serial.println(output);
        
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
      mess.isLeft = isLeft;
      peer.send((u8*)&mess, sizeof(message));
      Serial.write((u8*)&mess, sizeof(message));
    }
  }
  else{
    b1_press = b1.update();
    b2_press = b2.update();
    // Serial.println(b1_press);
    // Serial.println("NOT IN GAME");
    // Serial.println(b2_press);
    if (b1_press == 1){
      Serial.println("right");
      // make post request with step to the right
      // POST REQUEST
      char body[100];
      sprintf(body, "op=right");
      int body_len = strlen(body);
      sprintf(request_buffer, POST_URL);
      strcat(request_buffer, "Host: 608dev-2.net\r\n");
      strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
      sprintf(request_buffer + strlen(request_buffer), "Content-Length: %d\r\n", body_len);  //append string formatted to end of request buffer
      strcat(request_buffer, "\r\n"); //new line from header to body
      strcat(request_buffer, body); //body
      strcat(request_buffer, "\r\n"); //new line
      // Serial.println(request_buffer);
      do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
      // Serial.println(response_buffer);  //viewable in Serial Terminal
    }

    if (b2_press == 1){
      Serial.println("left");
      // make post request with step to the left
      // POST REQUEST
      char body[100];
      sprintf(body, "op=left");
      int body_len = strlen(body);
      sprintf(request_buffer, POST_URL);
      strcat(request_buffer, "Host: 608dev-2.net\r\n");
      strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
      sprintf(request_buffer + strlen(request_buffer), "Content-Length: %d\r\n", body_len);  //append string formatted to end of request buffer
      strcat(request_buffer, "\r\n"); //new line from header to body
      strcat(request_buffer, body); //body
      strcat(request_buffer, "\r\n"); //new line
      // Serial.println(request_buffer);
      do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
      // Serial.println(response_buffer);  //viewable in Serial Terminal
    }

    if (b1_press == 2){
      Serial.println("select");
      // make post request with select=True
      // POST REQUEST
      char body[100];
      sprintf(body, "op=select");
      int body_len = strlen(body);
      sprintf(request_buffer, POST_URL);
      strcat(request_buffer, "Host: 608dev-2.net\r\n");
      strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
      sprintf(request_buffer + strlen(request_buffer), "Content-Length: %d\r\n", body_len);  //append string formatted to end of request buffer
      strcat(request_buffer, "\r\n"); //new line from header to body
      strcat(request_buffer, body); //body
      strcat(request_buffer, "\r\n"); //new line
      // Serial.println(request_buffer);
      do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
      // Serial.println(response_buffer);  //viewable in Serial Terminal

      in_game += 1;
      select_changed = true;
      if (select_changed){
        char body[100];
        sprintf(body, "op=reset");
        int body_len = strlen(body);
        sprintf(request_buffer, POST_URL);
        strcat(request_buffer, "Host: 608dev-2.net\r\n");
        strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
        sprintf(request_buffer + strlen(request_buffer), "Content-Length: %d\r\n", body_len);  //append string formatted to end of request buffer
        strcat(request_buffer, "\r\n"); //new line from header to body
        strcat(request_buffer, body); //body
        strcat(request_buffer, "\r\n"); //new line
        // Serial.println(request_buffer);
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
        // Serial.println(response_buffer);  //viewable in Serial Terminal
        select_changed = false;
      }


      // // GET REQUEST
      // strcpy(request_buffer, GET_URL);
      // strcat(request_buffer, "Host: 608dev-2.net\r\n");
      // strcat(request_buffer, "\r\n");
    
      // do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, 1000, 0);
      // // Serial.println(response_buffer);
      // DynamicJsonDocument document(500);
      // DeserializationError error = deserializeJson(document, response_buffer);

      // if (error){
      //   Serial.println("error");
      // }
      // else{
      //   int in_game_state = document["in_game_state"];
      //   Serial.println(in_game_state);
      // }
    }
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
