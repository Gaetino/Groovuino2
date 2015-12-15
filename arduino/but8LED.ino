#include <Wire.h>


//Generic information
unsigned char satellite_type = 0; // 0 : but8led

unsigned char satellite_mode = 1; //0 = Exclusif (un seul bouton allumé à la fois)
	                          //1 = Multi (plusieurs boutons allumés à la fois. Quand on clique sur un bouton déjà allumé, on l'éteint)
		                  //2 = Exclusif temporaire (un seul bouton allumé uniquement quand on appuie dessus)
			          //3 = Mode sauvegarde

int i2c_address = 1;

byte message[] = {0,0,0};

// message : 
//   byte 1 : 1 = state has changed, 0 = no change
//   byte 2 : param number
//   byte 3 : param value


bool send_state = false;

bool ButState[8]= {0,0,0,0,0,0,0,0};
bool LEDState[8]= {0,0,0,0,0,0,0,0};


//Pin connected to ST_CP of 74HC595
int latchPin = 8;
//Pin connected to SH_CP of 74HC595
int clockPin = 6;
////Pin connected to DS of 74HC595
int dataPin = 4;

//holder for infromation you're going to pass to shifting function
byte data = 0; 

int pin_but[8] = {16, 10, 7, 9, 14, 15, 1, 5};
int LED_shiftout[8] = {5, 4, 1, 0, 7, 6, 3, 2};



void setup() {
  //set pins to output because they are addressed in the main loop
  Serial.begin(9600);
  
  Wire.begin(i2c_address);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  for(int i = 0; i<8; i++)
  {
    pinMode(pin_but[i],INPUT_PULLUP);
  }
  pinMode(18,INPUT_PULLUP);
  pinMode(19,INPUT_PULLUP);
  pinMode(20,INPUT_PULLUP);
  pinMode(21,INPUT_PULLUP);

}

void loop() {
  
  
  for(int i=0; i<8 ; i++)
  {
    if(!digitalRead(pin_but[i]) && !ButState[i]) 
    {
	ButPress(i);
	ButState[i] = true;
    }
    if(digitalRead(pin_but[i]) && ButState[i]) 
    {
       ButRelease(i);
       ButState[i] = false;
    }
  }
}

void ButPress(unsigned char numBut)
{ 
  //Serial.println("Butpress");
  switch(satellite_mode)
  {
    case 0:
      digitalWrite(latchPin, 0);
      //red LEDs
      shiftOut(dataPin, clockPin,  1<<LED_shiftout[numBut]);
      //no longer needs to listen for information
      digitalWrite(latchPin, 1);
      message[1] = numBut;
      message[2] = 1;
      send_state = true;
      break;
      
    case 1:
      if(LEDState[numBut]) LEDState[numBut] = false;
      else LEDState[numBut] = true;
      byte data = 0;
      for(int i=0; i<8 ; i++)
      {
        if(LEDState[i]) data = data | (1<<LED_shiftout[i]);
      }
      digitalWrite(latchPin, 0);
      shiftOut(dataPin, clockPin,  data);
      digitalWrite(latchPin, 1);
      message[1] = numBut;
      message[2] = LEDState[numBut];
      send_state = true;
      break;
  }
}

void ButRelease(unsigned char numBut)
{
//  Serial.print("but released : ");
//  Serial.println(numBut);
  switch(satellite_mode)
  {
  
  case 0:
	break;
  }
}
 
void shiftOut(int myDataPin, int myClockPin, byte myDataOut) {
  // This shifts 8 bits out MSB first, 
  //on the rising edge of the clock,
  //clock idles low

  //internal function setup
  int i=0;
  int pinState;
  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, OUTPUT);

  //clear everything out just in case to
  //prepare shift register for bit shifting
  digitalWrite(myDataPin, 0);
  digitalWrite(myClockPin, 0);

  //for each bit in the byte myDataOut�
  //NOTICE THAT WE ARE COUNTING DOWN in our for loop
  //This means that %00000001 or "1" will go through such
  //that it will be pin Q0 that lights. 
  for (i=7; i>=0; i--)  {
    digitalWrite(myClockPin, 0);

    //if the value passed to myDataOut and a bitmask result 
    // true then... so if we are at i=6 and our value is
    // %11010100 it would the code compares it to %01000000 
    // and proceeds to set pinState to 1.
    if ( myDataOut & (1<<i) ) {
      pinState= 1;
    }
    else {	
      pinState= 0;
    }

    //Sets the pin to HIGH or LOW depending on pinState
    digitalWrite(myDataPin, pinState);
    //register shifts bits on upstroke of clock pin  
    digitalWrite(myClockPin, 1);
    //zero the data pin after shift to prevent bleed through
    digitalWrite(myDataPin, 0);
  }

  //stop shifting
  digitalWrite(myClockPin, 0);
}

void requestEvent()
{
  if(send_state)
  {
    message[0] = 1;
    Wire.write(message,3);
    send_state = false;
  }
  else
  {
    message[0] = 0;
    Wire.write(message,3);
  }
}


void receiveEvent(int howMany)
{
  if(Wire.read()==255) satellite_mode = Wire.read()-1;    // receive byte as an integer
}
