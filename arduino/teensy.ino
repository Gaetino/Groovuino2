
int but8LED_num = 2;
int but8LED_address[8] = {0,2,0,0,0,0,0,0};
int but8LED_mode[8] = {0,0,0,0,0,0,0,0};

boolean multi = true;

//DONNEES DU MODE MULTI
int module_number = 2;
int multi_mode = 0; // 0 : ligne
                    // 1 : colonne
int line_up_mode = 0; // 0 : Exclusif
                      // 1 : on/off
int line_down_mode = 1;

int col_mode[4] = {0,0,0,0};

int multi_address[8] = {0,2,0,0,0,0,0,0};

void setup()
{
  Serial3.begin(9600);  // start serial for output
  delay(2000);
  send_config();
}

void loop()
{
  while(Serial3.available()) 
  {
    if(Serial3.read()==255)
    {
      delay(5);
      int test = Serial3.read();
      if(test==0)
      {
        Serial.println("transmit");
        send_config();
      }
      if(test==1)
      {
        Serial.println("param receive");
        Serial.println(Serial3.read());
        Serial.println(Serial3.read());
        Serial.println(Serial3.read());
      }
    }
  }
}

void send_config()
{
  if(!multi)
  {
    Serial3.write(255);
    Serial3.write(0);
    for(int i = 0; i<but8LED_num; i++)
    {
      Serial3.write(0);
      Serial3.write(but8LED_address[i]);
      Serial3.write(but8LED_mode[i]);
    }
  }
  else
  {
    Serial3.write(255);
    Serial3.write(1);
    if(multi_mode==0)
    {
      Serial.write(0);
      Serial.write(line_up_mode);
      Serial.write(line_down_mode);
    }
    if(multi_mode==1)
    {
      Serial.write(1);
      for(int i=0; i<4; i++)
      {
        Serial.write(col_mode[i]);
      }
    }
    for(int i=0; i<module_number; i++)
    {
      Serial.write(multi_address[i]);
    }
    Serial.write(255);
  }
}
