#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=148,164
AudioSynthWaveform       waveform3;      //xy=150,274
AudioSynthWaveform       waveform2;      //xy=153,218
AudioSynthNoiseWhite     noise1;         //xy=154,330
AudioMixer4              mixer1;         //xy=335,238
AudioEffectEnvelope      envelope1;      //xy=492,233
AudioOutputAnalog        dac1;           //xy=654,227
AudioConnection          patchCord1(waveform1, 0, mixer1, 0);
AudioConnection          patchCord2(waveform3, 0, mixer1, 2);
AudioConnection          patchCord3(waveform2, 0, mixer1, 1);
AudioConnection          patchCord4(noise1, 0, mixer1, 3);
AudioConnection          patchCord5(mixer1, envelope1);
AudioConnection          patchCord6(envelope1, dac1);
// GUItool: end automatically generated code

AudioControlSGTL5000 audioShield;

void setup() 
{
  Serial.begin(9600);
  Serial3.begin(9600);
  
  delay(2000);
  send_config();

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(25);
	
	 // turn on the output
  audioShield.enable();
  audioShield.volume(0.5);

  // by default the Teensy 3.1 DAC uses 3.3Vp-p output
  // if your 3.3V power has noise, switching to the
  // internal 1.2V reference can give you a clean signal
  //dac.analogReference(INTERNAL);
	
	  // reduce the gain on mixer channels, so more than 1
  // sound can play simultaneously without clipping
  mix1.gain(0, 0.4);
  mix1.gain(1, 0.4);
  mix1.gain(2, 0.4);
  mix1.gain(3, 0.4);
}


void loop() 
{
  while(Serial3.available()>4)
  {
    if(Serial3.read()==255)
    {   
      delay(5);
      int comm = Serial3.read();
      if(comm==0)
      {
        Serial.println("transmit");
        send_config();
      }
      if(comm==1)
      {
        unsigned char module_number = Serial3.read();
        unsigned char param_number = Serial3.read();
        unsigned char param_value = Serial3.read();

        /*Serial.print("module num ");
        Serial.print(module_number);
        Serial.print(" - param num ");
        Serial.print(param_number);
        Serial.print(" - val ");
        Serial.println(param_value);*/
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
