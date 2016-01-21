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

int but8LED_number = 0;
int but8LED_address[8] = {0,0,0,0,0,0,0,0};
int but8LED_mode[8] = {0,0,0,0,0,0,0,0};

boolean multi_active;
int multi_type;
int line_up_mode, line_down_mode;
int line_up_lighted, line_down_lighted;
int module_lighted_up, param_lighted_up;
int module_lighted_down, param_lighted_down;

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

  load_config(); 
  
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
  
  for(int i=0; i<but8LED_number; i++)
  {
    write_module(but8LED_address[i],but8LED_mode[i]);
    if(multi_active)
    {
      Serial.println("multi : reset");
      Wire.beginTransmission(but8LED_address[i]);
      Wire.write(255);
      Wire.write(2);
      Wire.endTransmission();
    }
  }
  if(multi_active)
  {
    Serial.println("multi : init");
    Wire.beginTransmission(but8LED_address[0]);
    Wire.write(255);
    Wire.write(1);
    Wire.write(0);
    Wire.endTransmission();
  }
  
}


boolean listen_config()
{
  Serial.println("----listen config----");
  boolean ret = false;
  
  while(Serial1.available())
  {
    int bibi = Serial1.read();
    Serial.println(bibi);
    if(bibi==255)
    {
      delay(10);
      int bibi3 = Serial1.read();
      
      if(bibi3==0)
      {
        Serial.println("config simple");
        int i = 0;
        delay(50);
        clearDisplay(WHITE);
        updateDisplay();
        setStr("LOAD CONFIG",0,0,BLACK);
        updateDisplay();
        refresh_menu=true;

        while(Serial1.available())
        {
          delay(10);
          int bibi2 = Serial1.read();
          Serial.print("received : ");
          Serial.println(bibi2);
          if(bibi2==0)
          {
            delay(10);
            but8LED_address[i] = Serial1.read();
            Serial.print("add : ");
            Serial.println(but8LED_address[i]);
            delay(10);
            but8LED_mode[i] = Serial1.read();
            Serial.print("mode : ");
            Serial.println(but8LED_mode[i]);
            i++;
          }
        }
        delay(1000);
        but8LED_number = i;
        ret = true;
      }
      if(bibi3==1)
      {
        Serial.println("config multi");
        multi_active = true;
        int nb_modules = Serial1.read();
        but8LED_number = nb_modules;
        multi_type = Serial1.read();
        if(multi_type==0) {line_up_mode = Serial1.read(); line_down_mode = Serial1.read();}
        Serial.print("nb modules : ");
        Serial.println(nb_modules);
        Serial.print("multi_type : ");
        Serial.println(multi_type);
        Serial.print("line_up_mode : ");
        Serial.println(line_up_mode);
        Serial.print("line_down_mode : ");
        Serial.println(line_down_mode);
        
        refresh_menu=true;
        
        for(int i = 0; i<nb_modules; i++)
        {
          but8LED_address[i] = Serial1.read();
          but8LED_mode[i] = 3;
          Serial.print("module ");
          Serial.println(i);
          Serial.print("but8LED_address : ");
          Serial.println(but8LED_address[i]);
          Serial.print("but8LED_mode : ");
          Serial.println(but8LED_mode[i]);
          
        }
        ret = true;
      }
    }
  }
  return ret;
}

void load_config()
{
  Serial.println("----load config----");
  clearDisplay(WHITE);
  updateDisplay();
  setStr("LOAD CONFIG",0,0,BLACK);
  updateDisplay();
  
  Serial1.write(255);
  Serial1.write(0);
  
  delay(100);
   
  if(!Serial1.available())
  {
    clearDisplay(WHITE);
    updateDisplay();
    setStr("MAINBOARD",0,0,BLACK);
    setStr("   NOT",0,15,BLACK);
    setStr("RESPONDING",0,30,BLACK);
    updateDisplay();
    delay(2000);
    return;
  }
  
  boolean conf = listen_config();
  
  if(conf) 
  {
    clearDisplay(WHITE);
    updateDisplay();
    setStr("LOAD OK",0,0,BLACK);
    updateDisplay();
  }
  else
  {
    clearDisplay(WHITE);
    updateDisplay();
    setStr("LOAD KO",0,0,BLACK);
    updateDisplay();
  }
  
  config_loaded = true;
  delay(2000);
}

