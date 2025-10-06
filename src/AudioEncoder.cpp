#include "AudioEncoder.h"
#include <fstream>
#include <iostream>
#include <cstring>

AudioEncoder::AudioEncoder() {}

AudioEncoder::~AudioEncoder() {}

std::string AudioEncoder::extractFileName(const std::string& path) {
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        return path.substr(lastSlash + 1);
    }
    return path;
}

std::vector<uint8_t> AudioEncoder::readInputFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open input file: " << filename << std::endl;
        return std::vector<uint8_t>();
    }
    
    // Get file size
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Read file data
    std::vector<uint8_t> data(fileSize);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    file.close();
    
    std::cout << "Read " << fileSize << " bytes from " << filename << std::endl;
    return data;
}

std::vector<uint8_t> AudioEncoder::createDataPacket(const std::string& filename, 
                                                     const std::vector<uint8_t>& fileData) {
    std::vector<uint8_t> packet;
    
    // Extract just the filename (not full path)
    std::string baseFilename = extractFileName(filename);
    
    // Packet format:
    // [4 bytes: Magic number "AEDC"]
    // [1 byte: Filename length]
    // [N bytes: Filename]
    // [4 bytes: File data length]
    // [M bytes: File data]
    // [4 bytes: CRC32 checksum]
    
    // Magic number
    packet.push_back('A');
    packet.push_back('E');
    packet.push_back('D');
    packet.push_back('C');
    
    // Filename length and filename
    uint8_t filenameLen = std::min((size_t)255, baseFilename.length());
    packet.push_back(filenameLen);
    
    for (size_t i = 0; i < filenameLen; i++) {
        packet.push_back(static_cast<uint8_t>(baseFilename[i]));
    }
    
    // File data length
    uint32_t fileDataLen = fileData.size();
    packet.push_back((fileDataLen >> 0) & 0xFF);
    packet.push_back((fileDataLen >> 8) & 0xFF);
    packet.push_back((fileDataLen >> 16) & 0xFF);
    packet.push_back((fileDataLen >> 24) & 0xFF);
    
    // File data
    packet.insert(packet.end(), fileData.begin(), fileData.end());
    
    // Calculate CRC32 for integrity check
    uint32_t crc = ErrorCorrection::calculateCRC32(packet);
    packet.push_back((crc >> 0) & 0xFF);
    packet.push_back((crc >> 8) & 0xFF);
    packet.push_back((crc >> 16) & 0xFF);
    packet.push_back((crc >> 24) & 0xFF);
    
    std::cout << "Created data packet: " << packet.size() << " bytes" << std::endl;
    std::cout << "  Filename: " << baseFilename << " (" << (int)filenameLen << " bytes)" << std::endl;
    std::cout << "  File data: " << fileDataLen << " bytes" << std::endl;
    std::cout << "  CRC32: 0x" << std::hex << crc << std::dec << std::endl;
    
    return packet;
}

bool AudioEncoder::encodeFile(const std::string& inputFile, const std::string& outputFile) {
    std::cout << "\n=== ENCODING ===" << std::endl;
    std::cout << "Input file: " << inputFile << std::endl;
    std::cout << "Output file: " << outputFile << std::endl;
    
    // Read input file
    std::vector<uint8_t> fileData = readInputFile(inputFile);
    if (fileData.empty()) {
        return false;
    }
    
    // Create data packet with metadata
    std::vector<uint8_t> packet = createDataPacket(inputFile, fileData);
    
    // Apply error correction
    std::cout << "\nApplying error correction..." << std::endl;
    std::vector<uint8_t> encodedData = errorCorrection.encode(packet);
    std::cout << "Encoded data size: " << encodedData.size() << " bytes" << std::endl;
    
    // Modulate to audio
    std::cout << "\nModulating to audio..." << std::endl;
    std::vector<float> audioSamples = modulator.modulate(encodedData);
    
    double duration = static_cast<double>(audioSamples.size()) / modulator.getSampleRate();
    std::cout << "Audio duration: " << duration << " seconds" << std::endl;
    std::cout << "Audio samples: " << audioSamples.size() << std::endl;
    
    // Write WAV file
    std::cout << "\nWriting WAV file..." << std::endl;
    if (!wavFile.write(outputFile, audioSamples, modulator.getSampleRate(), 1)) {
        return false;
    }
    
    std::cout << "\n✓ Encoding complete!" << std::endl;
    return true;
}
