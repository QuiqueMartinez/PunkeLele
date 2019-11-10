#pragma once
#include <AudioStream.h>
#include <Audio.h>

extern const AudioSynthWavetable::sample_data b_samples[1];
const uint8_t b_ranges[] = {127, };

const AudioSynthWavetable::instrument_data b = {1, b_ranges, b_samples };


extern const uint32_t sample_0_b_A0[11136];
