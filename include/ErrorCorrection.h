#ifndef ERROR_CORRECTION_H
#define ERROR_CORRECTION_H

#include <vector>
#include <cstdint>

/**
 * @brief Reed-Solomon error correction implementation
 * 
 * Provides forward error correction for data transmission over noisy channels.
 * Uses Reed-Solomon (255, 223) coding scheme.
 */
class ErrorCorrection {
public:
    ErrorCorrection();
    ~ErrorCorrection();

    /**
     * @brief Encode data with Reed-Solomon error correction
     * @param data Input data to encode
     * @return Encoded data with parity bytes
     */
    std::vector<uint8_t> encode(const std::vector<uint8_t>& data);

    /**
     * @brief Decode Reed-Solomon encoded data
     * @param data Encoded data with possible errors
     * @return Decoded data with errors corrected
     */
    std::vector<uint8_t> decode(const std::vector<uint8_t>& data);

    /**
     * @brief Calculate CRC32 checksum
     * @param data Input data
     * @return CRC32 checksum value
     */
    static uint32_t calculateCRC32(const std::vector<uint8_t>& data);

private:
    // Galois Field tables for Reed-Solomon
    std::vector<uint8_t> gf_exp;
    std::vector<uint8_t> gf_log;
    
    void initGaloisField();
    uint8_t gfMul(uint8_t a, uint8_t b);
    uint8_t gfDiv(uint8_t a, uint8_t b);
    std::vector<uint8_t> gfPolyMul(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b);
    std::vector<uint8_t> gfPolyDiv(const std::vector<uint8_t>& dividend, const std::vector<uint8_t>& divisor);
    std::vector<uint8_t> rsGeneratorPoly(int nsym);
    std::vector<uint8_t> rsEncode(const std::vector<uint8_t>& msg, int nsym);
    std::vector<uint8_t> rsDecode(const std::vector<uint8_t>& msg, int nsym);
};

#endif // ERROR_CORRECTION_H
