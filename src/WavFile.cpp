#include "WavFile.h"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <iostream>

WavFile::WavFile() {}

WavFile::~WavFile() {}

void WavFile::prepareHeader(WavHeader& header, int numSamples, int sampleRate, int channels) {
    // RIFF header
    std::memcpy(header.riff, "RIFF", 4);
    header.fileSize = 36 + numSamples * channels * 2; // 2 bytes per sample (16-bit)
    std::memcpy(header.wave, "WAVE", 4);
    
    // Format chunk
    std::memcpy(header.fmt, "fmt ", 4);
    header.fmtSize = 16;
    header.audioFormat = 1; // PCM
    header.numChannels = channels;
    header.sampleRate = sampleRate;
    header.bitsPerSample = 16;
    header.byteRate = sampleRate * channels * 2;
    header.blockAlign = channels * 2;
    
    // Data chunk
    std::memcpy(header.data, "data", 4);
    header.dataSize = numSamples * channels * 2;
}

bool WavFile::write(const std::string& filename, 
                    const std::vector<float>& samples,
                    int sampleRate,
                    int channels) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << filename << std::endl;
        return false;
    }
    
    // Prepare header
    WavHeader header;
    prepareHeader(header, samples.size(), sampleRate, channels);
    
    // Write header
    file.write(reinterpret_cast<char*>(&header), sizeof(WavHeader));
    
    // Convert float samples to 16-bit PCM and write
    for (float sample : samples) {
        // Clamp sample to [-1.0, 1.0]
        float clampedSample = std::max(-1.0f, std::min(1.0f, sample));
        
        // Convert to 16-bit signed integer
        int16_t pcmSample = static_cast<int16_t>(clampedSample * 32767.0f);
        
        file.write(reinterpret_cast<char*>(&pcmSample), sizeof(int16_t));
    }
    
    file.close();
    std::cout << "Wrote " << samples.size() << " samples to " << filename << std::endl;
    return true;
}

bool WavFile::read(const std::string& filename,
                   std::vector<float>& samples,
                   int& sampleRate,
                   int& channels) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for reading: " << filename << std::endl;
        return false;
    }
    
    // Read header
    WavHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(WavHeader));
    
    // Verify RIFF and WAVE
    if (std::memcmp(header.riff, "RIFF", 4) != 0 || 
        std::memcmp(header.wave, "WAVE", 4) != 0) {
        std::cerr << "Error: Invalid WAV file format" << std::endl;
        return false;
    }
    
    // Check for PCM format
    if (header.audioFormat != 1) {
        std::cerr << "Error: Only PCM format is supported" << std::endl;
        return false;
    }
    
    sampleRate = header.sampleRate;
    channels = header.numChannels;
    
    // Calculate number of samples
    int numSamples = header.dataSize / (channels * (header.bitsPerSample / 8));
    samples.clear();
    samples.reserve(numSamples);
    
    // Read samples based on bit depth
    if (header.bitsPerSample == 16) {
        for (int i = 0; i < numSamples; i++) {
            int16_t pcmSample;
            file.read(reinterpret_cast<char*>(&pcmSample), sizeof(int16_t));
            
            // Convert to float [-1.0, 1.0]
            float sample = static_cast<float>(pcmSample) / 32768.0f;
            samples.push_back(sample);
        }
    } else if (header.bitsPerSample == 8) {
        for (int i = 0; i < numSamples; i++) {
            uint8_t pcmSample;
            file.read(reinterpret_cast<char*>(&pcmSample), sizeof(uint8_t));
            
            // Convert to float [-1.0, 1.0] (8-bit is unsigned, centered at 128)
            float sample = (static_cast<float>(pcmSample) - 128.0f) / 128.0f;
            samples.push_back(sample);
        }
    } else {
        std::cerr << "Error: Unsupported bit depth: " << header.bitsPerSample << std::endl;
        return false;
    }
    
    file.close();
    std::cout << "Read " << samples.size() << " samples from " << filename << std::endl;
    std::cout << "Sample rate: " << sampleRate << " Hz, Channels: " << channels << std::endl;
    
    return true;
}
