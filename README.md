STM32F303 ADC DAC DSP
----

[![dimtass](https://circleci.com/gh/dimtass/stm32f303-adc-dsp-dac.svg?style=svg)](https://circleci.com/gh/dimtass/stm32f303-adc-dsp-dac)

This project code is part of the following blog post here:

[https://www.stupid-projects.com/biquad-audio-dsp-filters-using-stm32f303cc-black-pill/](https://www.stupid-projects.com/biquad-audio-dsp-filters-using-stm32f303cc-black-pill/).

It's better to look into that post for more technical details
and how to test the code.

A brief description is that the stm32f303cct6 (aka RobotDyn STM32-MINI
or black-pill), is programmed to use a timer to trigger an ADC channel
in order to sample an input with a constant sample rate. Then every
sample is copied to the RAM via a DMA channel and its post-processed with
a specified filter in the code and finaly the result is forward to the DAC.

The filter topology used is the [digital biquad filter](https://en.wikipedia.org/wiki/Digital_biquad_filter).
The mathematic formula derived from the biquad is:
```
y(n) = a0*x(n) + a1*x(n-1) + a2*x(n-2) - b*y(n-1) + b2*y(n-2)
```

This is the sum of products of samples and coefficients.

## Supported filters
- First order all-pass filter (fo_apf)
- First order high-pass filter (fo_hpf)
- First order low-pass filter (fo_lpf)
- First order high-shelving filter (fo_shelving_high)
- First order low-shelving filter (fo_shelving_low)
- Second order all-pass filter (so_apf)
- Second order band-pass filter (so_bpf)
- Second order band-stop filter (so_bsf)
- Second order Butterworth band-pass filter (so_butterworth_bpf)
- Second order Butterworth band-stop filter (so_butterworth_bsf)
- Second order Butterworth high-pass filter (so_butterworth_hpf)
- Second order Butterworth low-pass filter (so_butterworth_lpf)
- Second order high-pass filter (so_hpf)
- Second order Linkwitz-Riley high-pass filter (so_linkwitz_riley_hpf)
- Second order Linkwitz-Riley low-pass filter (so_linkwitz_riley_lpf)
- Second order Low-pass filter (so_lpf)
- Second order parametric/peaking boost filter with constant-Q (so_parametric_cq_boost)
- Second order parametric/peaking cut filter with constant-Q (so_parametric_cq_cut)
- Second order parametric/peaking filter with non-constant-Q (so_parametric_ncq)

## Code explanation
All filters are based on the digital biquad filter design.
The filters are located in `source/libs/filter_lib` and in order to use
a filter you need first to calculate the coefficients and then assign
the specific filter function to the arrays of pointers that is allocated
for running in the processing state.

The array of pointers to functions is in the `src/main.c`:
```cpp
#define NUM_OF_FILTERS 5
F_SIZE (*filter_p[NUM_OF_FILTERS])(F_SIZE sample);
```

The processing is done inside the DMA interrupt. You can apply up to five filters,
but it's recommended to use only one for testing in the beginning. 
```cpp
void DMA1_Channel1_IRQHandler(void)
```

In order to assign a filter to the list of the filters you need first to
calculate the coefficients for the filter and then assign the filter function
to the list. For example to use the 2nd-order Butterworth low-pass filter,
then in the `main()` function you can do this:
```cpp
	/* Set your filter here: */
	so_butterworth_lpf_calculate_coeffs(5000, SAMPLE_RATE);
	filter_p[0] = &so_butterworth_lpf_filter;
```

This first line will initialize the filter coefficients according to the
corner-frequency (or cut-off frequency) `fc` which in this case is 5000Hz
(or 5KHz) and the sampling rate, which by default is 96000Hz (or 96KHz).
You can change the sampling rate by setting the `SAMPLE_RATE` you want in
```cpp
#define SAMPLE_RATE 96000
```

#### Multiple filters
If you want to add more filters (e.g. create a bandpass filter), then you
can do this:
```cpp
	/* Set your filter here: */
	so_butterworth_lpf_calculate_coeffs(10000, SAMPLE_RATE);
	so_butterworth_hpf_calculate_coeffs(5000, SAMPLE_RATE);
	so_butterworth_hpf_set_offset(2048);
	filter_p[0] = &so_butterworth_hpf_filter;
	filter_p[1] = &so_butterworth_lpf_filter;
```

The above code will create a bandpass filter with 5KHz bandwidth and corner
frequencies at 3KHz and 8KHz.

What is this `so_butterworth_hpf_set_offset()` function? Well, this is used
to add an offset to the output value. It seems that for high-pass filters,
you need to add an offset in some cases, for example in case that the input
signal is a DC. In this case I've found that adding an offset of `2048` it
corrects the output. You need to test this, before adding an offset to any
filter, though.

## Clone the repo
In order to build and use this repo you need to also clone the
submodule repo that contains the [C code for the filters](https://bitbucket.org/dimtass/dsp-c-filters/src/master/).
It's easier to do this by running this command:

```sh
git clone --recursive https://github.com/dimtass/stm32f303-adc-dsp-dac.git
```

## Build the code
To build the code it's better to use a docker image that I'm using
to build stm32 firmwares and it's known to be stable and working.
Assuming that you already have Docker installed, run this command:

```sh
docker run --rm -it -v `pwd`:/tmp -w=/tmp dimtass/stm32-cde-image:0.1 -c "./build.sh"
```

This command will fetch the `dimtass/stm32-cde-image:0.1` image which includes
the `gcc-arm-none-eabi-9-2019-q4-major` compiler and cmake version `3.5.1` and
will build the code. The outcome binary firmare will be in the `build-stm/src`
folder. You can then flash it using the flash script if you already have the
[st-flash](https://github.com/texane/stlink/blob/master/doc/man/st-flash.md)
tool, by running this command:

```sh
./flash.sh
```

## pinout
The following table show the STM32F303CC pinout for this project.

STM32 pin | Function
- | -
A0 | ADC in
A4 | DAC out
A9 | UART Tx
A10 | UART Rx

## Debug port
It's good to also connect a USB to serial module to the STM32 in order to get
debug messages. By default when the code is running, then the uart port
prints the processed samples per second. Since the default value is 96000, you
should see this in your COM terminal (I'm using CuteCom).

```sh
96000
```

## Overclocking
In order to use very high sampling rates you'll need to overclock the STM32.
With the default 72MHz frequency I've managed to achieve up to 192KHz. With
the core overclocked to 128MHz I've managed to achieve 342KHz. To overclock
the STM32 then build the project using this command:

```sh
USE_OVERCLOCKING=ON ./build.sh
```

Or if using docker:
```sh
docker run --rm -it -v `pwd`:/tmp -w=/tmp dimtass/stm32-cde-image:0.1 -c "USE_OVERCLOCKING=ON ./build.sh"
```

## FW details
* `CMSIS version`: 4.2.0
* `StdPeriph Library version`: 1.2.3
* `STM3 USB Driver version`: 4.1.0

