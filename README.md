# VEUVE: STM32L4 Guitar Effect Processor

VEUVE is a real-time DSP application designed for the STM32L476xx microcontroller. It transforms the board into a low-latency audio effect pedal capable of processing guitar signals through digital effects with high efficiency.

## Features

* **DMA-Powered Audio Pipeline**: Utilizes DMA for both ADC1 input and DAC1 output to ensure low-latency processing.
* **Real-time Audio Effects**:
    * **Passthrough (Effect 0)**: Transparent signal routing.
    * **Distortion (Effect 1)**: Features adjustable gain and clipping thresholds.
    * **Octave Fuzz (Effect 2)**: Implements full-wave rectification logic for upper octave harmonics.
* **Double Buffering (Ping-Pong)**: Uses half-complete and full-complete DMA callbacks to process audio data without interrupting the stream.
* **Dry/Wet Mixing**: Integrated blend control to balance the processed signal with the original input.

## Technical Implementation

### Audio Processing Chain
1.  **Input**: ADC1 samples the incoming signal.
2.  **Conversion**: 12-bit ADC values (0-4095) are converted to floats for processing.
3.  **DC Offset Management**: The application removes the 2048-count DC offset before processing and restores it before outputting to the DAC.
4.  **Signal Processing**:
    * **Distortion**: The signal is amplified by a `GAIN` factor and capped at `EFFECT_THRESHOLD`.
    * **Octave Fuzz**: Negative signal excursions are flipped to positive using absolute value logic.
5.  **Output**: Processed samples are sent to DAC Channel 1 via DMA, triggered by Timer 6.

### Hardware Configuration
* **MCU**: STM32L476xx.
* **Peripherals**: ADC1, DAC1, TIM6 (Trigger)

## Building the Project

The project uses a CMake-based build system.

1.  **Prerequisites**:
    * ARM GNU Toolchain (`arm-none-eabi-gcc`).
    * CMake 3.22 or higher.
2.  **Compilation**:
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

## Configuration

Effect settings can be (for now) modified in `Core/Src/main.c` via macros:
* `EFFECT`: Select 0, 1, or 2.
* `MIX`: Adjust dry/wet ratio (0.0f to 1.0f).
* `GAIN`: Adjust distortion intensity.
* `EFFECT_THRESHOLD`: Set the clipping level.

## Future Roadmap

The project is actively evolving with the following planned features:
* **New Effects**: Implementation of Delay, Autowah, Octaver and hopefully many more...
* **QoL Enhancements**: Improved settings management through external configuration files.
* **Advanced Controls**: Integration of GUI, Tap Tempo support, and effect daisy-chaining capabilities.
* **Hardware Release**: Once the electronic design is finalized, complete circuit schematics and CAD models for the housing will be released.