void main_menu()
{
  clearDisplay(WHITE);
  updateDisplay();
  setStr("LOAD CONFIG", 2, 2, BLACK);
  setStr("SET MODULE", 2, 17, BLACK);
  setStr("PLAY", 2, 32, BLACK);
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
  if(but8LED_number > 0) setStr("MODULE 1", 2, 17, BLACK);
  if(but8LED_number > 1) setStr("MODULE 2", 2, 32, BLACK);
  if(but8LED_number > 0) setRect(0, 0, 80, 12, 0, BLACK);
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
  for(int i = 0; i<but8LED_number; i++)
  {
   ret *= module_ok[i];  
  }
  return ret;
}

void multi_param_up()
{
    Serial.println("----send param----");
    Serial.println(param_lighted_up);
    Wire.beginTransmission(but8LED_address[module_lighted_up]);
    Wire.write(255);
    Wire.write(1);
    Wire.write(param_lighted_up);
    Wire.endTransmission();
}

void multi_param_down()
{
    Serial.println("----send param----");
    Serial.println(param_lighted_down);
    Wire.beginTransmission(but8LED_address[module_lighted_down]);
    Wire.write(255);
    Wire.write(1);
    Wire.write(param_lighted_down);
    Wire.endTransmission();
}

void transco_multi(int num, boolean line_choice, int num_module)
{
  Serial.println("----transco_multi----");
  // line up
  Serial.print("num : ");
  Serial.println(num);
  Serial.print("num dans module : ");
  Serial.println(num%4);
  Serial.print("line choice : ");
  Serial.println(line_choice);
  if(line_choice)  {param_lighted_up = num%4; module_lighted_up = num_module;}
  // line down
  else {param_lighted_down = 4 + num%4; module_lighted_down = num_module;}
  
}

void listen_modules()
{
  for(int i = 0; i<but8LED_number; i++)
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
          /*Serial.println("----received module----");
          Serial.print("param number : ");
          Serial.println(param_number);
          Serial.print("param value : ");
          Serial.println(param_value);*/
          if(multi_active)
          {
            if(param_number<4)
            {
              Serial.println("line up pressed");
              if(line_up_mode==0)
              {
                Serial.print("param lighted up : ");
                Serial.println(param_lighted_up + module_lighted_up*4);
                Serial.print("param number : ");
                Serial.println(param_number+i*4);
                if((param_lighted_up+module_lighted_up*4)!=(param_number+i*4)) 
                {
                  multi_param_up();
                  transco_multi(param_number+i*4, 1, i);
                  multi_param_up();
                }
              }
              else
              {
                transco_multi(param_number+i*4, 1, i);
                multi_param_up();
              }
            }
            else
            {
              Serial.println("line up pressed");
              if(line_down_mode==0)
              {
                Serial.print("param lighted down : ");
                Serial.println(param_lighted_down + module_lighted_down*4);
                Serial.print("param number : ");
                Serial.println(param_number+i*4);
                if((param_lighted_down+module_lighted_down*4)!=(param_number+i*4)) 
                {
                  multi_param_down();
                  transco_multi(param_number+i*4, 0, i);
                  multi_param_down();
                }
              }
              else
              {
                transco_multi(param_number+i*4, 0, i);
                multi_param_down();
              }
            }
          }
          else
          {
            Serial1.write(255);
            Serial1.write(1);
            Serial1.write(i);
            Serial1.write(param_number);
            Serial1.write(param_value);
          }
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
  for(int i=0; i<but8LED_number; i++)
  {
    Serial.println(i);
    Wire.beginTransmission(but8LED_address[i]);
    int error = Wire.endTransmission();
    //Serial.print("error : ");
    //Serial.println(error);
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
  if(config_loaded)
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
      if(numscreen==0)
      {
        main_menu();
      }
      if(numscreen==2)
      {
        write_module(but8LED_address[0],1);
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
        write_module(but8LED_address[0],0);
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
        lignes = but8LED_number+1;
        list_module();
      }
      break;
    case 1:
      if(numscreen==0)
      {
        load_config();
        main_menu();
      }
      if(numscreen==1)
      {
        menu = 1;
        numscreen = 0;
        lignes = 3;
        main_menu();
      }
      if(numscreen==2)
      {
        menu = 1;
        numscreen = 1;
        lignes = but8LED_number+1;
        list_module();
      }
      break;
    }
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
