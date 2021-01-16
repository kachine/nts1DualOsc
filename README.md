# DualOsc
Dual oscillator for KORG NTS-1


## What's this
Simple dual oscillator with sync capable.


## Prerequisite
[logue-sdk](https://github.com/korginc/logue-sdk)

If you use pre-built binary, logue-sdk is not required. Simply transfer [DualOsc.ntkdigunit](https://github.com/kachine/nts1DualOsc/raw/main/DualOsc.ntkdigunit) to your NTS-1 using NTS-1 digital Librarian software.


## How to build
You have to change PLATFORMDIR variable in Makefile, it should correspond to where you installed logue-sdk (something like below).
```Makefile
PLATFORMDIR = $(PATHTO}/logue-sdk/platform/nutekt-digital
```

Then, simply type "make" to build this project.
```sh
$ make
```


## How to use
There are 5 paramters for this oscillator:
- Oscillator 1 waveform
This parameter selects the waveform of oscillator 1.
Choose "WAV1" by pressing OSC and tweaking TYPE knob, then input by B knob.
|Waveform|VALUE(in display)|
|---|---|
|Sawtooth wave|1|
|Square wave|2|
|Triangle wave|3|
|Sine wave|4|
|White noise|5|

- Oscillator 2 waveform
This parameter selects the waveform of oscillator 2.
Choose "WAV2" by pressing OSC and tweaking TYPE knob, then input by B knob.
Available value is same as oscillator 1.

- Balance
This parameter controls the mix balance of oscillator 1 and 2.
Choose "BALANCE" by pressing OSC and tweaking TYPE knob, then input by B knob.
The smaller value makes oscillator 1 louder, greater value makes oscillator 2 louder.

- Detune
This parameter controls the detune amount of oscillator 2.
Choose "DETUNE" by pressing OSC and tweaking TYPE knob, then input by B knob.
The positive value means detune to higher pitch, negative value means detune to lower pitch. The maximum detune amount is +or- 1 octave.

- Sync
This parameter switches whether the oscillator 2 synchronize to oscillator 1 or not.
Choose "SYNC" by pressing OSC and tweaking TYPE knob, then input by B knob.
Select 1 to enable synchronization, 0 to disable.

If you use NTS-1's built-in LFO, S(shape) is effective for detune amount. Ofcourse, P(pitch) is also effective for pitch.


---
The names of products or companies are the trademarks or registered trademarks of their respective companies. These companies have no relation to this project.
