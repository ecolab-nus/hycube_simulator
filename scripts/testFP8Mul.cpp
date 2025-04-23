#include <cstdint>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <vector>

float e4m3_to_float(uint8_t value) {
    bool sign = (value & 0x80) != 0;           // Bit 7
    uint8_t exp = (value & 0x78) >> 3;         // Bits 6-3
    uint8_t mantissa = value & 0x07;           // Bits 2-0

    if (exp == 0) {
        if (mantissa == 0) {
            return sign ? -0.0f : 0.0f;
        }
    }

    if (exp == 0x0F) {
        if (mantissa == 0) {
            return sign ? -INFINITY : INFINITY;
        }
        return NAN;
    }

    const int bias = 7;
    float result = powf(2.0f, static_cast<float>(exp) - bias);
    result *= (1.0f + static_cast<float>(mantissa) / 8.0f);
    return sign ? -result : result;
}

uint8_t float_to_e4m3(float value) {
    if (std::isnan(value)) return 0x7F;
    if (std::isinf(value)) return value > 0 ? 0x78 : 0xF8;
    if (value == 0.0f || std::fabs(value) < std::pow(2.0f, -6.0f)) {
        return std::signbit(value) ? 0x80 : 0x00;
    }

    uint8_t sign = std::signbit(value) ? 0x80 : 0x00;
    float abs_value = std::fabs(value);
    int exp = std::floor(std::log2(abs_value));
    
    if (exp > 7) return sign | 0x78;
    
    float normalized = abs_value / std::pow(2.0f, exp);
    float mantissa_value = (normalized - 1.0f) * 8.0f;
    uint8_t mantissa = static_cast<uint8_t>(std::round(mantissa_value));
    
    if (mantissa == 8) {
        mantissa = 0;
        exp++;
        if (exp > 7) return sign | 0x78;
    }
    
    uint8_t biased_exp = static_cast<uint8_t>(exp + 7);
    return sign | ((biased_exp & 0x0F) << 3) | (mantissa & 0x07);
}

uint8_t mul_e4m3(uint8_t a, uint8_t b) {
    float float_a = e4m3_to_float(a);
    float float_b = e4m3_to_float(b);
    float sum = float_a * float_b;
    return float_to_e4m3(sum);
}

int main() {
    std::vector<uint8_t> array1 = {
        5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35,
        37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63, 65, 0
    };
    std::vector<uint8_t> array2 = {
        0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45,
        48, 51, 54, 57, 60, 63, 66, 69, 72, 75, 78, 81, 84, 87, 90, 0
    };
    // Print header
    std::cout << std::setw(4) << "A1" << " | "
              << std::setw(4) << "A2" << " | "
              << std::setw(10) << "Bin1" << " | "
              << std::setw(10) << "Bin2" << " | "
              << std::setw(10) << "Float1" << " | "
              << std::setw(10) << "Float2" << " | "
              << std::setw(10) << "Sum" << " | "
              << std::setw(10) << "BinRes" << " | "
              << std::setw(4) << "Res" << "\n";

    std::cout << std::string(85, '-') << "\n";

    // Print data rows
    for (size_t i = 0; i < array1.size(); i++) {
        uint8_t num1 = array1[i];
        uint8_t num2 = array2[i];
        float float1 = e4m3_to_float(num1);
        float float2 = e4m3_to_float(num2);
        uint8_t result = mul_e4m3(num1, num2);
        float sum = e4m3_to_float(result);

        std::cout << std::dec << std::setw(4) << static_cast<int>(num1) << " | "
                  << std::dec << std::setw(4) << static_cast<int>(num2) << " | "
                  << std::bitset<8>(num1) << " | "
                  << std::bitset<8>(num2) << " | "
                  << std::fixed << std::setprecision(6)
                  << std::setw(10) << float1 << " | "
                  << std::setw(10) << float2 << " | "
                  << std::setw(10) << sum << " | "
                  << std::bitset<8>(result) << " | "
                  << std::dec << std::setw(4) << static_cast<int>(result) << "\n";
    }

    return 0;
}