/**
 * @file GEN_2_DAC_callback_ch_AM.ino
 * @brief One channel AM to internal DAC
 * @author Phil Schatzmann, Boyan Petrov
 * @copyright GPLv3
 * HW NodeMCU-32S
 */

#include <cmath>
#include "AudioTools.h"

typedef int16_t sound_t;
uint16_t sample_rate = 44100;
uint16_t channels = 2;

AudioInfo info(44100, 2, 16);

//*******************************************************************************
class SineOscillator {  // A*sin(2*pi*f/sr)
  float frequency, amplitude, angle = 0.0f, offset = 0.0f;
public:
  SineOscillator(float freq, float amp)
    : frequency(freq), amplitude(amp) {
    offset = 2 * M_PI * frequency / sample_rate;
  }
  float process() {
    auto sample = amplitude * sinf(angle);
    angle += offset;

    //remainder(angle, 2 * M_PI); // too slow angle wrapper
    if (angle > 0) {
      angle = fmod(angle + M_PI, 2.0 * M_PI) - M_PI;
    } else {
      angle = fmod(angle - M_PI, 2.0 * M_PI) + M_PI;
    }

    return sample;
  }
};

// define this before the callback
SineOscillator Tone(11025, 1); // Carrier frequency

//***********************************************************************************
auto invertLch = [](uint8_t* data, size_t bytes) {
  size_t sample_count = bytes / sizeof(int16_t);
  int16_t* data16 = (int16_t*)data;
  for (int j = 0; j < sample_count; j += 2) {
    //data16[j] = -data16[j];
    //data16[j] = Tone.process() * norm;
    data16[j] = data16[j] * Tone.process();
  }
  return bytes;
};
//**********************************************************************************
SineWaveGenerator<sound_t> sineWave(15000);     // subclass of SoundGenerator with max amplitude of 32000
GeneratedSoundStream<sound_t> sound(sineWave);  // Stream generated from sine wave ( has no output)
AnalogAudioStream out;
CallbackStream cb(out, invertLch);
StreamCopy copier(cb, sound);                  // sound has no output, use copy

// Arduino Setup
void setup(void) {

  pinMode(26, OUTPUT);
  pinMode(25, OUTPUT);

  // Open Serial
  //Serial.begin(115200);                           
  // change to Warning to improve the quality
  //AudioLogger::instance().begin(Serial, AudioLogger::Error);

  auto config = out.defaultConfig(TX_MODE);  // DAC has only TX_MODE
  config.sample_rate = sample_rate;
  config.bits_per_sample = 16;
  config.channels = channels;

  // Run all
  out.begin(config);
  cb.begin(info);
  sineWave.begin(channels, sample_rate, 1500);
}

//const int blen = 16;
//int16_t buffer[blen];

// Arduino loop
void loop() {
  copier.copy();
}
