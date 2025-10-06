#ifndef AUDIO_MODULATOR_H
#define AUDIO_MODULATOR_H

#include <vector>
#include <cstdint>
#include <complex>

/**
 * @brief Multi-tone FSK (Frequency Shift Keying) modulator/demodulator
 * 
 * Uses multiple frequency tones to encode data into audible sound.
 * Includes synchronization signals and noise filtering for robust decoding.
 */
class AudioModulator {
public:
    AudioModulator(int sampleRate = 44100);
    ~AudioModulator();

    /**
     * @brief Modulate binary data into audio samples
     * @param data Binary data to modulate
     * @return Audio samples as float values (-1.0 to 1.0)
     */
    std::vector<float> modulate(const std::vector<uint8_t>& data);

    /**
     * @brief Demodulate audio samples into binary data
     * @param samples Audio samples to demodulate
     * @return Demodulated binary data
     */
    std::vector<uint8_t> demodulate(const std::vector<float>& samples);

    int getSampleRate() const { return sampleRate; }

private:
    int sampleRate;
    double symbolDuration;      // Duration of each symbol in seconds
    int samplesPerSymbol;       // Number of samples per symbol
    
    // Frequency configuration for 256-FSK (8 bits per symbol) - MUCH FASTER!
    static constexpr int NUM_TONES = 256;
    static constexpr double BASE_FREQ = 2000.0;  // Start frequency (Hz)
    static constexpr double FREQ_SPACING = 50.0; // Frequency spacing (Hz)
    static constexpr double SYNC_FREQ = 1000.0;    // Synchronization frequency
    
    // Helper functions
    std::vector<float> generatePreamble();
    std::vector<float> generateTone(double frequency, int numSamples);
    int detectTone(const std::vector<float>& samples, int startIdx);
    double goertzelFilter(const std::vector<float>& samples, int startIdx, double frequency);
    std::vector<int> findPreamble(const std::vector<float>& samples);
    void applyBandpassFilter(std::vector<float>& samples);
};

#endif // AUDIO_MODULATOR_H
