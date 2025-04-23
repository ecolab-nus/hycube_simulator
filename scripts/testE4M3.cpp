#include <cstdint>
#include <cmath>
#include <limits>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cstring>  // Added for std::memcpy



float e4m3_to_float(uint8_t value) {
    // Extract components
    bool sign = (value & 0x80) != 0;           // Bit 7
    uint8_t exp = (value & 0x78) >> 3;         // Bits 6-3
    uint8_t mantissa = value & 0x07;           // Bits 2-0

    // Handle special cases
    if (exp == 0) {
        if (mantissa == 0) {
            return sign ? -0.0f : 0.0f;        // +/- zero
        }
        // Denormals not supported in E4M3
    }

    if (exp == 0x0F) {  // All exponent bits set
        if (mantissa == 0) {
            return sign ? -INFINITY : INFINITY; // +/- infinity
        }
        return NAN;                            // NaN
    }

    // Calculate normal number
    const int bias = 7;
    float result = powf(2.0f, static_cast<float>(exp) - bias);  // 2^(e-bias)
    result *= (1.0f + static_cast<float>(mantissa) / 8.0f);     // 1 + m/8
    
    return sign ? -result : result;
}

uint8_t float_to_e4m3(float value) {
    // Handle special cases
    if (std::isnan(value)) {
        return 0x7F;  // NaN representation in E4M3
    }
    
    if (std::isinf(value)) {
        return value > 0 ? 0x78 : 0xF8;  // +/- infinity
    }
    
    if (value == 0.0f || std::fabs(value) < std::pow(2.0f, -6.0f)) {
        return std::signbit(value) ? 0x80 : 0x00;  // +/- zero
    }

    // Extract sign
    uint8_t sign = std::signbit(value) ? 0x80 : 0x00;
    float abs_value = std::fabs(value);

    // Calculate exponent
    int exp = std::floor(std::log2(abs_value));
    
    // Check for overflow
    if (exp > 7) {
        return sign | 0x78;  // Return +/- infinity
    }
    
    // Normalize to 1.xxx format
    float normalized = abs_value / std::pow(2.0f, exp);
    
    // Calculate mantissa (3 bits)
    float mantissa_value = (normalized - 1.0f) * 8.0f;
    uint8_t mantissa = static_cast<uint8_t>(std::round(mantissa_value));
    
    // Handle rounding overflow
    if (mantissa == 8) {
        mantissa = 0;
        exp++;
        if (exp > 7) {
            return sign | 0x78;  // Overflow to infinity
        }
    }
    
    // Combine components
    uint8_t biased_exp = static_cast<uint8_t>(exp + 7);  // Add bias of 7
    return sign | ((biased_exp & 0x0F) << 3) | (mantissa & 0x07);
}
void print_e4m3(uint8_t value) {
    std::cout << "E4M3 Binary: " << std::bitset<8>(value) << "\n";
    std::cout << "Sign:     " << ((value & 0x80) ? "1 (negative)" : "0 (positive)") << "\n";
    std::cout << "Exponent: " << std::bitset<4>((value >> 3) & 0x0F) << " (bias 7)\n";
    std::cout << "Mantissa: " << std::bitset<3>(value & 0x07) << "\n";
    std::cout << "Float value: " << e4m3_to_float(value) << "\n\n";
}

void print_float_bits(float value) {
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(float));
    std::cout << "IEEE-754 Binary: " << std::bitset<32>(bits) << "\n";
    std::cout << "Sign:     " << ((bits >> 31) & 1) << "\n";
    std::cout << "Exponent: " << std::bitset<8>((bits >> 23) & 0xFF) << " (bias 127)\n";
    std::cout << "Mantissa: " << std::bitset<23>(bits & 0x7FFFFF) << "\n";
    std::cout << "Value: " << value << "\n\n";
}
void test_conversions() {
    std::cout << "Testing E4M3 <-> Float conversions\n";
    std::cout << "================================\n\n";

    // Test special values
    float test_values[] = {
        0.0f, -0.0f,                           // Zero
        INFINITY, -INFINITY,                   // Infinity
        NAN,                                   // NaN
        1.0f, -1.0f,                          // One
        1.75f, -1.75f,                        // Common test value
        0.5f, -0.5f,                          // Powers of 2
        2.0f, -2.0f,
        4.0f, -4.0f,
        std::pow(2.0f, -6.0f),                // Smallest normal
        (2.0f - std::pow(2.0f, -3.0f)) * std::pow(2.0f, 7.0f), // Largest normal
        std::pow(2.0f, -7.0f),                // Should round to zero
        std::pow(2.0f, 8.0f)                  // Should round to infinity
    };

    for (float value : test_values) {
        std::cout << "Testing value: " << value << "\n";
        std::cout << "Original float:\n";
        print_float_bits(value);
        
        uint8_t e4m3 = float_to_e4m3(value);
        std::cout << "Converted to E4M3:\n";
        print_e4m3(e4m3);
        
        float reconstructed = e4m3_to_float(e4m3);
        std::cout << "Reconstructed float:\n";
        print_float_bits(reconstructed);
        
        std::cout << "--------------------------------\n\n";
    }
}

int main() {
    test_conversions();
    return 0;
}