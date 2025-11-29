#ifndef _DSP_USER_H
#define _DSP_USER_H

#include <stdint.h>
#include <sys/ioctl.h>

#define DSP_DEVICE_NAME "adaptive_dsp"
#define DSP_MAX_BUFFER_SIZE (44100 * 10)
#define FIXED_POINT_SCALE 32768

// DSP modes
#define DSP_MODE_FLOATING_POINT 0
#define DSP_MODE_FIXED_POINT    1

// Configuration structure (user-space version)
struct dsp_config {
    int adaptive_mode;
    int noise_reduction;
    int compression;
    int cpu_threshold;
    int snr_threshold;
    int current_precision;
    int debug_level;
    
    // Extended features
    int enable_fft;
    int enable_lms;
    int enable_resampling;
    int enable_echo_cancellation;
    int enable_vad;
    int target_sample_rate;
    int fft_size;
    int lms_filter_length;
    float lms_step_size;
};

// Metrics structure (user-space version)
struct dsp_metrics {
    long latency;
    long cpu_usage;
    long snr;
    int precision;
    long processing_time;
    
    // Extended metrics
    int voice_activity;
    int32_t signal_energy;
    int32_t noise_energy;
    int resample_ratio;
    int fft_bin_energy[32];
};

// LMS parameters for IOCTL
struct lms_params {
    int length;
    float step_size;
};

// IOCTL commands
#define DSP_IOC_MAGIC 'D'

#define DSP_IOCTL_GET_CONFIG    _IOR(DSP_IOC_MAGIC, 1, struct dsp_config)
#define DSP_IOCTL_SET_CONFIG    _IOW(DSP_IOC_MAGIC, 2, struct dsp_config)
#define DSP_IOCTL_GET_METRICS   _IOR(DSP_IOC_MAGIC, 3, struct dsp_metrics)
#define DSP_IOCTL_RESET_BUFFERS _IO(DSP_IOC_MAGIC, 4)
#define DSP_IOCTL_SET_SAMPLE_RATE _IOW(DSP_IOC_MAGIC, 5, uint32_t)
#define DSP_IOCTL_SET_FFT_SIZE      _IOW(DSP_IOC_MAGIC, 6, int)
#define DSP_IOCTL_SET_LMS_PARAMS    _IOW(DSP_IOC_MAGIC, 7, struct lms_params)
#define DSP_IOCTL_SET_RESAMPLE_RATE _IOW(DSP_IOC_MAGIC, 8, int)
#define DSP_IOCTL_ENABLE_FFT        _IOW(DSP_IOC_MAGIC, 9, int)
#define DSP_IOCTL_ENABLE_VAD        _IOW(DSP_IOC_MAGIC, 10, int)

#endif
