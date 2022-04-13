#include <WiFi.h>    //Connect to WiFi Network
#include <SPI.h>     //Used in support of TFT Display
#include <string.h>  //used for some string handling and processing.
#include <Wire.h>
#include <ArduinoJson.h>

const char POST_URL[] = "POST https://608dev-2.net/sandbox/sc/team27/esp_server.py HTTP/1.1\r\n";
const char GET_URL[] = "GET https://608dev-2.net/sandbox/sc/team27/esp_server.py HTTP/1.1\r\n";

char network[] = "MIT";
char password[] = "";

uint8_t channel = 1; //network channel on 2.4 GHz
byte bssid[] = { 0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41 };  //6 byte MAC address of AP you're targeting.

const uint8_t LOOP_PERIOD = 10;  //milliseconds
uint32_t primary_timer = 0;
uint32_t posting_timer = 0;

//Some constants and some resources:
const int RESPONSE_TIMEOUT = 6000;  //ms to wait for response from host
const int POSTING_PERIOD = 6000;    //ms to wait between posting step

const uint16_t IN_BUFFER_SIZE = 1000;   //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000;  //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE];    //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE];  //char array buffer to hold HTTP response

long randNumber;
char direction[10];
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
  randomSeed(analogRead(0));
  posting_timer = millis();
}


void loop() {
  if (millis() - posting_timer > POSTING_PERIOD) {
    posting_timer = millis();
    randNumber = random(4);

    if (randNumber == 0){
      strcpy(direction, "left");
    }
    else if (randNumber == 1){
      strcpy(direction, "right");
    }
    else if (randNumber == 2){
      strcpy(direction, "up");
    }
    else if (randNumber == 3){
      strcpy(direction, "down");
    }
    
    Serial.println(direction);
  
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
    Serial.print("SCORE: ");
    Serial.println(score);
  }
}
