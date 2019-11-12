#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

// I2C digital pin expander
#define PCF8574 0x38

// Samples
#include "b_samples.h"
#include "AudioSampleSd.h"
#include "AudioSampleHihat.h"
#include "AudioSampleKick.h"

#include "BassLockup.h"

// GUItool: begin automatically generated code
AudioPlayMemory      KICK_Wave;//xy=257,290
AudioPlayMemory      SD_Wave;  //xy=259,335
AudioPlayMemory      CHH_Wave; //xy=263,433
AudioPlayMemory      OHH_Wave; //xy=264,376
AudioSynthWavetable      BASS_Wave;//xy=266,469
AudioPlaySdWav           Guitar_Wave;
AudioMixer4              Drums_L;  //xy=526,303
AudioMixer4              Drums_R;  //xy=529,392
AudioMixer4              Master_R; //xy=696,465
AudioMixer4              Master_L; //xy=697,334

AudioOutputI2S i2s1;
AudioConnection          patchCord1(KICK_Wave, 0, Drums_L, 0);
AudioConnection          patchCord2(KICK_Wave, 0, Drums_R, 0);
AudioConnection          patchCord3(SD_Wave, 0, Drums_L, 1);
AudioConnection          patchCord4(SD_Wave, 0, Drums_R, 1);
AudioConnection          patchCord5(CHH_Wave, 0, Drums_L, 3);
AudioConnection          patchCord6(CHH_Wave, 0, Drums_R, 3);
AudioConnection          patchCord7(OHH_Wave, 0, Drums_L, 2);
AudioConnection          patchCord8(OHH_Wave, 0, Drums_R, 2);
AudioConnection          patchCord9(BASS_Wave, 0, Master_L, 1);
AudioConnection          patchCord10(BASS_Wave, 0, Master_R, 1);
AudioConnection          patchCord11(Drums_L, 0, Master_L, 0);
AudioConnection          patchCord12(Drums_R, 0, Master_R, 0);
AudioConnection          patchCord13(Master_R, 0, i2s1, 1);
AudioConnection          patchCord14(Master_L, 0, i2s1, 0);
AudioConnection          patchCord15(Guitar_Wave, 0, Master_L, 2);
AudioConnection          patchCord16(Guitar_Wave, 0, Master_R, 2);

// GUItool: end automatically generated code
AudioOutputAnalog  dac;     // play to both I2S audio board and on-chip DAC
AudioControlSGTL5000 audioShield;

// TODO Drum Banks
int beat_kick[]   = {1, 0, 0, 0, 1, 1, 0, 0};
int sd_beat []    = {0, 0, 1, 0, 0, 0, 1, 0};
int hh_beat []    = {0, 1, 0, 1, 1, 1, 0, 1};
int  bass_beat[]  = {1, 1, 1, 1, 1, 1, 1, 1};

enum STATES
{
  PLAYING,
  PAUSED,
  WRITTING_SETTINGS,
};
STATES state = PLAYING;

void setup()
{
  AudioMemory(30);
  audioShield.enable();
  audioShield.volume(0.75);

  Drums_R.gain(2, 0.15);
  Drums_R.gain(3, 0.15);
  Drums_L.gain(2, 0.15);
  Drums_L.gain(3, 0.15);

  Master_L.gain(1, 1);
  Master_R.gain(1, 1);

  Master_L.gain(2, 0.5);
  Master_R.gain(2, 0.5);

  BASS_Wave.setInstrument(b);
  BASS_Wave.amplitude(1);

  // Set all pins as inputs
  Wire.begin();
  Wire.beginTransmission(PCF8574);
  Wire.write(0xFF);
  Wire.endTransmission();
  delay(500);

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) 
  {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
}

//volatile byte vaux;

//unsigned int  note;
bool gtrPlaying = false;
bool guitarSwitch = false;
bool holdnote = false;

int beat;
int tick;
#define base_interval  8
int tempo = 16*base_interval;//multiplo de 16
byte lastnote ;

void ProcessStatePause();
void ProcessStatePlaying();
void ProcessStateWritting();
void ProcessTick();

byte lastbuttons = 0;

//todo 
// Flags del sequencer

void loop()
{
  byte  aux = 0;
  Wire.requestFrom(PCF8574, 1, false );
  while (Wire.available())
  {
    aux = ~Wire.read();
  }
// los dos botones a la vez
if((aux&0x01)&& (aux&0x80) && state==PLAYING) 
{ 
  // En realidad marca paused
  state=PAUSED;
}
if (aux>>1&0x0F && state==PAUSED)
{
  state = PLAYING;
  }
  ProcessStateTick(); 
    
  switch (state)
  {
    case PAUSED:
      ProcessStatePause();
      break;
    case  PLAYING: 
      ProcessStatePlaying(aux);
      break;
    case WRITTING_SETTINGS:
      ProcessStateWritting();
      break;
  }

}

void ProcessStatePause()
{
   BASS_Wave.stop();
}

// levanta los flags que sean
// paternt/bass
// Notes
// asi sabemso si ha acabado el pattern antes de procesar el siguiente input

void ProcessStateTick()
{

}

void ProcessStatePlaying(byte input)
{
byte bass_note = 38;
  const char * gtrnote = " ";
  byte currentnote = (input >> 1) & 0x0F;

if(lastnote!=currentnote && currentnote!=0x00)
{
  bass_note = BassLockup[currentnote];
  gtrnote = GuitarLockup[currentnote];
  lastnote =currentnote;
}
else
{
    bass_note = BassLockup[lastnote];
  gtrnote = GuitarLockup[lastnote];
  }
  
  // detect beat
  
  tick = tick%tempo;
  
  int measure = tempo/base_interval;
  if (tick%measure == 0)
  {
    beat  = tick/measure;
  

  AudioNoInterrupts();

  if (beat_kick[beat ] != 0)     KICK_Wave.play(AudioSampleKick);
  if (sd_beat[beat ] != 0) SD_Wave.play(AudioSampleSd);
  if (hh_beat[beat ] != 0) CHH_Wave.play(AudioSampleHihat);
  if (bass_beat[beat ] != 0)
  {
      BASS_Wave.stop();
    BASS_Wave.playNote(bass_note , 100);
  }
  }
 
     if (!(input>>1 & 0x0F)  &&  gtrPlaying == true )
  {
      guitarSwitch = false;
     gtrPlaying = false;
      Guitar_Wave.stop();

    }


  else if ((holdnote ||(input & 0x40  )) &&  gtrPlaying == false )
  {
    holdnote = false;
    gtrPlaying  = true;

    guitarSwitch = true;
    Guitar_Wave.play(gtrnote);

  }


 else if (((input & 0x40  )) &&  guitarSwitch == false )
  {
holdnote = false;
    gtrPlaying  = true;
    guitarSwitch = true;
    Guitar_Wave.stop();


    Guitar_Wave.play(gtrnote);

  }

 else if ((~input & 0x40  ) )
  {
    guitarSwitch = false;
  }


  
 AudioInterrupts();
  delay (base_interval);
  // incrementa tempo
  delayMicroseconds(100);
  // TODO
  // se puede meter una pequeña compensación del lag si la guitarra no está sonando
 tick++;
  
  }
void ProcessStateWritting()
{
  }
