/*
 * File: DualOsc.c
 *
 * NTS-1 dual oscillator with sync
 *
 */
#include <DualOsc.h>

// Triangle oscillator
//  phase: phase ratio within 0 to 1
//  return: -1 <= sample <= +1
float osc_trif(float phase){
  float sample = 0.f;

  if(phase <= QUARTER_PHASE){
    sample = phase / QUARTER_PHASE;
  }else if(phase <= HALF_PHASE){
    sample = 1.f - (phase - QUARTER_PHASE) / QUARTER_PHASE;
  }else if(phase <= THREE_QUARTER_PHASE){
    sample = 0.f - (phase - HALF_PHASE) / QUARTER_PHASE;
  }else{
    sample = -1.f + (phase - THREE_QUARTER_PHASE) / QUARTER_PHASE;
  }

  return sample;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

// Initialize function
//  called once on instantiation of the oscillator
void OSC_INIT(uint32_t platform, uint32_t api)
{
  // Initialize voice parameters
  // Initialize
  VOICE.osc1_wf = WAVEFORM_SQU;
  VOICE.osc2_wf = WAVEFORM_SQU;
  VOICE.sync = INIT_SYNC;
  VOICE.osc1_phase = _osc_rand() / UINT_MAX; // Randomize phase ratio within 0 to 1
  VOICE.osc2_phase = _osc_rand() / UINT_MAX; // Randomize phase ratio within 0 to 1
  VOICE.balance = INIT_BALANCE;
  VOICE.detune  = INIT_DETUNE;
}

// Wave Generation function
//  callbacked for each sample (or frames)
void OSC_CYCLE(const user_osc_param_t * const params,
               int32_t *yn,
               const uint32_t frames)
{
  // Pointer to output buffer
  q31_t * __restrict y = (q31_t *) yn;

  // Last address of output buffer
  const q31_t * y_e = y + frames;

  // Current LFO value within -1 to 1
  float lfo = q31_to_f32(params->shape_lfo);
  // Convert LFO range within 0 to 1
  lfo = (lfo + 1.f) * 0.5f;

  // MIDI note# and pitch modifier of current process
  // If pitch bend message has already received, note value may be differ from actual MIDI note#
  // Pitch modifier value takes within 0 to 255, the value indicate 1/255 of semitone
  // The pitch modifier is always upperward, so downer pitch bend is processed as a combination of note# decrement and adequate upperward pitch modifier.
  uint8_t note = params->pitch >> 8;
  uint8_t mod = params->pitch & 0xFF;

  // Corresponding frequency of the MIDI note#
  // Not only notenumber but also pitchbend and built-in LFO pitch modulation is taken into account
  float frequency = osc_w0f_for_note(note, mod) * k_samplerate; // osc_notehzf(note) is not suited here, because of pitch bend handling is not enough.
  // Apply detune for osc2
  float osc2_frequency = 0;
  if(VOICE.detune > 0.f){
    // Detune for +1 octave range, with LFO modulation capability
    osc2_frequency = frequency * (1.f + VOICE.detune * lfo);
    // Detune for +1 semitone
    //osc2_frequency = frequency + frequency * (NOTEFREQRATIO - 1.f) * VOICE.detune;
  }else{
    // Detune for -1 octave range, with LFO modulation capability
    osc2_frequency = frequency * ( 1.f + 0.5f * VOICE.detune * lfo);
    // Detune for -1 semitone
    //osc2_frequency = frequency / NOTEFREQRATIO + (frequency - frequency / NOTEFREQRATIO) * (1.f + VOICE.detune);
  }

  // Working memory to store current sample value
  // Effective range is -1.0 <= sample < 1.0 to convert into Q31 format later
  float sample = 0.f, osc1 = 0.f, osc2 = 0.f;

  // Process one sample by sample in frames
  while( y != y_e ) {
    // Generate osc1 wave sample
    switch(VOICE.osc1_wf){
      case WAVEFORM_SAW: osc1 = osc_sawf(VOICE.osc1_phase); break;
      case WAVEFORM_SQU: osc1 = osc_sqrf(VOICE.osc1_phase); break;
      case WAVEFORM_TRI: osc1 = osc_trif(VOICE.osc1_phase); break;
      case WAVEFORM_SIN: osc1 = osc_sinf(VOICE.osc1_phase); break;
      case WAVEFORM_RND: osc1 = osc_white(); break;
      default: break;
    }
    // Generate osc2 wave sample
    switch(VOICE.osc2_wf){
      case WAVEFORM_SAW: osc2 = osc_sawf(VOICE.osc2_phase); break;
      case WAVEFORM_SQU: osc2 = osc_sqrf(VOICE.osc2_phase); break;
      case WAVEFORM_TRI: osc2 = osc_trif(VOICE.osc2_phase); break;
      case WAVEFORM_SIN: osc2 = osc_sinf(VOICE.osc2_phase); break;
      case WAVEFORM_RND: osc2 = osc_white(); break;
      default: break;
    }

    // Mix two oscillators
    sample = (1.f - VOICE.balance) * osc1 + VOICE.balance * osc2;

    // Write a sample to output
    *(y++) = f32_to_q31(sample);

    // Step a phase ratio
    VOICE.osc1_phase += frequency / k_samplerate;
    VOICE.osc2_phase += osc2_frequency / k_samplerate;
    // Keep the phase ratio within 0 to 1
    if(VOICE.osc1_phase >= 1.f){
      VOICE.osc1_phase -= (uint32_t) VOICE.osc1_phase;
      if(VOICE.sync == SYNC_ENABLED){
        // Enforce to synchronize osc2 phase to osc1
        VOICE.osc2_phase = VOICE.osc1_phase;
      }
    }else{
      VOICE.osc2_phase -= (uint32_t) VOICE.osc2_phase;
    }
  }
}

// MIDI note-on event process function
//  * This function is not hooked if active note is already exist
void OSC_NOTEON(const user_osc_param_t * const params)
{
  // Nothing to do
}

// MIDI note-off event process function
//  * This function is not hooked if active note remains
void OSC_NOTEOFF(const user_osc_param_t * const params)
{
  // Nothing to do
}

// Parameter change event process funnction
void OSC_PARAM(uint16_t index, uint16_t value)
{
  // 0-200 for bipolar percent parameters. 0% at 100, -100% at 0.
  // 0-100 for unipolar percent and typeless parameters.
  // 10 bit resolution for shape/shift-shape.
  switch (index) {
    case k_user_osc_param_id1: // Waveform1 (0-4)
      // Choose this param by pressing OSC and tweaking TYPE knob, then input by B knob
      VOICE.osc1_wf = (uint8_t) value;
      break;
    case k_user_osc_param_id2: // Waveform2 (0-4)
      // Choose this param by pressing OSC and tweaking TYPE knob, then input by B knob
      VOICE.osc2_wf = (uint8_t) value;
      break;
    case k_user_osc_param_id3: // Balance (0-100)
      // Choose this param by pressing OSC and tweaking TYPE knob, then input by B knob
      VOICE.balance = (float) value / MAX_PARAM;
      break;
    case k_user_osc_param_id4: // detune (0-200)
      // Choose this param by pressing OSC and tweaking TYPE knob, then input by B knob
      VOICE.detune = (float) ((int16_t) value - MAX_PARAM) / MAX_PARAM;
      break;
    case k_user_osc_param_id5: // Sync enable (0-1)
      // Choose this param by pressing OSC and tweaking TYPE knob, then input by B knob
      VOICE.sync = (uint8_t) value;
      break;
    case k_user_osc_param_id6: // Not used
      // Choose this param by pressing OSC and tweaking TYPE knob, then input by B knob
      break;
    case k_user_osc_param_shape: // Not used
      // 10bit parameter, 0 <= value <= 1023
      // Mapped to OSC mode A knob(shape) and MIDI CC#54
      //   CC#54 value=0:   uint16_t value=0
      //   CC#54 value=32:  uint16_t value=256
      //   CC#54 value=64:  uint16_t value=512
      //   CC#54 value=126: uint16_t value=1008
      //   CC#54 value=127: uint16_t value=1023
      //    --> CC#54 value*8 = uint16_t value, except CC value=127
      // You can use param_val_to_f32(value) to convert value to the range within 0 to 1 in float format
      break;
    case k_user_osc_param_shiftshape: // Not used
      // Similar to k_user_osc_param_shape, but mapped to OSC mode B knob(alt) and MIDI CC#55
      break;
    default:
      break;
  }
}
#pragma GCC diagnostic pop
