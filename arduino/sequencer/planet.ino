#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// WAV files converted to code by wav2sketch
#include "AudioSampleSnare.h"        // http://www.freesound.org/people/KEVOY/sounds/82583/
#include "AudioSampleTomtom.h"       // http://www.freesound.org/people/zgump/sounds/86334/
#include "AudioSampleHihat.h"        // http://www.freesound.org/people/mhc/sounds/102790/
#include "AudioSampleKick.h"         // http://www.freesound.org/people/DWSD/sounds/171104/
#include "AudioSampleGong.h"         // http://www.freesound.org/people/juskiddink/sounds/86773/
#include "AudioSampleCashregister.h" // http://www.freesound.org/people/kiddpark/sounds/201159/
#include <TimerOne.h>

// GUItool: begin automatically generated code
AudioPlayMemory          sound1;       //xy=94,177
AudioPlayMemory          sound4;       //xy=94,336
AudioPlayMemory          sound3;       //xy=100,279
AudioPlayMemory          sound2;       //xy=101,226
AudioMixer4              mix1;         //xy=292,258
AudioEffectDelay         delay1;         //xy=449,345
AudioMixer4              mix2;         //xy=624,264
AudioFilterBiquad        biquad1;        //xy=777,274
AudioOutputAnalog        dac1;           //xy=886,370
AudioConnection          patchCord1(sound1, 0, mix1, 0);
AudioConnection          patchCord2(sound4, 0, mix1, 3);
AudioConnection          patchCord3(sound3, 0, mix1, 2);
AudioConnection          patchCord4(sound2, 0, mix1, 1);
AudioConnection          patchCord5(mix1, 0, mix2, 0);
AudioConnection          patchCord6(mix1, delay1);
AudioConnection          patchCord7(delay1, 0, mix2, 1);
AudioConnection          patchCord8(delay1, 1, mix2, 2);
AudioConnection          patchCord9(delay1, 2, mix2, 3);
AudioConnection          patchCord10(mix2, biquad1);
AudioConnection          patchCord11(biquad1, dac1);
// GUItool: end automatically generated code

// Master mode : 0 = Simple sampler
//               1 = Sampler/Sequencer
//               2 = Simple sampler with filter
//               3 = Sampler/Sequencer with filter

const int MASTER_MODE = 1;

AudioControlSGTL5000 audioShield;

int tempo = 160;

// --------------SEQUENCEUR-----------------
const int nb_ticks = 8;
int step;
const int nb_voices = 4;
int voice = 0;

bool sequence[nb_voices][nb_ticks] =
{
   {0,0,0,0,0,0,0,0}
  ,{0,0,0,0,0,0,0,0}
  ,{0,0,0,0,0,0,0,0}
  ,{0,0,0,0,0,0,0,0}
};

// ------------- SATELLITE -----------------
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


// --------------filter----------------------
int freq = 800;
float res = 0.707; 

float cal;


void setup() {
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
  mix2.gain(1, 0.4);
  mix2.gain(2, 0.4);
  mix2.gain(3, 0.4);
  
  Serial.println("init tempo");

  if(MASTER_MODE==1||3) {
    Timer1.initialize(compute_time(tempo));
    Timer1.attachInterrupt(loop_beat);  
  }

  biquad1.setLowpass(0, freq, res);

}

int compute_time(int tempo) {
  return (60000000 / tempo / 2);
}

void loop() {
  while(Serial3.available()>4)
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
      if(test==2)
      {
      //int freq;
        unsigned char line_number = Serial3.read();
        unsigned char param_number = Serial3.read();
        unsigned char param_value = Serial3.read();

        Serial.print("line num ");
        Serial.print(line_number);
        Serial.print(" - param num ");
        Serial.print(param_number);
        Serial.print(" - val ");
        Serial.println(param_value);
      
        if(line_number==0) voice = param_number;
        else
        {
          sequence[voice][param_number] = param_value;
        }
        
/*        if(atome==3) {
          if(param_number==0) {
            freq = param_value*param_value;
            biquad1.setLowpass(0, freq, res);
          }
          if(param_number==1) {
            res = param_value/127;
            //freq = param_value*param_value;
            biquad1.setLowpass(0, freq, res);
          }
        }*/
      }
      if(test==1)
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
        if(param_number==0) {
            freq = param_value*param_value;
            biquad1.setLowpass(0, freq, res);
          }
          if(param_number==3) {
            cal = param_value;
            res = cal/127;
            //freq = param_value*param_value;
            biquad1.setLowpass(0, freq, res);
          }
      }
    }
  }
}

void loop_beat() {
  for(int i = 0; i<nb_voices; i++) {
    if(sequence[i][step]) {
      if(i==0) sound1.play(AudioSampleKick);
      if(i==1) sound2.play(AudioSampleSnare);
      if(i==2) sound3.play(AudioSampleHihat);
      if(i==3) sound4.play(AudioSampleTomtom);
      
    }
  } 
  step++;
  if(step>=nb_ticks) step=0;
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
    Serial3.write(but8LED_num);
    if(multi_mode==0)
    {
      Serial3.write(0);
      Serial3.write(line_up_mode);
      Serial3.write(line_down_mode);
    }
    if(multi_mode==1)
    {
      Serial3.write(1);
      for(int i=0; i<4; i++)
      {
        Serial3.write(col_mode[i]);
      }
    }
    for(int i=0; i<module_number; i++)
    {
      Serial3.write(multi_address[i]);
    }
    Serial3.write(255);
  }
}


