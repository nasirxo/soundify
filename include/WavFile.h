#ifndef WAV_FILE_H
#define WAV_FILE_H

#include <string>
#include <vector>
#include <cstdint>

/**
 * @brief WAV file handler for reading and writing audio files
 */
class WavFile {
public:
    WavFile();
    ~WavFile();

    /**
     * @brief Write audio samples to a WAV file
     * @param filename Output file path
     * @param samples Audio sample data (normalized -1.0 to 1.0)
     * @param sampleRate Sample rate in Hz
     * @param channels Number of channels (1 for mono, 2 for stereo)
     * @return true if successful, false otherwise
     */
    bool write(const std::string& filename, 
               const std::vector<float>& samples,
               int sampleRate = 44100,
               int channels = 1);

    /**
     * @brief Read audio samples from a WAV file
     * @param filename Input file path
     * @param samples Output audio sample data (normalized -1.0 to 1.0)
     * @param sampleRate Output sample rate
     * @param channels Output number of channels
     * @return true if successful, false otherwise
     */
    bool read(const std::string& filename,
              std::vector<float>& samples,
              int& sampleRate,
              int& channels);

private:
    struct WavHeader {
        char riff[4];           // "RIFF"
        uint32_t fileSize;      // File size - 8
        char wave[4];           // "WAVE"
        char fmt[4];            // "fmt "
        uint32_t fmtSize;       // Format chunk size (16 for PCM)
        uint16_t audioFormat;   // Audio format (1 for PCM)
        uint16_t numChannels;   // Number of channels
        uint32_t sampleRate;    // Sample rate
        uint32_t byteRate;      // Byte rate
        uint16_t blockAlign;    // Block align
        uint16_t bitsPerSample; // Bits per sample
        char data[4];           // "data"
        uint32_t dataSize;      // Data size
    };

    void prepareHeader(WavHeader& header, int numSamples, int sampleRate, int channels);
};

#endif // WAV_FILE_H
