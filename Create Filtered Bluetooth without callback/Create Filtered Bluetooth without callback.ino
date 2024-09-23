/**
 * @file streams-i2s-filter-i2s.ino
 * @brief Copy audio from I2S to I2S using an FIR filter
 * @author Phil Schatzmann
 * @copyright GPLv3
 * Modified for BT to internal DAC using FIR filters and copier: Fobi
 */

#include "AudioTools.h"
#include "BluetoothA2DPSink.h"
#include "coeffs60.h"

uint16_t sample_rate=44100;
uint16_t channels = 2;

AnalogAudioStream out;
// copy filtered values
FilteredStream<int16_t, float> filtered(out, channels);  // Defines the filter as BaseConverter
BluetoothA2DPSink a2dp_sink(filtered);          // sink directly to the filter, thats all.
StreamCopy copier(out, filtered);               // Copy filter to the DAC

// Arduino Setup
void setup(void) {  

  pinMode(26, OUTPUT);  // Not sure if I need this
  pinMode(27, OUTPUT);

  // 90 degree phase shift 160 taps (~4.5 KHz cutoff)
  filtered.setFilter(0, new FIR<float>(coeffs_60plus45));
  filtered.setFilter(1, new FIR<float>(coeffs_60minus45));

  auto config = out.defaultConfig(TX_MODE); // DAC has only TX_MODE in comparison with I2S: Fob
  config.sample_rate = sample_rate;
  config.bits_per_sample = 16;

  //Run all streams
  out.begin(config);
  a2dp_sink.start("InternalDAC"); 
}

// Arduino loop
void loop() {

  copier.copy();

}
