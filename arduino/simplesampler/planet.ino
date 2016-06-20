#include <Wire.h>

int value[4] = {0,0,0,0};
int states[4] = {0,0,0,0};

boolean state_send[4] = {0,0,0,0};

//Generic information
unsigned char satellite_type = 1; // 0 : but8led
                                  // 1 : 4enco

unsigned char satellite_mode = 0; 

int i2c_address ;

byte message[] = {0,0,0};

// message : 
//   byte 1 : 1 = state has changed, 0 = no change
//   byte 2 : param number
//   byte 3 : param value


bool send_state = false;

int pin_selector[4] = {A3,A2,A1,A0};

void setup() {
  Serial.begin(9600);
  
  for(int i = 0; i<4; i++)
  {
    pinMode(pin_selector[i],INPUT_PULLUP);
  }

  i2c_address = (1-digitalRead(pin_selector[0])) + ((1-digitalRead(pin_selector[1]))<<1) + ((1-digitalRead(pin_selector[2]))<<2) + ((1-digitalRead(pin_selector[3]))<<3);

  Wire.begin(i2c_address);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  
  pinMode(0,INPUT_PULLUP);
  pinMode(1,INPUT_PULLUP);
  pinMode(2,INPUT_PULLUP);
  pinMode(3,INPUT_PULLUP);
  pinMode(14,INPUT_PULLUP);
  pinMode(15,INPUT_PULLUP);
  pinMode(16,INPUT_PULLUP);
  pinMode(8,INPUT_PULLUP);
  pinMode(9,INPUT_PULLUP);
  pinMode(10,INPUT_PULLUP);
  
    
}

void loop() {
  
  if(Serial.available())
  {
    Serial.read();
    Serial.print("adresse : ");
    Serial.println(i2c_address);
    Serial.print("mode : ");
    Serial.println(satellite_mode);
    
  }
  
  read_encoder();
  read_encoder2();
  value[0] -= states[0];
  if(states[0] != 0) {Serial.println(value[0]); state_send[0]=true;}
  value[1] -= states[1];
  if(states[1] != 0) {Serial.println(value[1]); state_send[1]=true;}
  value[2] -= states[2];
  if(states[2] != 0) {Serial.println(value[2]); state_send[2]=true;}
  value[3] += states[3];
  if(states[3] != 0) {Serial.println(value[3]); state_send[3]=true;}
 
}


void read_encoder()
{
  int enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
  static uint8_t old_AB = 0;
  /**/
  old_AB <<= 2;                   //remember previous state
  char read_pin = PIND;
  old_AB |= ( (read_pin & 0x0C)>>2 );  //add current state
  states[0] = ( enc_states[( old_AB & 0x0f )]);
}

void read_encoder2()
{
  int enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
  static uint8_t old_AB = 0;
  /**/
  old_AB <<= 2;                   //remember previous state
  char read_pin = PINB;
  old_AB |= ( (read_pin & 0x06)>>1 );  //add current state
  states[1] = ( enc_states[( old_AB & 0x0f )]);
  
  static uint8_t old_AB2 = 0;
  old_AB2 <<= 2;                   //remember previous state
  old_AB2 |= ( (read_pin & 0x18)>>3 );  //add current state
  states[2] = ( enc_states[( old_AB2 & 0x0f )]);
  
  static uint8_t old_AB3 = 0;
  old_AB3 <<= 2;                   //remember previous state
  old_AB3 |= ( (read_pin & 0x60)>>5 );  //add current state
  states[3] = ( enc_states[( old_AB3 & 0x0f )]);
}


void requestEvent()
{
  message[0] = 0;
  for(int i=0; i<4; i++)
  {
    if(state_send[i])
    {
      Serial.println("send");
      message[0] = 1;
      message[1] = i;
      message[2] = value[i];
      
      state_send[i] = false;
    }
  }
  Wire.write(message,3);
  Serial.println(message[0]);
}


void receiveEvent(int howMany)
{
  if(Wire.read()==255) 
  {
    //Serial.print("received : ");
    int input = Wire.read();
    //Serial. println(input);
    if(input==0) 
    {
      satellite_mode = Wire.read();    // receive byte as an integer
      //Serial.print("mode : ");
      //Serial.println(satellite_mode);
    }
  }
}
