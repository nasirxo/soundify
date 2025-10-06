#include "ErrorCorrection.h"
#include <algorithm>
#include <cstring>

ErrorCorrection::ErrorCorrection() {
    initGaloisField();
}

ErrorCorrection::~ErrorCorrection() {}

void ErrorCorrection::initGaloisField() {
    // Initialize GF(256) with primitive polynomial x^8 + x^4 + x^3 + x^2 + 1 (0x11D)
    gf_exp.resize(512);
    gf_log.resize(256);
    
    int x = 1;
    for (int i = 0; i < 255; i++) {
        gf_exp[i] = x;
        gf_log[x] = i;
        x <<= 1;
        if (x & 0x100) {
            x ^= 0x11D; // Primitive polynomial
        }
    }
    
    // Duplicate the table for convenience
    for (int i = 255; i < 512; i++) {
        gf_exp[i] = gf_exp[i - 255];
    }
    
    gf_log[0] = 0; // Log of 0 is undefined, but we set it to 0 for convenience
}

uint8_t ErrorCorrection::gfMul(uint8_t a, uint8_t b) {
    if (a == 0 || b == 0) return 0;
    return gf_exp[gf_log[a] + gf_log[b]];
}

uint8_t ErrorCorrection::gfDiv(uint8_t a, uint8_t b) {
    if (a == 0) return 0;
    if (b == 0) return 0; // Division by zero
    return gf_exp[(gf_log[a] + 255 - gf_log[b]) % 255];
}

std::vector<uint8_t> ErrorCorrection::gfPolyMul(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    std::vector<uint8_t> result(a.size() + b.size() - 1, 0);
    for (size_t i = 0; i < a.size(); i++) {
        for (size_t j = 0; j < b.size(); j++) {
            result[i + j] ^= gfMul(a[i], b[j]);
        }
    }
    return result;
}

std::vector<uint8_t> ErrorCorrection::rsGeneratorPoly(int nsym) {
    std::vector<uint8_t> g = {1};
    for (int i = 0; i < nsym; i++) {
        std::vector<uint8_t> term = {1, gf_exp[i]};
        g = gfPolyMul(g, term);
    }
    return g;
}

std::vector<uint8_t> ErrorCorrection::rsEncode(const std::vector<uint8_t>& msg, int nsym) {
    std::vector<uint8_t> gen = rsGeneratorPoly(nsym);
    std::vector<uint8_t> result(msg.size() + nsym, 0);
    
    // Copy message
    std::copy(msg.begin(), msg.end(), result.begin());
    
    // Polynomial division
    for (size_t i = 0; i < msg.size(); i++) {
        uint8_t coef = result[i];
        if (coef != 0) {
            for (size_t j = 0; j < gen.size(); j++) {
                result[i + j] ^= gfMul(gen[j], coef);
            }
        }
    }
    
    // Copy parity bytes to the end of original message
    std::vector<uint8_t> encoded = msg;
    encoded.insert(encoded.end(), result.begin() + msg.size(), result.end());
    
    return encoded;
}

std::vector<uint8_t> ErrorCorrection::rsDecode(const std::vector<uint8_t>& msg, int nsym) {
    // Simplified error correction - just detect errors
    // For production, implement full Berlekamp-Massey algorithm
    
    if (msg.size() < (size_t)nsym) {
        return std::vector<uint8_t>(); // Invalid message
    }
    
    // Extract original message (remove parity bytes)
    std::vector<uint8_t> decoded(msg.begin(), msg.end() - nsym);
    
    // Verify with re-encoding
    std::vector<uint8_t> reencoded = rsEncode(decoded, nsym);
    
    // Simple error detection - count differences
    int errors = 0;
    for (size_t i = 0; i < msg.size(); i++) {
        if (msg[i] != reencoded[i]) {
            errors++;
        }
    }
    
    // If errors are within correctable range, accept the message
    if (errors <= nsym / 2) {
        return decoded;
    }
    
    // Too many errors - return empty vector
    return std::vector<uint8_t>();
}

std::vector<uint8_t> ErrorCorrection::encode(const std::vector<uint8_t>& data) {
    const int RS_NSYM = 32; // 32 parity bytes for error correction
    const int RS_BLOCK_SIZE = 223; // Maximum data per block
    
    std::vector<uint8_t> encoded;
    
    // Process data in blocks
    for (size_t i = 0; i < data.size(); i += RS_BLOCK_SIZE) {
        size_t blockSize = std::min(RS_BLOCK_SIZE, (int)(data.size() - i));
        std::vector<uint8_t> block(data.begin() + i, data.begin() + i + blockSize);
        
        // Pad block if necessary
        if (block.size() < RS_BLOCK_SIZE) {
            block.resize(RS_BLOCK_SIZE, 0);
        }
        
        // Encode block
        std::vector<uint8_t> encodedBlock = rsEncode(block, RS_NSYM);
        encoded.insert(encoded.end(), encodedBlock.begin(), encodedBlock.end());
    }
    
    return encoded;
}

std::vector<uint8_t> ErrorCorrection::decode(const std::vector<uint8_t>& data) {
    const int RS_NSYM = 32;
    const int RS_BLOCK_SIZE = 223;
    const int ENCODED_BLOCK_SIZE = RS_BLOCK_SIZE + RS_NSYM;
    
    std::vector<uint8_t> decoded;
    
    // Process data in blocks
    for (size_t i = 0; i < data.size(); i += ENCODED_BLOCK_SIZE) {
        if (i + ENCODED_BLOCK_SIZE > data.size()) {
            break; // Incomplete block
        }
        
        std::vector<uint8_t> block(data.begin() + i, data.begin() + i + ENCODED_BLOCK_SIZE);
        
        // Decode block
        std::vector<uint8_t> decodedBlock = rsDecode(block, RS_NSYM);
        
        if (decodedBlock.empty()) {
            // Failed to decode - too many errors
            continue;
        }
        
        decoded.insert(decoded.end(), decodedBlock.begin(), decodedBlock.end());
    }
    
    return decoded;
}

uint32_t ErrorCorrection::calculateCRC32(const std::vector<uint8_t>& data) {
    uint32_t crc = 0xFFFFFFFF;
    
    for (uint8_t byte : data) {
        crc ^= byte;
        for (int i = 0; i < 8; i++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return ~crc;
}
