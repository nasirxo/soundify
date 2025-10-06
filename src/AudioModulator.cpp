#include "AudioModulator.h"
#include <cmath>
#include <algorithm>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

AudioModulator::AudioModulator(int sampleRate) 
    : sampleRate(sampleRate) {
    // Each symbol is 30ms for faster transmission (was 50ms)
    symbolDuration = 0.03;
    samplesPerSymbol = static_cast<int>(sampleRate * symbolDuration);
}

AudioModulator::~AudioModulator() {}

std::vector<float> AudioModulator::generatePreamble() {
    // Generate a distinctive preamble for synchronization
    // Uses alternating sync frequency and silence
    std::vector<float> preamble;
    
    // Repeat sync tone 5 times for reliable detection
    for (int i = 0; i < 5; i++) {
        std::vector<float> syncTone = generateTone(SYNC_FREQ, samplesPerSymbol);
        preamble.insert(preamble.end(), syncTone.begin(), syncTone.end());
    }
    
    return preamble;
}

std::vector<float> AudioModulator::generateTone(double frequency, int numSamples) {
    std::vector<float> tone(numSamples);
    
    for (int i = 0; i < numSamples; i++) {
        double t = static_cast<double>(i) / sampleRate;
        tone[i] = 0.7f * std::sin(2.0 * M_PI * frequency * t);
    }
    
    // Apply envelope to reduce clicking
    int rampSamples = numSamples / 10;
    for (int i = 0; i < rampSamples; i++) {
        float envelope = static_cast<float>(i) / rampSamples;
        tone[i] *= envelope;
        tone[numSamples - 1 - i] *= envelope;
    }
    
    return tone;
}

std::vector<float> AudioModulator::modulate(const std::vector<uint8_t>& data) {
    std::vector<float> samples;
    
    // Add preamble for synchronization
    std::vector<float> preamble = generatePreamble();
    samples.insert(samples.end(), preamble.begin(), preamble.end());
    
    // Add data length (4 bytes)
    uint32_t dataLength = data.size();
    for (int i = 0; i < 4; i++) {
        uint8_t byte = (dataLength >> (i * 8)) & 0xFF;
        
        // Each byte is now 1 symbol (8 bits per symbol - 256-FSK)
        double frequency = BASE_FREQ + byte * FREQ_SPACING;
        std::vector<float> tone = generateTone(frequency, samplesPerSymbol);
        samples.insert(samples.end(), tone.begin(), tone.end());
    }
    
    // Encode data - each byte is one symbol now!
    for (uint8_t byte : data) {
        double frequency = BASE_FREQ + byte * FREQ_SPACING;
        std::vector<float> tone = generateTone(frequency, samplesPerSymbol);
        samples.insert(samples.end(), tone.begin(), tone.end());
    }
    
    // Add ending preamble
    samples.insert(samples.end(), preamble.begin(), preamble.end());
    
    return samples;
}

double AudioModulator::goertzelFilter(const std::vector<float>& samples, int startIdx, double frequency) {
    double omega = 2.0 * M_PI * frequency / sampleRate;
    double coeff = 2.0 * std::cos(omega);
    
    double q0 = 0.0, q1 = 0.0, q2 = 0.0;
    
    int endIdx = std::min(startIdx + samplesPerSymbol, (int)samples.size());
    
    for (int i = startIdx; i < endIdx; i++) {
        q0 = coeff * q1 - q2 + samples[i];
        q2 = q1;
        q1 = q0;
    }
    
    // Calculate magnitude
    double real = q1 - q2 * std::cos(omega);
    double imag = q2 * std::sin(omega);
    double magnitude = std::sqrt(real * real + imag * imag);
    
    return magnitude;
}

int AudioModulator::detectTone(const std::vector<float>& samples, int startIdx) {
    double maxMagnitude = 0.0;
    int detectedTone = -1;
    
    // Check all possible tones
    for (int tone = 0; tone < NUM_TONES; tone++) {
        double frequency = BASE_FREQ + tone * FREQ_SPACING;
        double magnitude = goertzelFilter(samples, startIdx, frequency);
        
        if (magnitude > maxMagnitude) {
            maxMagnitude = magnitude;
            detectedTone = tone;
        }
    }
    
    return detectedTone;
}

std::vector<int> AudioModulator::findPreamble(const std::vector<float>& samples) {
    std::vector<int> positions;
    
    // Search for sync frequency pattern
    for (size_t i = 0; i < samples.size() - samplesPerSymbol * 5; i += samplesPerSymbol / 2) {
        int matchCount = 0;
        
        // Check for 5 consecutive sync tones
        for (int j = 0; j < 5; j++) {
            double magnitude = goertzelFilter(samples, i + j * samplesPerSymbol, SYNC_FREQ);
            
            // Check if magnitude is strong enough
            if (magnitude > 10.0) { // Threshold for sync detection
                matchCount++;
            }
        }
        
        if (matchCount >= 4) { // Allow 1 miss
            positions.push_back(i + 5 * samplesPerSymbol); // Position after preamble
            i += 5 * samplesPerSymbol; // Skip past this preamble
        }
    }
    
    return positions;
}

std::vector<uint8_t> AudioModulator::demodulate(const std::vector<float>& samples) {
    std::vector<uint8_t> data;
    
    // Find preamble
    std::vector<int> preamblePositions = findPreamble(samples);
    
    if (preamblePositions.empty()) {
        std::cerr << "Error: No preamble found in audio!" << std::endl;
        return data;
    }
    
    int startPos = preamblePositions[0];
    
    // Read data length (4 bytes = 4 symbols now with 256-FSK)
    uint32_t dataLength = 0;
    for (int i = 0; i < 4; i++) {
        if (startPos + (i + 1) * samplesPerSymbol > (int)samples.size()) {
            std::cerr << "Error: Audio too short to read length!" << std::endl;
            return data;
        }
        
        int tone = detectTone(samples, startPos + i * samplesPerSymbol);
        if (tone < 0 || tone >= NUM_TONES) {
            std::cerr << "Error: Invalid tone detected!" << std::endl;
            return data;
        }
        
        uint8_t byte = static_cast<uint8_t>(tone);
        dataLength |= (static_cast<uint32_t>(byte) << (i * 8));
    }
    
    std::cout << "Detected data length: " << dataLength << " bytes" << std::endl;
    
    // Read data - each symbol is now a full byte
    startPos += 4 * samplesPerSymbol; // Skip length bytes
    
    for (uint32_t i = 0; i < dataLength; i++) {
        if (startPos + samplesPerSymbol > (int)samples.size()) {
            std::cerr << "Warning: Audio ended prematurely. Decoded " << i << " of " << dataLength << " bytes." << std::endl;
            break;
        }
        
        int tone = detectTone(samples, startPos);
        
        if (tone < 0 || tone >= NUM_TONES) {
            std::cerr << "Warning: Invalid tone at byte " << i << std::endl;
            tone = 0; // Default to 0
        }
        
        data.push_back(static_cast<uint8_t>(tone));
        startPos += samplesPerSymbol;
    }
    
    return data;
}
