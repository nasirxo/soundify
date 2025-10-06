#include "AudioDecoder.h"
#include <fstream>
#include <iostream>
#include <cstring>

AudioDecoder::AudioDecoder() {}

AudioDecoder::~AudioDecoder() {}

bool AudioDecoder::parseDataPacket(const std::vector<uint8_t>& packet,
                                   std::string& filename,
                                   std::vector<uint8_t>& fileData) {
    if (packet.size() < 14) { // Minimum packet size
        std::cerr << "Error: Packet too small" << std::endl;
        return false;
    }
    
    size_t pos = 0;
    
    // Verify magic number
    if (packet[pos++] != 'A' || packet[pos++] != 'E' || 
        packet[pos++] != 'D' || packet[pos++] != 'C') {
        std::cerr << "Error: Invalid magic number" << std::endl;
        return false;
    }
    
    // Read filename length
    uint8_t filenameLen = packet[pos++];
    
    if (pos + filenameLen + 4 > packet.size()) {
        std::cerr << "Error: Invalid filename length" << std::endl;
        return false;
    }
    
    // Read filename
    filename.clear();
    for (size_t i = 0; i < filenameLen; i++) {
        filename += static_cast<char>(packet[pos++]);
    }
    
    // Read file data length
    uint32_t fileDataLen = packet[pos++];
    fileDataLen |= (static_cast<uint32_t>(packet[pos++]) << 8);
    fileDataLen |= (static_cast<uint32_t>(packet[pos++]) << 16);
    fileDataLen |= (static_cast<uint32_t>(packet[pos++]) << 24);
    
    if (pos + fileDataLen + 4 > packet.size()) {
        std::cerr << "Error: Invalid file data length" << std::endl;
        return false;
    }
    
    // Read file data
    fileData.clear();
    fileData.insert(fileData.end(), packet.begin() + pos, packet.begin() + pos + fileDataLen);
    pos += fileDataLen;
    
    // Read and verify CRC32
    uint32_t storedCrc = packet[pos++];
    storedCrc |= (static_cast<uint32_t>(packet[pos++]) << 8);
    storedCrc |= (static_cast<uint32_t>(packet[pos++]) << 16);
    storedCrc |= (static_cast<uint32_t>(packet[pos++]) << 24);
    
    // Calculate CRC32 of packet (excluding CRC32 itself)
    std::vector<uint8_t> packetForCrc(packet.begin(), packet.begin() + pos - 4);
    uint32_t calculatedCrc = ErrorCorrection::calculateCRC32(packetForCrc);
    
    if (storedCrc != calculatedCrc) {
        std::cerr << "Warning: CRC32 mismatch! Stored: 0x" << std::hex << storedCrc 
                  << ", Calculated: 0x" << calculatedCrc << std::dec << std::endl;
        std::cerr << "Data may be corrupted, but attempting to save anyway..." << std::endl;
    } else {
        std::cout << "✓ CRC32 verified: 0x" << std::hex << calculatedCrc << std::dec << std::endl;
    }
    
    std::cout << "Parsed packet:" << std::endl;
    std::cout << "  Filename: " << filename << std::endl;
    std::cout << "  File size: " << fileDataLen << " bytes" << std::endl;
    
    return true;
}

bool AudioDecoder::writeOutputFile(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not create output file: " << path << std::endl;
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();
    
    std::cout << "Wrote " << data.size() << " bytes to " << path << std::endl;
    return true;
}

bool AudioDecoder::decodeFile(const std::string& inputFile, const std::string& outputDir) {
    std::cout << "\n=== DECODING ===" << std::endl;
    std::cout << "Input file: " << inputFile << std::endl;
    std::cout << "Output directory: " << outputDir << std::endl;
    
    // Read WAV file
    std::vector<float> audioSamples;
    int sampleRate, channels;
    
    std::cout << "\nReading WAV file..." << std::endl;
    if (!wavFile.read(inputFile, audioSamples, sampleRate, channels)) {
        return false;
    }
    
    // Convert stereo to mono if necessary
    if (channels == 2) {
        std::cout << "Converting stereo to mono..." << std::endl;
        std::vector<float> mono;
        for (size_t i = 0; i < audioSamples.size(); i += 2) {
            mono.push_back((audioSamples[i] + audioSamples[i + 1]) / 2.0f);
        }
        audioSamples = mono;
    }
    
    // Demodulate audio
    std::cout << "\nDemodulating audio..." << std::endl;
    std::vector<uint8_t> encodedData = modulator.demodulate(audioSamples);
    
    if (encodedData.empty()) {
        std::cerr << "Error: Failed to demodulate audio" << std::endl;
        return false;
    }
    
    std::cout << "Demodulated " << encodedData.size() << " bytes" << std::endl;
    
    // Apply error correction
    std::cout << "\nApplying error correction..." << std::endl;
    std::vector<uint8_t> decodedData = errorCorrection.decode(encodedData);
    
    if (decodedData.empty()) {
        std::cerr << "Error: Failed to decode data (too many errors)" << std::endl;
        return false;
    }
    
    std::cout << "Decoded " << decodedData.size() << " bytes" << std::endl;
    
    // Parse data packet
    std::string filename;
    std::vector<uint8_t> fileData;
    
    std::cout << "\nParsing data packet..." << std::endl;
    if (!parseDataPacket(decodedData, filename, fileData)) {
        std::cerr << "Error: Failed to parse data packet" << std::endl;
        return false;
    }
    
    // Construct output path
    std::string outputPath = outputDir;
    if (!outputPath.empty() && outputPath.back() != '/' && outputPath.back() != '\\') {
        outputPath += "/";
    }
    outputPath += filename;
    
    // Write output file
    std::cout << "\nWriting output file..." << std::endl;
    if (!writeOutputFile(outputPath, fileData)) {
        return false;
    }
    
    std::cout << "\n✓ Decoding complete!" << std::endl;
    std::cout << "Output file: " << outputPath << std::endl;
    
    return true;
}
