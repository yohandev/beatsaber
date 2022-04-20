#include <WiFi.h>    //Connect to WiFi Network
#include <SPI.h>     //Used in support of TFT Display
#include <string.h>  //used for some string handling and processing.
#include <Wire.h>
#include <ArduinoJson.h>
#include <mpu6050_esp32.h>
#include <math.h>

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

const char POST_URL[] = "POST https://608dev-2.net/sandbox/sc/team27/esp_server.py HTTP/1.1\r\n";
const char GET_URL[] = "GET https://608dev-2.net/sandbox/sc/team27/esp_server.py HTTP/1.1\r\n";

char network[] = "MIT";
char password[] = "";

uint8_t channel = 1; //network channel on 2.4 GHz
byte bssid[] = { 0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41 };  //6 byte MAC address of AP you're targeting.

const uint8_t LOOP_PERIOD = 10;  //milliseconds
uint32_t posting_timer = 0;

//Some constants and some resources:
const int RESPONSE_TIMEOUT = 6000;  //ms to wait for response from host
const int POSTING_PERIOD = 6000;    //ms to wait between posting step

const uint16_t IN_BUFFER_SIZE = 1000;   //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000;  //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE];    //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE];  //char array buffer to hold HTTP response

long randNumber;
char mock_direction[10];
int score;

void setup() {
  Serial.begin(115200);  //begin serial comms
  while (!Serial);
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

  randomSeed(analogRead(0));
  posting_timer = millis();
}


void loop() {

  if (millis() - primary_timer > DT){
    primary_timer = millis();
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
  }

  if (millis() - posting_timer > POSTING_PERIOD) {
    posting_timer = millis();
    randNumber = random(4);

    if (randNumber == 0){
      strcpy(mock_direction, "left");
    }
    else if (randNumber == 1){
      strcpy(mock_direction, "right");
    }
    else if (randNumber == 2){
      strcpy(mock_direction, "up");
    }
    else if (randNumber == 3){
      strcpy(mock_direction, "down");
    }
    
  
    // POST REQUEST
    char body[100];
    sprintf(body, "dir=%s", direction);
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

    // GET REQUEST
    strcpy(request_buffer, GET_URL);
    strcat(request_buffer, "Host: 608dev-2.net\r\n");
    strcat(request_buffer, "\r\n");
  
    do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, 1000, 0);
    // Serial.println(response_buffer);
    DynamicJsonDocument document(500);
    DeserializationError error = deserializeJson(document, response_buffer);

    if (error){
      Serial.println("error");
    }
    else{
      score = document["score"];
    }
    // Serial.print("SCORE: ");
    // Serial.println(score);
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

