#include <Wire.h>
#include <SPI.h>
#include "LCD_Functions.h"

int nDevices = 0;
int lignes = 3;
const int pin_button = 14;
const int pin_A = 8;  // pin 12
const int pin_B = 4;  // pin 11
unsigned char encoder_A;
unsigned char encoder_B;
unsigned char encoder_A_prev=0;
boolean butstate = false;
int numscreen = 0; // 0 = menu principal
                   // 1 = liste de modules dans "set module"

int but8LED_address = 2;
int but8LED_mode = 2;

int menu = 1;

boolean config_ok = false;
boolean config_loaded = false;
boolean refresh_menu = false;

boolean module_ok[8] = {1,1,1,1,1,1,1,1};

void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial1.begin(9600);  // start serial for output
  Serial.begin(9600);
  
  pinMode(9, OUTPUT);
  pinMode(pin_A, INPUT_PULLUP);
  pinMode(pin_B, INPUT_PULLUP);
  pinMode(pin_button, INPUT);
  
  lcdBegin(); // This will setup our pins, and initialize the LCD
  updateDisplay(); // with displayMap untouched, SFE logo
  setContrast(60); // Good values range from 40-60
  delay(3000);
  
  main_menu();
}

void upload_modules()
{
  Serial.println("----upload_modules----");
  clearDisplay(WHITE);
  updateDisplay();
  setStr("UP MODULES",0,0,BLACK);
  updateDisplay();
  delay(2000);
  
  write_module(but8LED_address,but8LED_mode);
}


void main_menu()
{
  clearDisplay(WHITE);
  updateDisplay();
  setStr("KIT 1", 2, 2, BLACK);
  setStr("KIT 2", 2, 17, BLACK);
  setStr("KIT 3", 2, 32, BLACK);
  setRect(0, 0, 80, 12, 0, BLACK);
  updateDisplay();
}

void select_menu(int sens)
{
  if(sens==1 && menu<lignes)
  {
    menu++;
    if(menu==2) 
    {
      setRect(0, 0, 80, 12, 0, WHITE);
      setRect(0, 15, 80, 27, 0, BLACK);
      updateDisplay();
    }
    if(menu==3) 
    {
      setRect(0, 15, 80, 27, 0, WHITE);
      setRect(0, 30, 80, 42, 0, BLACK);
      updateDisplay();
    }
  }
  if(sens==-1 && menu>1)
  {
    menu--;
    if(menu==2) 
    {
      setRect(0, 30, 80, 42, 0, WHITE);
      setRect(0, 15, 80, 27, 0, BLACK);
      updateDisplay();
    }
    if(menu==1) 
    {
      setRect(0, 15, 80, 27, 0, WHITE);
      setRect(0, 0, 80, 12, 0, BLACK);
      updateDisplay();
    }
  }
}

void write_module(byte module_num, byte mode_num)
{
  Serial.println("----write_module----");
  Serial.print("adresse : ");
  Serial.println(module_num);
  Wire.beginTransmission(module_num);
  Wire.write(255);
  Wire.write(0);
  Serial.print("mode : ");
  Serial.println(mode_num);
  Wire.write(mode_num);
  Wire.endTransmission();
}


boolean modules_valides()
{
  boolean ret = 1;
  ret = module_ok[0];  
  return ret;
}

void listen_modules()
{
  Wire.requestFrom(but8LED_address, 3);    
  delay(2);
  
  byte changed = 0;
  byte param_number = 0;
  byte param_value = 0; 
  
  if(Wire.available())    // slave may send less than requested
  { 
    module_ok[0] = true;
    changed = Wire.read();
    param_number = Wire.read();
    param_value = Wire.read();
    if(changed > 0) 
    {
      Serial1.write(255);
      Serial1.write(1);
      Serial1.write(0);
      Serial1.write(param_number);
      Serial1.write(param_value);
    }
    delay(1);
  }
  else
  {
    Serial.println("not responding");
    module_ok[0] = false;
    clearDisplay(WHITE);
    updateDisplay();
    setStr("MODULES KO",0,0,BLACK);
    updateDisplay();
  } 
  
  delay(2);
}

void scan_modules()
{
  Serial.println("----scan modules----");
  int ret = true;

  Wire.beginTransmission(but8LED_address);
  int error = Wire.endTransmission();
  if (error==0) module_ok[0] = true;
  else  module_ok[0] = false;
  delay(1);  
  
  if(modules_valides())
  {
    clearDisplay(WHITE);
    updateDisplay();
    setStr("MODULES OK",0,0,BLACK);
    updateDisplay();
    delay(2000);
    upload_modules();
    main_menu();
  }
  delay(500);
}

void loop()
{
  if(refresh_menu) {upload_modules(); main_menu(); refresh_menu=false;}

  if(modules_valides()) listen_modules();
  else scan_modules();

  int rot = ReadRotary();
  if(rot!=0) select_menu(rot);
  if(butpressed())
  {
    switch(menu)
    {
    case 3: 
      Serial1.write(255);
      Serial1.write(1);
      Serial1.write(1);
      Serial1.write(3);
      Serial1.write(1);
      break;
    case 2:
      Serial1.write(255);
      Serial1.write(1);
      Serial1.write(1);
      Serial1.write(2);
      Serial1.write(1);
      break;
    case 1:
      Serial1.write(255);
      Serial1.write(1);
      Serial1.write(1);
      Serial1.write(1);
      Serial1.write(1);
      break;
    }
  }
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

boolean butpressed()
{
  if(digitalRead(pin_button) && !butstate)
  {
    Serial.println("pressed");
    butstate = true;
    return true;
  }
  if(!digitalRead(pin_button) && butstate) butstate = false;
  return false;
}
