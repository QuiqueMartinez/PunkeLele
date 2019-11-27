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
//int beat;
enum STATES
{
  PLAYING_MODE_1 = 1 << 0,
  PLAYING_MODE_2 = 1 << 1,
  PLAYING_MODE_3 = 1 << 2,
  PLAYING_MODE_4 = 1 << 3,
  PAUSED =  1 << 4,
  WRITTING_SETTINGS = 1 << 5,
};
STATES state = PLAYING_MODE_3;

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

  Master_L.gain(2, 0.65);
  Master_R.gain(2, 0.65);

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


// minimum Count of the sequencer resolution
int tick = 0;
bool beatflag; // indicates if trigger a souind
int beat;
//delay between ticks
#define base_interval  20
// one beat contains 8 ticks
#define ticks_per_beat  8
#define beats_per_pattern  8

byte currentbuttons = 0;
byte lastbuttons = 0;
byte  signals = 0;

byte lastnote ;

void ProcessStatePause();
void ProcessStatePlaying();
void ProcessStateWritting();
void ProcessTick();
void ProcessInput();


//todo
// Flags del sequencer
byte sequencerFlags = 0;

enum {
  GREEN = 1 << 0,
  RED = 1 << 1,
  YELLOW = 1 << 2,
  BLUE = 1 << 3,
  ORANGE = 1 << 4,
  DOWN = 1 << 5,
  UP = 1 << 6,
  MUTE = 1 << 7,
};


enum // flags
{
  NEW_PATTERN,
  NEW_BEAT,
  MARK_DOWNSTROKE,
  MARK_UPSTROKE,
};




void loop()
{

  ProcessInputs();
  ProcessTick();


  switch (state)
  {
    case PAUSED:
      ProcessStatePause();
      break;
    case  PLAYING_MODE_1:
    case  PLAYING_MODE_2:
    case  PLAYING_MODE_3:
    case  PLAYING_MODE_4:
    if(currentbuttons & GREEN)//shift
      {
          if (signals & UP) 
          {
            switch (state)
            {
              case PLAYING_MODE_1: state= PLAYING_MODE_2; break;
              case PLAYING_MODE_2: state= PLAYING_MODE_3; break;
              case PLAYING_MODE_3: state= PLAYING_MODE_4; break;
              case PLAYING_MODE_4: state= PLAYING_MODE_1; break;
              default : break;
            }
            
          }
          if (signals & DOWN) 
          {
        switch (state)
            {
              case PLAYING_MODE_1: state= PLAYING_MODE_4; break;
              case PLAYING_MODE_2: state= PLAYING_MODE_1; break;
              case PLAYING_MODE_3: state= PLAYING_MODE_2; break;
              case PLAYING_MODE_4: state= PLAYING_MODE_3; break;
              default: break;
            }
           }
         
      }

      ProcessStatePlaying();

      break;
    case WRITTING_SETTINGS:
      ProcessStateWritting();
      break;
  }

}


void ProcessInputs()
{
    // Pressed keys
  currentbuttons = 0;

  Wire.requestFrom(PCF8574, 1, false );
  while (Wire.available())
  {
    currentbuttons = ~Wire.read();
  }
  if (currentbuttons == lastbuttons) return;
  signals = lastbuttons^currentbuttons;
  lastbuttons = currentbuttons;
  // los dos botones a la vez
  /*if ((currentbuttons & GREEN) && (currentbuttons & MUTE) && state == PLAYING_MODE_3 && (sequencerFlags & NEW_PATTERN) )
  {
    // En realidad marca paused
    state = PAUSED;
  }
  if (currentbuttons >> 1 & 0x0F && state == PAUSED && (sequencerFlags & NEW_PATTERN) )
  {
    state = PLAYING_MODE_1;
  }*/
}

void ProcessStatePause()
{
  BASS_Wave.stop();
}

// levanta los flags que sean
// paternt/bass
// Notes
// asi sabemso si ha acabado el pattern antes de procesar el siguiente input

void ProcessTick()
{
  tick = tick % (ticks_per_beat * beats_per_pattern);
 
  sequencerFlags = 0;
  if (tick % ticks_per_beat == 0)   sequencerFlags = sequencerFlags |  NEW_BEAT;
  if (tick == 0) sequencerFlags = sequencerFlags |  NEW_PATTERN;
  beat = tick / ticks_per_beat;
  tick++;

  delay (base_interval);
}

void ProcessStatePlaying()
{
  
  const char * gtrnote = " ";
  
  byte currentnote = (currentbuttons >> 1) & 0x0F;

  // Detect key release
  bool noteKeysReleased = (!(currentbuttons >> 1 & 0x0F)  &&  gtrPlaying == true );
  bool newNote = (lastnote != currentnote) && (currentnote != 0x00);

  // detect new notea
  if (newNote)
  {
  //  bass_note = BassLockup[currentnote];
    gtrnote = GuitarLockup[currentnote];
    lastnote = currentnote;
  }
  else
  {
  //  bass_note = BassLockup[lastnote];
    gtrnote = GuitarLockup[lastnote];
  }

 

  AudioNoInterrupts();

  
  
  // Drums section
  DrumsSection();
 // si doy para arriba guitarrazo sin compasion
  BassSection(newNote, BassLockup[currentnote]);
  // Bass Section
  
  

   if (!(currentbuttons & GREEN))
   {
  // stops guitar if no
  if (noteKeysReleased )
  {
    guitarSwitch = false;
    gtrPlaying = false;
    Guitar_Wave.stop();

  }


  else if ((holdnote || (currentbuttons & 0x40  )) &&  gtrPlaying == false )
  {
    holdnote = false;
    gtrPlaying  = true;

    guitarSwitch = true;
    Guitar_Wave.play(gtrnote);

  }


  else if (((currentbuttons & 0x40  )) &&  guitarSwitch == false )
  {
    holdnote = false;
    gtrPlaying  = true;
    guitarSwitch = true;
    Guitar_Wave.stop();


    Guitar_Wave.play(gtrnote);

  }

  else if ((~currentbuttons & 0x40  ) )
  {
    guitarSwitch = false;
  }
   }


  AudioInterrupts();


}

void DrumsSection()
{
  if (state == PLAYING_MODE_1) return;
  else if (sequencerFlags & NEW_BEAT)
  {
    if (beat_kick[beat] != 0)    KICK_Wave.play(AudioSampleKick);
    if (sd_beat[beat ] != 0) SD_Wave.play(AudioSampleSd);
    if (hh_beat[beat ] != 0) CHH_Wave.play(AudioSampleHihat);

  }

  }

  void BassSection(bool newnote, byte bassnote)
  {
   if (state & (PLAYING_MODE_1 | PLAYING_MODE_2) && sequencerFlags|NEW_BEAT )
   {
          BASS_Wave.stop();
   }
   else if (sequencerFlags & NEW_BEAT && state & PLAYING_MODE_3  )
   {
    if (bass_beat[beat ] != 0 )
    {
      BASS_Wave.stop();
      BASS_Wave.playNote(bassnote , 100);
    }

    }
  }

void ProcessStateWritting()
{
}
