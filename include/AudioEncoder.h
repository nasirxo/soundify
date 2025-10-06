#ifndef AUDIO_ENCODER_H
#define AUDIO_ENCODER_H

#include <string>
#include <vector>
#include <cstdint>
#include "AudioModulator.h"
#include "ErrorCorrection.h"
#include "WavFile.h"

/**
 * @brief Main encoder class for converting files to audio
 */
class AudioEncoder {
public:
    AudioEncoder();
    ~AudioEncoder();

    /**
     * @brief Encode a file into an audio file
     * @param inputFile Path to input file (.txt, .jpg, .png, etc.)
     * @param outputFile Path to output audio file (.wav)
     * @return true if successful, false otherwise
     */
    bool encodeFile(const std::string& inputFile, const std::string& outputFile);

private:
    AudioModulator modulator;
    ErrorCorrection errorCorrection;
    WavFile wavFile;

    std::vector<uint8_t> readInputFile(const std::string& filename);
    std::vector<uint8_t> createDataPacket(const std::string& filename, 
                                          const std::vector<uint8_t>& fileData);
    std::string extractFileName(const std::string& path);
};

#endif // AUDIO_ENCODER_H
