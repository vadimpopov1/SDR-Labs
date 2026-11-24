#pragma once

#include <stddef.h>
#include <stdint.h>

#define SPS 10
#define TX_AMPLITUDE (2047 << 4)

void map_qpsk(const uint8_t *bits, int16_t *out_symbols, size_t num_symbols);
void upsample_zeros(const int16_t *in_symbols, size_t num_symbols, int16_t *out_buffer, size_t sps);
void convolve(const int16_t *in_upsampled, int16_t *out_shaped, size_t total_samples);
