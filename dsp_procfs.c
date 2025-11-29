#include "dsp_core.h"
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

extern struct dsp_context *dsp_ctx;
static struct proc_dir_entry *dsp_proc_dir = NULL;
static struct proc_dir_entry *dsp_proc_config = NULL;
static struct proc_dir_entry *dsp_proc_metrics = NULL;
static struct proc_dir_entry *dsp_proc_status = NULL;
static struct proc_dir_entry *dsp_proc_advanced = NULL;

static ssize_t dsp_proc_config_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    struct dsp_context *ctx = dsp_ctx;
    char config_str[512];
    int len;
    
    if (*ppos > 0) return 0;
    
    if (!ctx) return -EINVAL;
    
    mutex_lock(&ctx->lock);
    
    len = snprintf(config_str, sizeof(config_str),
        "Adaptive DSP Configuration:\n"
        "=======================\n"
        "Adaptive Mode:    %d\n"
        "Noise Reduction:  %d\n"
        "Compression:      %d\n"
        "CPU Threshold:    %d%%\n"
        "SNR Threshold:    %d.%02d dB\n"
        "Current Precision: %s\n"
        "Debug Level:      %d\n"
        "FFT Enabled:      %d\n"
        "LMS Enabled:      %d\n"
        "Resampling:       %d\n"
        "VAD Enabled:      %d\n"
        "Target Rate:      %d Hz\n",
        ctx->config.adaptive_mode,
        ctx->config.noise_reduction,
        ctx->config.compression,
        ctx->config.cpu_threshold,
        ctx->config.snr_threshold / 100, ctx->config.snr_threshold % 100,
        ctx->config.current_precision ? "Fixed-Point" : "Floating-Point",
        ctx->config.debug_level,
        ctx->config.enable_fft,
        ctx->config.enable_lms,
        ctx->config.enable_resampling,
        ctx->config.enable_vad,
        ctx->config.target_sample_rate);
    
    mutex_unlock(&ctx->lock);
    
    if (len > count) len = count;
    
    if (copy_to_user(buf, config_str, len)) {
        return -EFAULT;
    }
    
    *ppos = len;
    return len;
}

static ssize_t dsp_proc_metrics_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    struct dsp_context *ctx = dsp_ctx;
    char metrics_str[512];
    int len;
    
    if (*ppos > 0) return 0;
    
    if (!ctx) return -EINVAL;
    
    mutex_lock(&ctx->lock);
    
    len = snprintf(metrics_str, sizeof(metrics_str),
        "DSP Processing Metrics:\n"
        "====================\n"
        "Processing Time:  %ld ms\n"
        "CPU Usage:        %ld%%\n"
        "SNR:              %ld.%02ld dB\n"
        "Latency:          %ld%%\n"
        "Precision Mode:   %s\n"
        "Voice Activity:   %d\n"
        "Signal Energy:    %d\n"
        "Resample Ratio:   %d%%\n"
        "Buffer Ready:     %d\n"
        "Processing:       %d\n",
        ctx->metrics.processing_time,
        ctx->metrics.cpu_usage,
        ctx->metrics.snr / 100, ctx->metrics.snr % 100,
        ctx->metrics.latency,
        ctx->metrics.precision ? "Fixed-Point" : "Floating-Point",
        ctx->metrics.voice_activity,
        ctx->metrics.signal_energy,
        ctx->metrics.resample_ratio,
        ctx->buffer_ready,
        ctx->processing);
    
    mutex_unlock(&ctx->lock);
    
    if (len > count) len = count;
    
    if (copy_to_user(buf, metrics_str, len)) {
        return -EFAULT;
    }
    
    *ppos = len;
    return len;
}

static ssize_t dsp_proc_status_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    struct dsp_context *ctx = dsp_ctx;
    char status_str[256];
    int len;
    
    if (*ppos > 0) return 0;
    
    if (!ctx) return -EINVAL;
    
    mutex_lock(&ctx->lock);
    
    len = snprintf(status_str, sizeof(status_str),
        "DSP Module Status:\n"
        "================\n"
        "Input Buffer:     %zu samples\n"
        "Output Buffer:    %zu samples\n"
        "Sample Rate:      %u Hz\n"
        "FIFO Metrics:     %u entries\n",
        ctx->input_buf.size,
        ctx->output_buf.size,
        ctx->input_buf.sample_rate,
        kfifo_len(&ctx->metric_fifo));
    
    mutex_unlock(&ctx->lock);
    
    if (len > count) len = count;
    
    if (copy_to_user(buf, status_str, len)) {
        return -EFAULT;
    }
    
    *ppos = len;
    return len;
}

