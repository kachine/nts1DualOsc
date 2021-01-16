/*
 * File: DualOsc.h
 *
 * NTS-1 dual oscillator with sync
 *
 */
#include "userosc.h"
#include <limits.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define SIGN(a)   ((a) >= 0 ? 1 : -1)

#define QUARTER_PHASE       0.25f
#define HALF_PHASE          0.5f
#define THREE_QUARTER_PHASE 0.75f
//#define NOTEFREQRATIO       1.059463094f // Frequency ratio between semitone, equivalent to powf(2, (float) 1 / 12)
#define MAX_PARAM           100
#define INIT_WAVEFORM       WAVEFORM_SAW
#define INIT_SYNC           SYNC_DISABLED
#define INIT_BALANCE        0.5f
#define INIT_DETUNE         0.5f

enum {
  WAVEFORM_SAW = 0,
  WAVEFORM_SQU = 1,
  WAVEFORM_TRI = 2,
  WAVEFORM_SIN = 3,
  WAVEFORM_RND = 4
};

enum {
  SYNC_DISABLED = 0,
  SYNC_ENABLED = 1
};

struct _voice {
  uint8_t osc1_wf;  // OSC1 waveform (0 to 4)
  uint8_t osc2_wf;  // OSC2 waveform (0 to 4)
  uint8_t sync;     // OSC2 sync (0:disable / 1:enable)
  float osc1_phase; // OSC1 phase ratio within 0 to 1
  float osc2_phase; // OSC2 phase ratio within 0 to 1
  float balance;    // OSC balance within 0 to 1
  float detune;     // OSC2 detune within -1 to 1
};
struct _voice VOICE;    // Global variable to keep voice parameters

void OSC_INIT(uint32_t platform, uint32_t api);
void OSC_CYCLE(const user_osc_param_t * const params,
               int32_t *yn,
               const uint32_t frames);
void OSC_NOTEON(const user_osc_param_t * const params);
void OSC_NOTEOFF(const user_osc_param_t * const params);
void OSC_PARAM(uint16_t index, uint16_t value);
