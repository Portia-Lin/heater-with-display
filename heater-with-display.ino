#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "GyverEncoder.h"
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define TFT_CS 9
#define TFT_RST 12
#define TFT_DC 10
#define ENC_CLK 2
#define ENC_DT 3
#define ENC_SW 4
#define H_OUTPUT 7
#define DS18B20 6

int pointer = 0;
int mode = 0;
int state = 0;
int temperature = 40;
int actTemperature = 0;
int startTime = 0;
long lastUpdateTime = 0;
const int updateTime = 2000;
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Encoder enc1(ENC_CLK, ENC_DT, ENC_SW);
OneWire oneWire(DS18B20);
DallasTemperature sensor(&oneWire);

//временные выводы для питания
int gnd = 8;
int gnd_ds = 5;
int v5 = A0;

void setup() {
  // put your setup code here, to run once:
  pinMode(H_OUTPUT, H_OUTPUT);
  digitalWrite(H_OUTPUT, LOW);

  pinMode(gnd, OUTPUT);
  pinMode(gnd_ds, OUTPUT);
  pinMode(v5, OUTPUT);
  digitalWrite(gnd, LOW);
  digitalWrite(gnd_ds, LOW);
  digitalWrite(v5, HIGH);
  
  Serial.begin(9600);
  Serial.println("Hello! ST7765 TFT Menu");
  tft.initR(INITR_BLACKTAB);
  enc1.setType(TYPE2);
  sensor.begin();
  sensor.setResolution(9);
  detectTemperature();
  delay(500);
  menu();
  displayPointer();
}

void loop() {
  // put your main code here, to run repeatedly:
  enc1.tick();
  if (enc1.isTurn() && state == 0) displayPointer(); //разберись с mode
  if (enc1.isClick() && pointer == 3) changeState();
  if (millis() - lastUpdateTime > updateTime) {
    lastUpdateTime = millis();
    detectTemperature();
    temperatureControl();
  }
}

void menu() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
  tft.setCursor(22, 10);
  tft.print("TEMPERATURE:");
  tft.setCursor(22, 38);
  tft.print("START AFTER:");
  tft.setCursor(22, 66);
  tft.print("MODE:");
  tft.setCursor(22, 94);
  tft.print("STATUS:");
  
  tft.setTextColor(ST77XX_ORANGE);
  tft.setTextSize(2);
  tft.setCursor(22, 20);
  tft.print(String(temperature) + "'C");
  tft.setCursor(22, 48);
  tft.print(String(startTime) + "m");
  tft.setCursor(22, 76);
  tft.print("Normal");
  tft.setCursor(22, 104);
  tft.print("START");

  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(3);
  tft.setCursor(22, 130);
  tft.print("100'C");
}

void displayPointer() {
  if (enc1.isRight()) {
      pointer++;
      if (pointer >= 4) pointer = 0;
    }
    if (enc1.isLeft()) {
      pointer--;
      if (pointer < 0) pointer = 3;
    }
    
    tft.fillRoundRect(7, 20, 10, 100, 0, ST77XX_BLACK);
    tft.setTextColor(ST77XX_BLUE);
    tft.setTextSize(2);
    switch(pointer) {
    case 0:
      tft.setCursor(7, 20);
      tft.print(">");
      break;
    case 1:
      tft.setCursor(7, 48);
      tft.print(">");
      break;
    case 2:
      tft.setCursor(7, 76);
      tft.print(">");
      break;
    case 3:
      tft.setCursor(7, 104);
      tft.print(">");
      break;
    }

    if (enc1.isRightH()) {
      tft.setTextColor(ST77XX_ORANGE);
      switch (pointer) {
        case 0: 
          temperature = temperature + 2;
          tft.fillRoundRect(22, 20, 75, 14, 0, ST77XX_BLACK);
          tft.setCursor(22, 20);
          tft.print(String(temperature) + "'C");
          break;
        case 1: 
          startTime = startTime + 5;
          tft.fillRoundRect(22, 48, 75, 14, 0, ST77XX_BLACK);
          tft.setCursor(22, 48);
          tft.print(String(startTime) + "m");
          break;
        case 2: 
          mode = !mode;
          tft.fillRoundRect(22, 76, 75, 14, 0, ST77XX_BLACK);
          tft.setCursor(22, 76);
          if (mode == 0) tft.print("Normal");
          if (mode == 1) tft.print("Hold");
          break;
      }
    }
    
    if (enc1.isLeftH()) {
      tft.setTextColor(ST77XX_ORANGE);
      switch (pointer) {
        case 0: 
          temperature = temperature - 2;
          if (temperature < 0 ) temperature = 0;
          tft.fillRoundRect(22, 20, 75, 14, 0, ST77XX_BLACK);
          tft.setCursor(22, 20);
          tft.print(String(temperature) + "'C");
          break;
        case 1: 
          startTime = startTime - 5;
          if (startTime < 0 ) startTime = 0;
          tft.fillRoundRect(22, 48, 75, 14, 0, ST77XX_BLACK);
          tft.setCursor(22, 48);
          tft.print(String(startTime) + "m");
          break;
        case 2: 
          mode = !mode;
          tft.fillRoundRect(22, 76, 75, 14, 0, ST77XX_BLACK);
          tft.setCursor(22, 76);
          if (mode == 0) tft.print("Normal");
          if (mode == 1) tft.print("Hold");
          break;
      }
    }
}

void changeState() {
  state = !state;
  Serial.println(state);
  tft.fillRoundRect(22, 104, 75, 14, 0, ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(22, 104);
  switch(state) {
    case 0:
      tft.setTextColor(ST77XX_ORANGE);
      tft.print("START");
      digitalWrite(H_OUTPUT, LOW);
      Serial.println("Heater is Disable");
      break;
    case 1: 
      tft.setTextColor(ST77XX_RED);
      tft.print("STOP");
      digitalWrite(H_OUTPUT, HIGH);
      Serial.println("Heater is enabled with parameters:");
      Serial.println("Required temperature: " + String(temperature) + "(*C)");
      Serial.println("Enable after: " + String(startTime) + "(m)");
      Serial.println("Selected mode: " + String(mode));
      Serial.println("Current temperature: " + String(actTemperature) + "(*C)");
      break;
  }
}

void detectTemperature(){
    float tempTemperature;
    sensor.requestTemperatures();
    tempTemperature = sensor.getTempCByIndex(0);
    actTemperature = tempTemperature;
    Serial.println(actTemperature);
    tft.fillRoundRect(22, 130, 90, 21, 0, ST77XX_BLACK);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(3);
    tft.setCursor(22, 130);
    tft.print(String(actTemperature) + "'C");
}

void temperatureControl() {
  if (temperature - actTemperature > 5 && mode == 1 && state == 1) digitalWrite(H_OUTPUT, HIGH);  
  if (temperature <= actTemperature) {
    digitalWrite(H_OUTPUT, LOW);
    if (mode == 0) {
      state = 0;
      tft.fillRoundRect(22, 104, 75, 14, 0, ST77XX_BLACK);
      tft.setTextColor(ST77XX_ORANGE);
      tft.setTextSize(2);
      tft.setCursor(22, 104);
      tft.print("START");
    }
  }
}
