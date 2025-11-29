#ifndef _DSP_CORE_H
#define _DSP_CORE_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/kfifo.h>
#include <linux/timekeeping.h>
#include <linux/types.h>

#define DSP_DEVICE_NAME "adaptive_dsp"
#define DSP_MAX_BUFFER_SIZE (44100 * 10)
#define FIXED_POINT_SCALE 32768
#define FFT_MAX_SIZE 1024
#define LMS_FILTER_LENGTH 64
#define VAD_HISTORY_SIZE 10

// DSP modes
#define DSP_MODE_FLOATING_POINT 0
#define DSP_MODE_FIXED_POINT    1

// Fixed-point arithmetic
typedef int32_t fixed_point_t;

// Complex number structure
struct complex_fixed {
    fixed_point_t real;
    fixed_point_t imag;
};

// LMS filter structure
struct lms_filter {
    fixed_point_t coefficients[LMS_FILTER_LENGTH];
    fixed_point_t delay_line[LMS_FILTER_LENGTH];
    int position;
    fixed_point_t step_size;
};

// Voice Activity Detection state
struct vad_state {
    fixed_point_t energy_history[VAD_HISTORY_SIZE];
    int history_pos;
    fixed_point_t noise_floor;
    int voice_detected;
    int hangover_count;
};

// Audio buffer structure
struct dsp_buffer {
    int16_t *data;
    size_t size;
    uint32_t sample_rate;
    struct complex_fixed *fft_input;
    struct complex_fixed *fft_output;
    struct lms_filter lms_filter;
    struct vad_state vad;
};

// Configuration structure
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

// Metrics structure
struct dsp_metrics {
    long latency;
    long cpu_usage;
    long snr;
    int precision;
    long processing_time;
    struct timespec64 timestamp;
    
    // Extended metrics
    int voice_activity;
    int32_t signal_energy;
    int32_t noise_energy;
    int resample_ratio;
    int fft_bin_energy[32];
};

// Main context - NOTE: KFIFO size MUST be a power of 2 (128 instead of 100)
struct dsp_context {
    struct cdev cdev;
    dev_t devno;
    struct class *class;
    struct dsp_config config;
    struct dsp_buffer input_buf;
    struct dsp_buffer output_buf;
    struct dsp_metrics metrics;
    struct mutex lock;
    struct workqueue_struct *workqueue;
    struct work_struct processing_work;
    DECLARE_KFIFO(metric_fifo, struct dsp_metrics, 128);
    bool processing;
    bool buffer_ready;
};

// Inline helper functions
static inline fixed_point_t float_to_fixed(float f) {
    return (fixed_point_t)(f * FIXED_POINT_SCALE);
}

static inline float fixed_to_float(fixed_point_t fixed) {
    return ((float)fixed) / FIXED_POINT_SCALE;
}

static inline fixed_point_t fixed_multiply(fixed_point_t a, fixed_point_t b) {
    return (fixed_point_t)(((int64_t)a * b) / FIXED_POINT_SCALE);
}

static inline fixed_point_t fixed_divide(fixed_point_t a, fixed_point_t b) {
    return (fixed_point_t)(((int64_t)a * FIXED_POINT_SCALE) / b);
}

static inline fixed_point_t int_to_fixed(int i) {
    return i * FIXED_POINT_SCALE;
}

static inline int32_t abs_int32(int32_t x) {
    return (x < 0) ? -x : x;
}

// Function declarations
extern int dsp_init_module(void);
extern void dsp_cleanup_module(void);

// Algorithm declarations
void fixed_point_fft(struct complex_fixed *data, int n, int inverse);
void lms_adaptive_filter_init(struct lms_filter *filter, int32_t step_size);
void lms_adaptive_filter(struct dsp_buffer *buf, struct lms_filter *filter, int32_t *reference);
int resample_audio(struct dsp_buffer *input, struct dsp_buffer *output, int target_rate);
void echo_cancellation(struct dsp_buffer *playback, struct dsp_buffer *record, struct lms_filter *filter);
void vad_init(struct vad_state *vad);
int voice_activity_detection(struct dsp_buffer *buf, struct vad_state *vad);

// ProcFS functions
int dsp_procfs_init(void);
void dsp_procfs_cleanup(void);

#endif
