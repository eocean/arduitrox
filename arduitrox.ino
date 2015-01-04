//////////////////////////////////////////////////////////////////////////////
// Arduitrox version 1.0
// Main code by Lionel Pawlowski - Copyright (c) 2014
// ADS library by Adafruit
// Running average library by Rob Tillaart
// Commercial use and/or distribution of this code and relevant
// hardware forbidden without prior agreement.
// Provided "as is". Use at your own risk
//////////////////////////////////////////////////////////////////////////////

#include <Wire.h>
#include <LiquidCrystal.h>
#include <Adafruit_ADS1015.h>
#include <RunningAverage.h>

Adafruit_ADS1115 ads;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int opmode=0;
int cnt;
float oldRA;
float sig1,sig,sig2;
float relpourc;
float gain = 0.0078125;
float pour;
RunningAverage RA(10);
int samples=0;

// Définition du gaz de référence
float calibgas=20.95;

void setup(void)
{
// Message d'accueil si besoin (nom, etc)
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print("Arduitrox v1.0");
  lcd.setCursor(0,1);
  lcd.print("");
  delay(3000);
  Serial.begin(9600);
  ads.setGain(GAIN_SIXTEEN); // 16x gain 1 bit = 0.0078125mV
  ads.begin();
  oldRA=0;
  cnt=0;
  lcd.setCursor(0,0);
  lcd.print("Cal. auto");
  lcd.setCursor(0,1);
  lcd.print(" ");
}


void loop(void)
{
  int16_t adc0;
  oldRA=RA.getAverage();
  adc0 = ads.readADC_Differential_0_1();
  RA.addValue(adc0);
  cnt=cnt+1;
  sig=(RA.getAverage()-oldRA)/oldRA;
  sig1=abs(sig);

// Calibration Auto
  if (opmode==0) {
    if (sig1<0.0002) { // 0.0002
      samples=samples+1;
    } else {
    // samples = 0;
    }
    lcd.setCursor(0,1);
    lcd.print(RA.getAverage()*gain,2);
    lcd.print("mV ");
    lcd.print(calibgas,2);
    lcd.print("% ");
    lcd.setCursor(10,0);
    pour=samples/5;
    lcd.print(pour,0);
    lcd.print("% ");
    if ((RA.getAverage()*gain)<0.02) {
      lcd.setCursor(0,0);
      lcd.print("PANNE CELLULE O2");
      opmode=10;
    }
    Serial.print("CAL,");
    Serial.print(cnt);
    Serial.print(",");
    Serial.print(RA.getAverage()*gain,4);
    Serial.print(",");
    Serial.println(calibgas);
    if (samples==500) {
      samples=0;
      opmode=2;
      lcd.setCursor(0,0);
      lcd.print("CALIBR TERMINEE");
      lcd.setCursor(0,0);
      delay(3000);
      relpourc=100*calibgas/RA.getAverage();
      lcd.setCursor(0,0);
      lcd.print(" ");
      lcd.print(" ");
      lcd.begin(16, 2);
      delay(500);
    }
  }

// Lecture des échantillons
  if (opmode==2) {
    lcd.setCursor(0,0);
    lcd.print("%O2: ");
    pour = RA.getAverage()*relpourc/100;
    lcd.print(pour,1);
    lcd.print(" ");
    lcd.setCursor(0,1);
    lcd.print("mV : ");
    lcd.print(RA.getAverage()*gain,2);
    lcd.print(" ");
    lcd.setCursor(10,0);
    if (sig1<0.0002) {
      lcd.print("====");
    } else {
      lcd.print(" ");
    }
    Serial.print("MES,");
    Serial.print(cnt);
    Serial.print(",");
    Serial.print(RA.getAverage()*gain,4);
    Serial.print(",");
    Serial.println(pour,2);
    delay(200);
  }

// Calibration manuelle
  if ((analogRead(0)>600)&&(analogRead(0)<700)) {
    if (opmode<3) {
      opmode=3;
      samples=0;
      lcd.setCursor(0,0);
      lcd.print("Calibr. manuelle");
      lcd.setCursor(0,1);
      lcd.print(" ");
      delay(500);
    } else {
      lcd.setCursor(0,0);
      lcd.print(" ");
      lcd.setCursor(0,1);
      lcd.print(" ");
      delay(2000);
      lcd.setCursor(0,0);
      lcd.print("Cal. auto");
      lcd.setCursor(0,1);
      lcd.print(" ");
      opmode=0;
    }
  }
  if (opmode==3) {
    lcd.setCursor(0,1);
    lcd.print("%O2 REF: ");
    lcd.setCursor(9,1);
    lcd.print(calibgas,2);
    lcd.print(" ");
    if ((analogRead(0)==100)&&(calibgas<99)) {
      calibgas=calibgas+1;
      delay(100);
    }
    if ((analogRead(0)==0)&&(calibgas<100)) {
      calibgas=calibgas+0.05;
      delay(200);
    }
    if ((analogRead(0)==257)&&(calibgas>1)) {
      calibgas=calibgas-1;
      delay(100);
    }
    if ((analogRead(0)==411)&&(calibgas>0.05)) {
      calibgas=calibgas-0.05;
      delay(200);
    }
  }
}
