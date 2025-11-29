#ifndef _DSP_IOCTL_H
#define _DSP_IOCTL_H

#include <linux/ioctl.h>

#define DSP_IOC_MAGIC 'D'

// Basic commands
#define DSP_IOCTL_GET_CONFIG    _IOR(DSP_IOC_MAGIC, 1, struct dsp_config)
#define DSP_IOCTL_SET_CONFIG    _IOW(DSP_IOC_MAGIC, 2, struct dsp_config)
#define DSP_IOCTL_GET_METRICS   _IOR(DSP_IOC_MAGIC, 3, struct dsp_metrics)
#define DSP_IOCTL_RESET_BUFFERS _IO(DSP_IOC_MAGIC, 4)
#define DSP_IOCTL_SET_SAMPLE_RATE _IOW(DSP_IOC_MAGIC, 5, uint32_t)

// Extended commands
#define DSP_IOCTL_SET_FFT_SIZE      _IOW(DSP_IOC_MAGIC, 6, int)
#define DSP_IOCTL_SET_LMS_PARAMS    _IOW(DSP_IOC_MAGIC, 7, struct lms_params)
#define DSP_IOCTL_SET_RESAMPLE_RATE _IOW(DSP_IOC_MAGIC, 8, int)
#define DSP_IOCTL_ENABLE_FFT        _IOW(DSP_IOC_MAGIC, 9, int)
#define DSP_IOCTL_ENABLE_VAD        _IOW(DSP_IOC_MAGIC, 10, int)

struct lms_params {
    int length;
    float step_size;
};

#endif

