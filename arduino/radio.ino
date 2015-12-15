#include <Wire.h>
#include <SPI.h>
#include "LCD_Functions.h"

const int pin_A = 8;  // pin 12
const int pin_B = 4;  // pin 11
unsigned char encoder_A;
unsigned char encoder_B;
unsigned char encoder_A_prev=0;

int i2c_address_but8LED[8] = {0,0,0,0,0,0,0,0};

int mode = 1;

void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial1.begin(9600);  // start serial for output
  
  pinMode(9, OUTPUT);
  pinMode(pin_A, INPUT_PULLUP);
  pinMode(pin_B, INPUT_PULLUP);
  
  lcdBegin(); // This will setup our pins, and initialize the LCD
  updateDisplay(); // with displayMap untouched, SFE logo
  setContrast(60); // Good values range from 40-60
  delay(2000);
  
  clearDisplay(WHITE);
  updateDisplay();
  setStr("MODE 1", 2, 2, BLACK);
  setStr("MODE 2", 2, 17, BLACK);
  setStr("MODE 3", 2, 32, BLACK);
  setRect(0, 0, 40, 12, 0, BLACK);
  updateDisplay();
}

void scan_i2c()
{
  
for(address = 1; address < 16; address++ ) 
{
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      clearDisplay(WHITE);
      updateDisplay();
      setStr("found module ",0,0,BLACK);
      setStr(address,10,10,BLACK);
      nDevices++;
    }
    else if (error==4) 
    {
      Serial.print("Unknow error at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}

void select_mode(int sens)
{
  if(sens==1 && mode<3)
  {
    mode++;
    if(mode==2) 
    {
      setRect(0, 0, 40, 12, 0, WHITE);
      setRect(0, 15, 40, 27, 0, BLACK);
      updateDisplay();
    }
    if(mode==3) 
    {
      setRect(0, 15, 40, 27, 0, WHITE);
      setRect(0, 30, 40, 42, 0, BLACK);
      updateDisplay();
    }
    Wire.beginTransmission(1);
    Wire.write(mode);
    Wire.endTransmission();
  }
  if(sens==-1 && mode>1)
  {
    mode--;
    if(mode==2) 
    {
      setRect(0, 30, 40, 42, 0, WHITE);
      setRect(0, 15, 40, 27, 0, BLACK);
      updateDisplay();
    }
    if(mode==1) 
    {
      setRect(0, 15, 40, 27, 0, WHITE);
      setRect(0, 0, 40, 12, 0, BLACK);
      updateDisplay();
    }
    Wire.beginTransmission(1);
    Wire.write(mode);
    Wire.endTransmission();
  }
}

void loop()
{
  Wire.requestFrom(1, 3);    
  delay(2);
  
  byte changed = 0;
  byte param_number = 0;
  byte param_value = 0; 
  
  while(Wire.available())    // slave may send less than requested
  { 
    changed = Wire.read();
    param_number = Wire.read();
    param_value = Wire.read();
  }
  
  if(changed > 0) 
  {
    Serial1.write(255);
    Serial1.write()
    Serial1.write(param_number);
    Serial1.write(param_value);
  }
  
  delay(1);
  
  int rot = ReadRotary();
  if(rot!=0) select_mode(rot);
  
}



int ReadRotary()
{
  int ret = 0;
  encoder_A = digitalRead(pin_A);    // Read encoder pins
  encoder_B = digitalRead(pin_B);
  if((!encoder_A) && (encoder_A_prev)){
      // A has gone from high to low 
      if(encoder_B) ret = 1;
      else ret = -1;
  }
  encoder_A_prev = encoder_A;
  return ret;
}
