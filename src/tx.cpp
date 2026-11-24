#include "../headers/tx.h"

static const int16_t h[SPS] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

void map_qpsk(const uint8_t *bits, int16_t *out_symbols, size_t num_symbols) {
    for (size_t sym = 0; sym < num_symbols; sym++) {
        uint8_t bit_i = bits[2 * sym];
        uint8_t bit_q = bits[2 * sym + 1];

        out_symbols[2 * sym] = bit_i ? -TX_AMPLITUDE : TX_AMPLITUDE;
        out_symbols[2 * sym + 1] = bit_q ? -TX_AMPLITUDE : TX_AMPLITUDE;
    }
}

void upsample_zeros(const int16_t *in_symbols, size_t num_symbols, int16_t *out_buffer, size_t sps) {
    for (size_t sym = 0; sym < num_symbols; sym++) {
        size_t base_idx = sym * sps * 2;

        out_buffer[base_idx] = in_symbols[2 * sym];
        out_buffer[base_idx + 1] = in_symbols[2 * sym + 1];

        for (size_t k = 1; k < sps; k++) {
            out_buffer[base_idx + 2 * k] = 0;
            out_buffer[base_idx + 2 * k + 1] = 0;
        }
    }
}

void convolve(const int16_t *in_upsampled, int16_t *out_shaped, size_t total_samples) {
    for (size_t n = 0; n < total_samples; n++) {
        int32_t acc_i = 0;
        int32_t acc_q = 0;

        for (size_t k = 0; k < SPS; k++) {
            if (n >= k) {
                size_t past_idx = n - k;
                acc_i += in_upsampled[2 * past_idx] * h[k];
                acc_q += in_upsampled[2 * past_idx + 1] * h[k];
            }
        }

        out_shaped[2 * n] = (int16_t)acc_i;
        out_shaped[2 * n + 1] = (int16_t)acc_q;
    }
}
