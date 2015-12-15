#include <Wire.h>
#include <SPI.h>
#include "LCD_Functions.h"

int lignes = 3;
int nDevices = 0;
const int pin_button = 14;
const int pin_A = 8;  // pin 12
const int pin_B = 4;  // pin 11
unsigned char encoder_A;
unsigned char encoder_B;
unsigned char encoder_A_prev=0;
boolean butstate = false;
int numscreen = 0; // 0 = menu principal
                   // 1 = liste de modules dans "set module"

int i2c_address_but8LED[8] = {0,0,0,0,0,0,0,0};

int menu = 1;

void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial1.begin(9600);  // start serial for output
  
  pinMode(9, OUTPUT);
  pinMode(pin_A, INPUT_PULLUP);
  pinMode(pin_B, INPUT_PULLUP);
  pinMode(pin_button, INPUT);
  
  lcdBegin(); // This will setup our pins, and initialize the LCD
  updateDisplay(); // with displayMap untouched, SFE logo
  setContrast(60); // Good values range from 40-60
  delay(1000);
  
  clearDisplay(WHITE);
  updateDisplay();
  setStr("START SCAN...",0,0,BLACK);
  updateDisplay();
  scan_i2c();
  clearDisplay(WHITE);
  setStr("END SCAN...",0,0,BLACK);
  updateDisplay();
  delay(1000);
  
  clearDisplay(WHITE);
  main_menu();
}

void main_menu()
{
  clearDisplay(WHITE);
  updateDisplay();
  setStr("LOAD CONFIG", 2, 2, BLACK);
  setStr("SET MODULE", 2, 17, BLACK);
  setStr("SCAN MODULES", 2, 32, BLACK);
  setRect(0, 0, 80, 12, 0, BLACK);
  updateDisplay();
}

void mode_select()
{
  clearDisplay(WHITE);
  updateDisplay();
  setStr("RETOUR", 2, 2, BLACK);
  setStr("MODE 1", 2, 17, BLACK);
  setStr("MODE 2", 2, 32, BLACK);
  setRect(0, 0, 80, 12, 0, BLACK);
  updateDisplay();
}

void list_module()
{
  clearDisplay(WHITE);
  updateDisplay();
  setStr("RETOUR", 2, 2, BLACK);
  if(nDevices > 0) setStr("MODULE 1", 2, 17, BLACK);
  if(nDevices > 1) setStr("MODULE 2", 2, 32, BLACK);
  if(nDevices > 0) setRect(0, 0, 80, 12, 0, BLACK);
  setRect(0, 0, 80, 12, 0, BLACK);
  updateDisplay();
}

void scan_i2c()
{
  
  byte error, address;
  for(address = 1; address < 16; address++ ) 
  {
    clearDisplay(WHITE);
    updateDisplay();
    setStr("MODULE",0,0,BLACK);
    setChar(48+address,10,10,BLACK);
    updateDisplay();
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      i2c_address_but8LED[nDevices] = address;
      clearDisplay(WHITE);
      updateDisplay();
      setStr("FOUND MODULE ",0,0,BLACK);
      setChar(48+address,10,10,BLACK);
      updateDisplay();
      nDevices++;
      delay(2000);
    }
    else if (error==4) 
    {
      clearDisplay(WHITE);
      updateDisplay();
      setStr("ERROR MODULE ",0,0,BLACK);
      setChar(48+address,10,10,BLACK);
      updateDisplay();
      delay(2000);
    }   
    delay(200); 
  }
  if (nDevices == 0)
  {
    clearDisplay(WHITE);
    updateDisplay();
    setStr("NO MODULE",0,0,BLACK);
    updateDisplay();
    delay(1000);
  }
}

void update_menu(int mode)
{
  Wire.beginTransmission(1);
  Wire.write(mode);
  Wire.endTransmission();
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

void loop()
{
  for(int i = 0; i<nDevices; i++)
  {
    Wire.requestFrom(i2c_address_but8LED[i], 3);    
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
      Serial1.write(i2c_address_but8LED[i]);
      Serial1.write(param_number);
      Serial1.write(param_value);
    }
    delay(1);
  }
  
  int rot = ReadRotary();
  if(rot!=0) select_menu(rot);
  if(butpressed())
  {
    switch(menu)
    {
    case 3: 
      if(numscreen==0)
      {
        scan_i2c(); 
        lignes = 3;
        main_menu();
      }
      if(numscreen==2)
      {
        Wire.beginTransmission(1);
        Wire.write(255);
        Wire.write(2);
        Wire.endTransmission();
      }
      if(numscreen==1)
      {
        lignes = 3;
        menu = 1;
        numscreen = 2;
        mode_select();
      }
      break;
    case 2:
      if(numscreen==2)
      {
        Wire.beginTransmission(1);
        Wire.write(255);
        Wire.write(1);
        Wire.endTransmission();
      }
      if(numscreen==1)
      {
        lignes = 3;
        menu = 1;
        numscreen = 2;
        mode_select();
      }
      if(numscreen==0)
      {
        menu = 1;
        numscreen = 1;
        lignes = nDevices+1;
        list_module();
      }
      
      break;
    case 1:
      if(numscreen==1)
      {
        menu = 1;
        numscreen = 0;
        lignes = 3;
        main_menu();
      }
      break;
      if(numscreen==2)
      {
        menu = 1;
        numscreen = 1;
        lignes = nDevices+1;
        list_module();
      }
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
