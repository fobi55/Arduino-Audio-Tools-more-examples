/**
 * @file GEN_2_DAC_Callback.ino
 * @brief Delay L or R channel using Callback
 * @author Phil Schatzmann, Boyan Petrov
 * @copyright GPLv3
 * HW  NodeMCU-32S
 * Serial Input:
 * 0: L channel delay, even indexes
 * 1: R channel delay, odd indexes
 * All > 1 : delay samples for the delayed channel
 */

#include "AudioTools.h"

typedef int16_t sound_t;
uint16_t sample_rate = 44100;  // For the output
uint8_t channels = 2;

AudioInfo info(sample_rate, channels, 16);  //For the CallbackStream

//**********************************************************************************

const int blen = 20;
int16_t samples_old[blen];
int16_t samples_new[blen];

//ch: 0 -> even, 1 -> odd
void shift_ch_right(int16_t arr[], size_t n, int d, uint8_t ch) {
  d = d % n;  // adjust d if it's greater than n
  int dd = 2*d;

  for (int i = n; i >= 0; i -= 2) {  // shift right in reversed order

    if (i > n - dd) {
      // temp counts d to 0 by -1
      // read current last d samples
      samples_new[((i - 1 - (n - dd)) >> 1)] = arr[i - 2 + ch];
    }

    if (i >= dd) {  // shift right by d samples
      arr[i - 2 + ch] = arr[i - 2 - dd + ch];
    }

    if (i < dd) {
      // add previous last d samples in front
      arr[i + ch] = samples_old[(i >> 1)];
      // remember current last d samples
      samples_old[(i >> 1)] = samples_new[(i >> 1)];
    }
  }
}

//********************************************************************************
// Callback here:
uint8_t sel_ch = 1;  //0: even, 1: odd
uint8_t s_delay = 8;

auto delayLRch = [](uint8_t* data, size_t bytes) {
  size_t sample_count = bytes / sizeof(int16_t);
  int16_t* data16 = (int16_t*)data;
  shift_ch_right(data16, sample_count, s_delay, sel_ch);

  return bytes;
};

//********************************************************************************

SineWaveGenerator<sound_t> sineWave(12000);     // subclass of SoundGenerator with max amplitude of 32000
GeneratedSoundStream<sound_t> sound(sineWave);  // Stream generated from sine wave ( has no output)
AnalogAudioStream out;                          // DAC
CallbackStream cb(out, delayLRch);              // It has output
StreamCopy copier(cb, sound);                   // Copy sound to the CalbackStream

//********************************************************************************

// Arduino Setup
void setup(void) {

  pinMode(26, OUTPUT);
  pinMode(25, OUTPUT);

  // Open Serial
  Serial.begin(115200);
  // change to Warning to improve the quality
  //AudioLogger::instance().begin(Serial, AudioLogger::Error);

  auto config = out.defaultConfig(TX_MODE);  // DAC has only TX_MODE
  config.sample_rate = sample_rate;
  config.bits_per_sample = 16;
  config.channels = channels;

  out.begin(config);
  cb.begin(info);
  sineWave.begin(channels, sample_rate, 1500);
}

//********************************************************************************

// Arduino loop
void loop() {
  // Input
  if (Serial.available() > 0) {
    String sInput = Serial.readString();
    sInput.trim();
    //sel_ch = sInput.toInt();
    //s_delay = sInput.toInt();
    change_ch_delay(sInput.toInt());
    Serial.println(sInput);
  }

  copier.copy();  // Copies sound to the CallbackStream
}

//*********************************************************************************

void change_ch_delay(int chd) {

  switch (chd) {
    case 0:
      sel_ch = 0;
      break;
    case 1:
      sel_ch = 1;
      break;
    default:
      if (chd <= blen) {
        s_delay = chd;
      }

      break;
  }
}
