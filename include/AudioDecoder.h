#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

#include <string>
#include <vector>
#include <cstdint>
#include "AudioModulator.h"
#include "ErrorCorrection.h"
#include "WavFile.h"

/**
 * @brief Main decoder class for converting audio back to files
 */
class AudioDecoder {
public:
    AudioDecoder();
    ~AudioDecoder();

    /**
     * @brief Decode an audio file back to the original file
     * @param inputFile Path to input audio file (.wav)
     * @param outputDir Directory to save decoded file
     * @return true if successful, false otherwise
     */
    bool decodeFile(const std::string& inputFile, const std::string& outputDir);

private:
    AudioModulator modulator;
    ErrorCorrection errorCorrection;
    WavFile wavFile;

    bool parseDataPacket(const std::vector<uint8_t>& packet,
                        std::string& filename,
                        std::vector<uint8_t>& fileData);
    bool writeOutputFile(const std::string& path, const std::vector<uint8_t>& data);
};

#endif // AUDIO_DECODER_H
