void setup()
{
  Serial3.begin(9600);  // start serial for output
}

void loop()
{
  while(Serial3.available()) Serial.println(Serial3.read());
}
