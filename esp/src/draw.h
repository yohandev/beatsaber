#pragma once

#include <stdint.h>
#include <Arduino.h>

class Draw{
  int draw_counter;
  int draw_animationNumber;
  int old_note;
  TFT_eSPI tft;
  uint8_t AUDIO_TRANSDUCER;
  uint8_t AUDIO_PWM;



public:

void begin(){
  draw_counter =0;
  draw_animationNumber =0;
  old_note =0;
  AUDIO_TRANSDUCER = 14;
  AUDIO_PWM = 0;
  tft = TFT_eSPI();
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  pinMode(AUDIO_TRANSDUCER, OUTPUT);
  ledcSetup(AUDIO_PWM, 200, 12);//12 bits of PWM precision
  ledcWrite(AUDIO_PWM, 0); //0 is a 0% duty cycle for the NFET
  ledcAttachPin(AUDIO_TRANSDUCER, AUDIO_PWM);
}

void make_sound(){
  
  if (draw_animationNumber==1 && old_note!=493 && draw_counter<7){
    ledcWriteTone(AUDIO_PWM, 493);
    old_note = 493;
  }else if (draw_animationNumber==2 && old_note!=110 && draw_counter<7){
    ledcWriteTone(AUDIO_PWM, 110);
    old_note = 110;
  } else if ((draw_animationNumber ==0 || draw_counter>7) && old_note!=0){
    ledcWriteTone(AUDIO_PWM, 0);
    old_note = 0;
  }
}

void draw_startScreen(){
  tft.fillScreen(TFT_BLACK);

  tft.setFreeFont(&FreeSansOblique18pt7b);
  tft.setCursor(15,60);
  tft.setTextColor(tft.color565(255, 50, 50));
  tft.println("BEAT");
  
  tft.setCursor(0,100);
  tft.setTextColor(tft.color565(0, 200, 255));
  tft.println("SABER");

  tft.setFreeFont(NULL);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0,140);
  tft.println(" use website to begin");
}

void draw_score(){
  if (draw_animationNumber==0){
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&FreeSansOblique12pt7b);
  tft.setCursor(0,85);
  tft.printf("SCORE: %d", score);
  }
}

void draw_hit(){
  if(draw_animationNumber==1){
    if(draw_counter<10){
      tft.setFreeFont(&FreeSansOblique18pt7b);
        
      tft.drawCircle(64,80,draw_counter*15,TFT_GREEN);
      tft.drawCircle(64,80,(draw_counter-1)*15,TFT_BLACK);
      //tft.drawCircle(64,80,draw_counter*15-2,TFT_BLACK);
      tft.setCursor(17,90);
      tft.setTextColor(tft.color565(0,255*draw_counter/10 , 0));
      tft.println("NICE!");
      draw_counter+=1;  
    } else if (draw_counter<=20){
      draw_counter++;
    }
    else{
      draw_counter=0;
      draw_animationNumber = 0;
      tft.fillScreen(TFT_BLACK);
    }
  }


}

void draw_miss(){
  tft.setFreeFont(&FreeSansOblique18pt7b);

  if (draw_animationNumber==2){
    if(draw_counter<10){
      tft.fillTriangle(34,40,94,120,104,110,tft.color565(255,0 , 0));
      tft.fillTriangle(34,40,94,120,24,50,tft.color565(255,0 , 0));

      tft.fillTriangle(34,120,94,40,104,50,tft.color565(255,0 , 0));
      tft.fillTriangle(34,120,94,40,24,110,tft.color565(255,0 , 0));
      draw_counter+=1;
    }else if (draw_counter<20){
      tft.fillTriangle(34,40,94,120,104,110,tft.color565(255-255*(draw_counter-10)/10,0 , 0));
      tft.fillTriangle(34,40,94,120,24,50,tft.color565(255-255*(draw_counter-10)/10,0 , 0));

      tft.fillTriangle(34,120,94,40,104,50,tft.color565(255-255*(draw_counter-10)/10,0 , 0));
      tft.fillTriangle(34,120,94,40,24,110,tft.color565(255-255*(draw_counter-10)/10,0 , 0));
      draw_counter+=2;
    } else{
      draw_counter = 0;
      draw_animationNumber = 0;
      tft.fillScreen(TFT_BLACK);
    }
    
  }
}

void draw(){
  make_sound();
  draw_miss();
  draw_hit();
  draw_score();
}

}