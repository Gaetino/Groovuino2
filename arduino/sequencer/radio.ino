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

int but8LED_address[2] = {0,2};
int but8LED_mode[2] = {0,0};

int sample_num = 0;
boolean sequence[8][8];

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
  
  for(int i = 0; i<8; i++)
  {
    for(int j = 0; j<8; j++)
    {
      sequence[i][j] = false;
    }
  }

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
  
  for(int i=0; i<2; i++)
  {
    write_module(but8LED_address[i],but8LED_mode[i]);
    Serial.println("multi : reset");
    Wire.beginTransmission(but8LED_address[i]);
    Wire.write(255);
    Wire.write(2);
    Wire.endTransmission();
  }

  Serial.println("multi : init");
  Wire.beginTransmission(but8LED_address[0]);
  Wire.write(255);
  Wire.write(1);
  Wire.write(0);
  Wire.endTransmission();
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
  for(int i = 0; i<2; i++)
  {
   ret *= module_ok[i];  
  }
  return ret;
}

void write_param(byte module_num, byte param_num)
{
  Serial.println("----send param----");
  Serial.println(param_num);
  Wire.beginTransmission(but8LED_address[module_num]);
  Wire.write(255);
  Wire.write(1);
  Wire.write(param_num);
  Wire.endTransmission();
}


void send_line()
{
  for(int i=0; i<8; i++)
  {
    if(sequence[sample_num][i]) send_value(0, i);
  }
}

void send_value(boolean line_choice, int module_value)
{
  // line up
  if(line_choice)
  {
    if(module_value > 3) write_param(1,module_value-4);
    else write_param(0,module_value);
  }
  
  // line down
  else
  {
    if(module_value > 3) write_param(1,module_value);
    else write_param(0,module_value+4);
  }
}

void clear_module(byte module_num)
{
  Serial.println("----clear_module----");
  Serial.print("adresse : ");
  Serial.println(module_num);
  Wire.beginTransmission(but8LED_address[module_num]);
  Wire.write(255);
  Wire.write(2);
  Wire.endTransmission();
}

void set_param(int module_num, byte param_number, byte param_value)
{
  Serial.println("----received module----");
  Serial.print("param number : ");
  Serial.println(param_number);
  Serial.print("param value : ");
  Serial.println(param_value);
  
//LINE UP
  if(param_number<4)
  {
    int param_pressed = param_number+module_num*4;
    
    if(sample_num != param_pressed)
    {    
      //send_value(1,sample_num);
      sample_num = param_pressed;
      Serial.println("line up pressed");
      Serial1.write(255);
      Serial1.write(2);
      Serial1.write(0);
      Serial1.write(param_pressed);
      Serial1.write(1);
    }
  }
  
// LINE DOWN
  else
  {
    int param_pressed = param_number+module_num*4-4;
    Serial.println("line down pressed");
    
   
    if(!sequence[sample_num][param_pressed])
    {
      sequence[sample_num][param_pressed] = true;
    }
    else
    {
      sequence[sample_num][param_pressed] = false;
    }
    Serial1.write(255);
    Serial1.write(2);
    Serial1.write(1);
    Serial1.write(param_pressed);
    Serial1.write(sequence[sample_num][param_pressed]);
    //clear_module(0);
    //clear_module(1);
    //send_line();
  }
  update_modules();
}

void update_modules()
{
  clear_module(0);
  clear_module(1);
  send_value(1,sample_num);
  send_line();
}

void listen_modules()
{
  for(int i = 0; i<2; i++)
  {
    Wire.requestFrom(but8LED_address[i], 3);    
    delay(3);
    
    byte changed = 0;
    byte param_number = 0;
    byte param_value = 0; 
    
    if(Wire.available())    // slave may send less than requested
    { 
      module_ok[i] = true;
      changed = Wire.read();
      param_number = Wire.read();
      param_value = Wire.read();
      if(changed > 0) 
      {
        set_param(i, param_number, param_value);
      }
      delay(2);
    }
    else
    {
      Serial.println("not responding");
      module_ok[i] = false;
      clearDisplay(WHITE);
      updateDisplay();
      setStr("MODULES KO",0,0,BLACK);
      updateDisplay();
    } 
  }
  delay(2);
}

void scan_modules()
{
  Serial.println("----scan modules----");
  int ret = true;
  for(int i=0; i<2; i++)
  {
    Wire.beginTransmission(but8LED_address[i]);
    int error = Wire.endTransmission();
    if (error==0) module_ok[i] = true;
    else  module_ok[i] = false;
    delay(1); 
  }  
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
        break;
      case 2:
        break;
      case 1:
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
