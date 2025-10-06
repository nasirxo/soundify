#include <iostream>
#include <string>
#include <cstring>
#include "AudioEncoder.h"
#include "AudioDecoder.h"

void printUsage(const char* programName) {
    std::cout << "\n╔═══════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║         Audio Encoder/Decoder - File to Sound Converter          ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════════════════════╝" << std::endl;
    std::cout << "\nConvert any file to audible sound and back!" << std::endl;
    std::cout << "Supports: .txt, .jpg, .png, and any other file format" << std::endl;
    std::cout << "\nUSAGE:" << std::endl;
    std::cout << "  " << programName << " encode <input_file> <output.wav>" << std::endl;
    std::cout << "  " << programName << " decode <input.wav> <output_directory>" << std::endl;
    std::cout << "\nEXAMPLES:" << std::endl;
    std::cout << "  Encode a text file:" << std::endl;
    std::cout << "    " << programName << " encode document.txt output.wav" << std::endl;
    std::cout << "\n  Encode an image:" << std::endl;
    std::cout << "    " << programName << " encode photo.jpg output.wav" << std::endl;
    std::cout << "\n  Decode back to original file:" << std::endl;
    std::cout << "    " << programName << " decode output.wav ./" << std::endl;
    std::cout << "\nFEATURES:" << std::endl;
    std::cout << "  ✓ Encodes filename and extension automatically" << std::endl;
    std::cout << "  ✓ Reed-Solomon error correction for noise resistance" << std::endl;
    std::cout << "  ✓ 16-FSK modulation for robust audio transmission" << std::endl;
    std::cout << "  ✓ Works even after recording with phone microphone" << std::endl;
    std::cout << "  ✓ CRC32 checksum for data integrity verification" << std::endl;
    std::cout << "\nNOTES:" << std::endl;
    std::cout << "  - Generated audio is audible (1-2.5 kHz range)" << std::endl;
    std::cout << "  - For best results, play audio at moderate volume" << std::endl;
    std::cout << "  - Decoding works with phone recordings in quiet environments" << std::endl;
    std::cout << "\n";
}

void printBanner() {
    std::cout << "\n";
    std::cout << "  ╔═══════════════════════════════════════╗\n";
    std::cout << "  ║   Audio Encoder/Decoder v1.0.0        ║\n";
    std::cout << "  ║   Converting Files to Sound Waves     ║\n";
    std::cout << "  ╚═══════════════════════════════════════╝\n";
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    // Check arguments
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string command = argv[1];
    
    // Help command
    if (command == "help" || command == "--help" || command == "-h") {
        printUsage(argv[0]);
        return 0;
    }
    
    // Encode command
    if (command == "encode") {
        if (argc != 4) {
            std::cerr << "Error: Invalid number of arguments for encode command" << std::endl;
            printUsage(argv[0]);
            return 1;
        }
        
        printBanner();
        
        std::string inputFile = argv[2];
        std::string outputFile = argv[3];
        
        AudioEncoder encoder;
        if (encoder.encodeFile(inputFile, outputFile)) {
            std::cout << "\n✓ Success! File encoded to audio." << std::endl;
            std::cout << "You can now play the audio file or record it with your phone." << std::endl;
            return 0;
        } else {
            std::cerr << "\n✗ Encoding failed!" << std::endl;
            return 1;
        }
    }
    
    // Decode command
    else if (command == "decode") {
        if (argc != 4) {
            std::cerr << "Error: Invalid number of arguments for decode command" << std::endl;
            printUsage(argv[0]);
            return 1;
        }
        
        printBanner();
        
        std::string inputFile = argv[2];
        std::string outputDir = argv[3];
        
        AudioDecoder decoder;
        if (decoder.decodeFile(inputFile, outputDir)) {
            std::cout << "\n✓ Success! Audio decoded back to original file." << std::endl;
            return 0;
        } else {
            std::cerr << "\n✗ Decoding failed!" << std::endl;
            std::cerr << "Make sure the audio file is valid and not too corrupted." << std::endl;
            return 1;
        }
    }
    
    // Unknown command
    else {
        std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    return 0;
}
