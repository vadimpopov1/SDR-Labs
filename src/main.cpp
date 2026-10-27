#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const double freq1 = 800e6;
static const double freq2 = 850e6;

static volatile sig_atomic_t running = 1;
static void on_sigint(int s) { (void)s; running = 0; }

SoapySDRDevice *make_device(const char *uri) {
  SoapySDRKwargs args = {0};
  SoapySDRKwargs_set(&args, "driver", "plutosdr");
  SoapySDRKwargs_set(&args, "uri", uri);
  SoapySDRKwargs_set(&args, "direct", "1");
  SoapySDRKwargs_set(&args, "timestamp_every", "1920");
  SoapySDRKwargs_set(&args, "loopback", "0");
  SoapySDRDevice *sdr = SoapySDRDevice_make(&args);
  SoapySDRKwargs_clear(&args);
  return sdr;
}

static int check(int rc, const char *what) {
  if (rc != 0) {
    fprintf(stderr, "%s failed: rc=%d (%s) | %s\n", what, rc,
            SoapySDR_errToStr(rc), SoapySDRDevice_lastError());
    return 1;
  }
  return 0;
}

int run(const char *uri, bool is_tx) {
  signal(SIGINT, on_sigint);

  SoapySDRDevice *sdr = make_device(uri);
  if (!sdr) {
    fprintf(stderr, "make failed (uri=%s): %s\n", uri, SoapySDRDevice_lastError());
    return 1;
  }
  fprintf(stderr, "[1] device opened\n");

  double sample_rate = 2e6;
  double rx_freq = freq1;
  double tx_freq = freq2;
  const char *filename = "rxdata.pcm";
  if (is_tx) {
    rx_freq = freq2;
    tx_freq = freq1;
    filename = "txdata.pcm";
  }

  if (check(SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, sample_rate), "setSampleRate RX")) return 1;
  if (check(SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, sample_rate), "setSampleRate TX")) return 1;
  if (check(SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, rx_freq, NULL), "setFrequency RX")) return 1;
  if (check(SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, tx_freq, NULL), "setFrequency TX")) return 1;
  fprintf(stderr, "[2] rate/freq set\n");

  size_t channels[] = {0};
  if (check(SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels[0], 45.0), "setGain RX")) return 1;
  if (check(SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, channels[0], 45.0), "setGain TX")) return 1;
  size_t channel_count = sizeof(channels) / sizeof(channels[0]);
  fprintf(stderr, "[3] gain set\n");

  SoapySDRStream *rxStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, NULL);
  if (!rxStream) { fprintf(stderr, "setupStream RX failed: %s\n", SoapySDRDevice_lastError()); return 1; }
  SoapySDRStream *txStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, NULL);
  if (!txStream) { fprintf(stderr, "setupStream TX failed: %s\n", SoapySDRDevice_lastError()); return 1; }
  fprintf(stderr, "[4] streams created\n");

  if (check(SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0), "activateStream RX")) return 1;
  if (check(SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0), "activateStream TX")) return 1;
  fprintf(stderr, "[5] streams active\n");

  size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);
  size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);
  fprintf(stderr, "[6] rx_mtu=%zu tx_mtu=%zu\n", rx_mtu, tx_mtu);
  if (rx_mtu == 0 || tx_mtu < 6) {
    fprintf(stderr, "MTU is too small\n");
    return 1;
  }

  int16_t *rx_buffer = (int16_t *)malloc(2 * rx_mtu * sizeof(int16_t));
  int16_t *tx_buff = (int16_t *)malloc(2 * tx_mtu * sizeof(int16_t));
  if (!rx_buffer || !tx_buff) { perror("malloc"); return 1; }

  const long timeoutUs = 400000;
  long long last_time = 0;
  FILE *file = fopen(filename, "wb");
  if (!file) { perror("fopen"); return 1; }
  size_t buffers_read = 0;

  for (size_t i = 0; i < 2 * tx_mtu; i += 2) {
    tx_buff[i] = 1500 << 4;      // I
    tx_buff[i + 1] = 1500 << 4;  // Q
  }

  void *rx_buffs[] = {rx_buffer};
  const void *tx_buffs[] = {tx_buff};

  while (running) {
    int flags = 0;
    long long timeNs = last_time;
    int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);
    if (sr < 0) {
      printf("RX error: %i (%s)\n", sr, SoapySDR_errToStr(sr));
      continue;
    }
    printf("Buffer: %zu - Samples: %i, Flags: %i, Time: %lli, TimeDiff: %lli\n",
           buffers_read, sr, flags, timeNs, timeNs - last_time);
    last_time = timeNs;

    long long tx_time = last_time + (4 * 1000 * 1000);  // на 4 [мс] в будущее

    for (size_t i = 0; i < 2; i++) {
      tx_buff[0 + i] = (int16_t)0xffff;
      tx_buff[10 + i] = (int16_t)0xffff;
    }
    for (size_t i = 0; i < 8; i++) {
      uint8_t b = (tx_time >> (i * 8)) & 0xff;
      tx_buff[2 + i] = b << 4;
    }

    if (buffers_read % 2 == 0) {
      flags = SOAPY_SDR_HAS_TIME;
      int st = SoapySDRDevice_writeStream(sdr, txStream, tx_buffs, tx_mtu, &flags, tx_time, timeoutUs);
      if ((size_t)st != tx_mtu) printf("TX Failed: %i\n", st);
    }

    fwrite(rx_buffer, sizeof(int16_t), 2 * (size_t)sr, file);
    buffers_read++;
  }

  fclose(file);
  SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
  SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);
  SoapySDRDevice_closeStream(sdr, rxStream);
  SoapySDRDevice_closeStream(sdr, txStream);
  SoapySDRDevice_unmake(sdr);
  free(rx_buffer);
  free(tx_buff);
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Использовать так: %s <rx|tx> [uri]\n", argv[0]);
    return 1;
  }
  const char *uri = "usb:6.6.6";
  if (argc > 2) uri = argv[2];
  if (strcmp(argv[1], "rx") == 0) return run(uri, false);
  if (strcmp(argv[1], "tx") == 0) return run(uri, true);
  printf("Ошибка режима работы\n");
  return 1;
}