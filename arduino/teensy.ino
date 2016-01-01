
int but8LED_num = 2;
int but8LED_address[8] = {0,2,0,0,0,0,0,0};
int but8LED_mode[8] = {0,0,0,0,0,0,0,0};

void setup()
{
  Serial3.begin(9600);  // start serial for output
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
  Serial3.write(255);
  Serial3.write(0);
  for(int i = 0; i<but8LED_num; i++)
  {
    Serial3.write(0);
    Serial3.write(but8LED_address[i]);
    Serial3.write(but8LED_mode[i]);
  }
}
