#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include "dsp_user.h"  // CHANGED: Use user-space header

#define DEVICE_PATH "/dev/adaptive_dsp"

void test_basic_features(int fd) {
    struct dsp_config config;
    struct dsp_metrics metrics;
    int16_t test_data[4410];
    int i, ret;
    
    printf("=== Testing Basic DSP Features ===\n");
    
    // Generate test signal (440 Hz sine wave)
    for (i = 0; i < 4410; i++) {
        test_data[i] = (int16_t)(32767 * sin(2 * M_PI * 440.0 * i / 44100.0));
    }
    
    // Write test data
    ret = write(fd, test_data, sizeof(test_data));
    printf("Written %d bytes of test data\n", ret);
    
    sleep(1);
    
    // Read metrics
    ioctl(fd, DSP_IOCTL_GET_METRICS, &metrics);
    printf("Processing Time: %ld ms\n", metrics.processing_time);
    printf("SNR: %ld.%02ld dB\n", metrics.snr / 100, metrics.snr % 100);
    printf("Voice Activity: %d\n", metrics.voice_activity);
}

void test_advanced_features(int fd) {
    struct dsp_config config;
    struct dsp_metrics metrics;
    struct lms_params lms_params;
    int16_t test_data[8820]; // 0.2 seconds
    int i, ret;
    
    printf("\n=== Testing Advanced DSP Features ===\n");
    
    // Configure advanced features
    ioctl(fd, DSP_IOCTL_GET_CONFIG, &config);
    config.enable_fft = 1;
    config.enable_vad = 1;
    config.enable_resampling = 1;
    config.target_sample_rate = 16000;
    config.fft_size = 512;
    config.enable_lms = 1;
    config.lms_filter_length = 32;
    config.lms_step_size = 0.01;
    ioctl(fd, DSP_IOCTL_SET_CONFIG, &config);
    
    // Generate complex test signal (speech-like)
    for (i = 0; i < 8820; i++) {
        if (i < 4410) {
            // First half: "speech" with modulation
            test_data[i] = (int16_t)(25000 * 
                sin(2 * M_PI * 200.0 * i / 44100.0) *
                sin(2 * M_PI * 5.0 * i / 44100.0));
        } else {
            // Second half: background noise
            test_data[i] = (int16_t)(5000 * sin(2 * M_PI * 1000.0 * i / 44100.0));
        }
    }
    
    // Process with advanced features
    ret = write(fd, test_data, sizeof(test_data));
    printf("Advanced processing: wrote %d bytes\n", ret);
    
    sleep(1);
    
    // Get advanced metrics
    ioctl(fd, DSP_IOCTL_GET_METRICS, &metrics);
    
    printf("Advanced Metrics:\n");
    printf("Voice Activity: %d\n", metrics.voice_activity);
    printf("Signal Energy: %d\n", metrics.signal_energy);
    printf("Resample Ratio: %d%%\n", metrics.resample_ratio);
    printf("FFT Bin 0 Energy: %d\n", metrics.fft_bin_energy[0]);
    printf("Processing Time: %ld ms\n", metrics.processing_time);
}

void test_echo_cancellation(int fd) {
    printf("\n=== Testing Echo Cancellation ===\n");
    
    // Note: This would require separate playback and recording buffers
    // For demonstration, we just enable the feature
    struct dsp_config config;
    ioctl(fd, DSP_IOCTL_GET_CONFIG, &config);
    config.enable_echo_cancellation = 1;
    ioctl(fd, DSP_IOCTL_SET_CONFIG, &config);
    
    printf("Echo cancellation feature enabled\n");
    printf("(Note: Full echo cancellation requires separate playback/record buffers)\n");
}

int main() {
    int fd;
    
    printf("Extended Adaptive DSP Kernel Module Test\n");
    printf("========================================\n");
    
    // Open device
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        printf("Make sure the kernel module is loaded: sudo insmod adaptive_dsp.ko\n");
        return EXIT_FAILURE;
    }
    
    printf("Device opened successfully\n");
    
    // Run tests
    test_basic_features(fd);
    test_advanced_features(fd);
    test_echo_cancellation(fd);
    
    // Close device
    close(fd);
    
    printf("\n=== Checking ProcFS Interface ===\n");
    system("echo 'Config:' && cat /proc/adaptive_dsp/config");
    system("echo -e '\\nMetrics:' && cat /proc/adaptive_dsp/metrics");
    system("echo -e '\\nAdvanced:' && cat /proc/adaptive_dsp/advanced");
    
    printf("\n=== Test Completed Successfully ===\n");
    return EXIT_SUCCESS;
}
