# Extended Adaptive DSP Kernel Module

Advanced Digital Signal Processing Linux kernel module with real-time audio processing capabilities.

## Features

- **Adaptive Precision**: Switches between fixed/floating point based on CPU load
- **FFT Analysis**: Fixed-point Fast Fourier Transform
- **LMS Adaptive Filter**: For noise and echo cancellation
- **Multi-rate Processing**: Real-time audio resampling
- **Voice Activity Detection**: Energy-based speech detection
- **Echo Cancellation**: Reference-based echo removal
- **ProcFS Interface**: Real-time monitoring via /proc/adaptive_dsp/

## Building

```bash
make
