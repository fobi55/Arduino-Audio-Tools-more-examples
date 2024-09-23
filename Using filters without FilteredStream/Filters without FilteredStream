
/**
 * @file streams-generator-i2s.ino
 * @author Phil Schatzmann
 * @brief see https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-stream/streams-generator-i2s/README.md 
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
 
 /*
  *Board NodeMCU-32S
 */

#include "AudioTools.h"
#include "coeffs60.h"                             // 90 degree phase shift 60 taps (~4.5 KHz cutoff) from NA5Y:
// https://github.com/thaaraak/ESP32-A1S-Tayloe

typedef int16_t sound_t;                          // sound will be represented as int16_t (with 2 bytes)
uint16_t sample_rate=16000;
uint8_t channels = 2;                             // The stream will have 2 channels 

//Input stream
SineWaveGenerator<sound_t> sineWave(12000);       // subclass of SoundGenerator with max amplitude of 12000
GeneratedSoundStream<sound_t> sound(sineWave);    // Stream generated from sine wave

// Output stream
CsvOutput<int16_t> out(Serial);                   //    

//FIR<float> fir0(coeffs_60plus45,-1);            // Will be declared latter for synax comparison
FIR<float> fir1(coeffs_60minus45,-1);

//Filters using Converter
ConverterNChannels<int16_t, float> f2(2);

StreamCopy copier(out, sound, 512);               // Copier in -> out

// Arduino Setup
void setup(void) {  
  // Open Serial 
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Error);

  // Using Converter
  f2.setFilter(0, new FIR<float>(coeffs_60plus45,-1));          //Without filter declaraton. Word 'new' allocates memory.
  //f2.setFilter(1, new FIR<float>(coeffs_60minus45,-1));  

  //Filter<float>* p1 = &fir1;
  //f2.setFilter(1, p1);
  // Or just:
  f2.setFilter(1, &fir1);                                       //With declared filter

  // Configure the output
  Serial.println("starting output...");
  auto config = out.defaultConfig(TX_MODE);   
  config.sample_rate = sample_rate;
  config.bits_per_sample = 16;

  // start output
  out.begin(config);

  // Setup sine wave
  sineWave.begin(channels, sample_rate, 1000);
  Serial.println("started...");
}

// Arduino loop - copy sound to out 
void loop() {

  copier.copy(f2);    //Filtering with Converter 

}