static ssize_t dsp_proc_advanced_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    struct dsp_context *ctx = dsp_ctx;
    char *advanced_str;
    int len, i;
    ssize_t ret;
    
    if (*ppos > 0) return 0;
    
    if (!ctx) return -EINVAL;
    
    advanced_str = kmalloc(1024, GFP_KERNEL);
    if (!advanced_str) return -ENOMEM;
    
    mutex_lock(&ctx->lock);
    
    len = snprintf(advanced_str, 1024,
        "Advanced DSP Features:\n"
        "====================\n"
        "FFT Analysis:      %s (%d-point)\n"
        "LMS Filtering:     %s (%d taps)\n"
        "Resampling:        %s (%d Hz -> %d Hz)\n"
        "Echo Cancellation: %s\n"
        "Voice Activity:    %s (Current: %d)\n"
        "Signal Energy:     %d\n"
        "Resample Ratio:    %d%%\n\n"
        "FFT Bin Energies (first 8):\n",
        ctx->config.enable_fft ? "Enabled" : "Disabled", ctx->config.fft_size,
        ctx->config.enable_lms ? "Enabled" : "Disabled", ctx->config.lms_filter_length,
        ctx->config.enable_resampling ? "Enabled" : "Disabled", 
        ctx->input_buf.sample_rate, ctx->config.target_sample_rate,
        ctx->config.enable_echo_cancellation ? "Enabled" : "Disabled",
        ctx->config.enable_vad ? "Enabled" : "Disabled", ctx->metrics.voice_activity,
        ctx->metrics.signal_energy, ctx->metrics.resample_ratio);
    
    for (i = 0; i < 8 && len < 1024 - 50; i++) {
        len += snprintf(advanced_str + len, 1024 - len,
                       "Bin %d: %d\n", i, ctx->metrics.fft_bin_energy[i]);
    }
    
    mutex_unlock(&ctx->lock);
    
    if (len > count) len = count;
    
    if (copy_to_user(buf, advanced_str, len)) {
        ret = -EFAULT;
    } else {
        *ppos = len;
        ret = len;
    }
    
    kfree(advanced_str);
    return ret;
}

static const struct proc_ops dsp_proc_config_fops = {
    .proc_read = dsp_proc_config_read,
    .proc_lseek = default_llseek,
};

static const struct proc_ops dsp_proc_metrics_fops = {
    .proc_read = dsp_proc_metrics_read,
    .proc_lseek = default_llseek,
};

static const struct proc_ops dsp_proc_status_fops = {
    .proc_read = dsp_proc_status_read,
    .proc_lseek = default_llseek,
};

static const struct proc_ops dsp_proc_advanced_fops = {
    .proc_read = dsp_proc_advanced_read,
    .proc_lseek = default_llseek,
};

int dsp_procfs_init(void) {
    dsp_proc_dir = proc_mkdir("adaptive_dsp", NULL);
    if (!dsp_proc_dir) {
        return -ENOMEM;
    }
    
    dsp_proc_config = proc_create("config", 0444, dsp_proc_dir, &dsp_proc_config_fops);
    dsp_proc_metrics = proc_create("metrics", 0444, dsp_proc_dir, &dsp_proc_metrics_fops);
    dsp_proc_status = proc_create("status", 0444, dsp_proc_dir, &dsp_proc_status_fops);
    dsp_proc_advanced = proc_create("advanced", 0444, dsp_proc_dir, &dsp_proc_advanced_fops);
    
    if (!dsp_proc_config || !dsp_proc_metrics || !dsp_proc_status || !dsp_proc_advanced) {
        dsp_procfs_cleanup();
        return -ENOMEM;
    }
    
    return 0;
}

void dsp_procfs_cleanup(void) {
    if (dsp_proc_config) proc_remove(dsp_proc_config);
    if (dsp_proc_metrics) proc_remove(dsp_proc_metrics);
    if (dsp_proc_status) proc_remove(dsp_proc_status);
    if (dsp_proc_advanced) proc_remove(dsp_proc_advanced);
    if (dsp_proc_dir) proc_remove(dsp_proc_dir);
}
