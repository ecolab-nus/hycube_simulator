#include <cstdint>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <vector>

// This function more closely mimics hardware implementation of E4M3 to float conversion
float e4m3_to_float(uint8_t value) {
    bool sign = (value & 0x80) != 0;           // Bit 7
    uint8_t exp = (value & 0x78) >> 3;         // Bits 6-3
    uint8_t mantissa = value & 0x07;           // Bits 2-0

    if (exp == 0) {
        if (mantissa == 0) {
            return sign ? -0.0f : 0.0f;        // +/- zero
        }
    }

    if (exp == 0x0F) {  // All exponent bits set
        if (mantissa == 0) {
            return sign ? -INFINITY : INFINITY; // +/- infinity
        }
        return NAN;                            // NaN
    }

    const int bias = 7;
    float result = powf(2.0f, static_cast<float>(exp) - bias);
    result *= (1.0f + static_cast<float>(mantissa) / 8.0f);
    
    return sign ? -result : result;
}

// Hardware-like implementation of E4M3 addition with explicit rounding logic
uint8_t add_e4m3_hardware(uint8_t a, uint8_t b) {
    // Extract components
    bool sign_a = (a & 0x80) != 0;
    uint8_t exp_a = (a & 0x78) >> 3;
    uint8_t mant_a = a & 0x07;
    
    bool sign_b = (b & 0x80) != 0;
    uint8_t exp_b = (b & 0x78) >> 3;
    uint8_t mant_b = b & 0x07;
    
    // Check for special cases
    bool a_is_zero = (exp_a == 0 && mant_a == 0);
    bool b_is_zero = (exp_b == 0 && mant_b == 0);
    bool a_is_inf = (exp_a == 0x0F && mant_a == 0);
    bool b_is_inf = (exp_b == 0x0F && mant_b == 0);
    
    // Special case handling
    if (a_is_zero) return b;
    if (b_is_zero) return a;
    if (a_is_inf && b_is_inf) {
        if (sign_a == sign_b) return a; // Same sign infinities
        return 0x7F; // Different sign infinities -> NaN
    }
    if (a_is_inf) return a;
    if (b_is_inf) return b;
    
    // Convert to expanded representation for addition
    // Extended precision: sign, 4-bit exp, 6-bit mantissa (3 bits + 3 guard bits)
    int32_t exp_unbiased_a = exp_a - 7;
    int32_t exp_unbiased_b = exp_b - 7;
    
    // Create 9-bit mantissas (1 implicit bit + 3 explicit bits + 5 guard bits)
    uint32_t mant_extended_a = ((uint32_t)1 << 8) | ((uint32_t)mant_a << 5);  // 1.mant_a000
    uint32_t mant_extended_b = ((uint32_t)1 << 8) | ((uint32_t)mant_b << 5);  // 1.mant_b000
    
    // Align exponents
    int32_t exp_result = std::max(exp_unbiased_a, exp_unbiased_b);
    int32_t shift_a = exp_result - exp_unbiased_a;
    int32_t shift_b = exp_result - exp_unbiased_b;
    
    // Shift mantissas according to exponent difference
    if (shift_a > 0 && shift_a < 15) {
        mant_extended_a >>= shift_a;
    } else if (shift_a >= 15) {
        mant_extended_a = 0;  // Too small to contribute
    }
    
    if (shift_b > 0 && shift_b < 15) {
        mant_extended_b >>= shift_b;
    } else if (shift_b >= 15) {
        mant_extended_b = 0;  // Too small to contribute
    }
    
    // Handle signs and perform addition
    uint32_t mant_result;
    bool sign_result;
    
    if (sign_a == sign_b) {
        // Same signs, add mantissas
        mant_result = mant_extended_a + mant_extended_b;
        sign_result = sign_a;
    } else {
        // Different signs, subtract smaller from larger
        if (mant_extended_a >= mant_extended_b) {
            mant_result = mant_extended_a - mant_extended_b;
            sign_result = sign_a;
        } else {
            mant_result = mant_extended_b - mant_extended_a;
            sign_result = sign_b;
        }
    }
    
    // Check if result is zero
    if (mant_result == 0) {
        return 0;  // Return +0
    }
    
    // Normalize result
    int32_t leading_zeros = 0;
    for (int i = 14; i >= 0; i--) {
        if (mant_result & ((uint32_t)1 << i)) {
            leading_zeros = 9 - i;
            break;
        }
    }
    
    if (leading_zeros < 0) {
        // Normalization by right shift (overflow)
        int32_t shift_right = -leading_zeros;
        exp_result += shift_right;
        
        // Capture rounding bits before shift
        bool guard = (mant_result & ((uint32_t)1 << (shift_right - 1))) != 0;
        bool round = (mant_result & ((uint32_t)1 << (shift_right - 2))) != 0;
        bool sticky = false;
        for (int i = 0; i < shift_right - 2; i++) {
            if (mant_result & ((uint32_t)1 << i)) {
                sticky = true;
                break;
            }
        }
        
        mant_result >>= shift_right;
        
        // Round-to-nearest, ties to even rounding
        bool round_up = guard && (round || sticky || (mant_result & 1));
        if (round_up) {
            mant_result++;
            
            // Check if rounding causes overflow
            if (mant_result & ((uint32_t)1 << 9)) {
                mant_result >>= 1;
                exp_result++;
            }
        }
    } else if (leading_zeros > 0) {
        // Normalization by left shift (underflow)
        mant_result <<= leading_zeros;
        exp_result -= leading_zeros;
    }
    
    // Check for overflow or underflow in exponent
    if (exp_result > 7) {
        return sign_result ? 0xF8 : 0x78;  // +/- infinity
    }
    
    if (exp_result < -6) {
        return sign_result ? 0x80 : 0x00;  // +/- zero (flush to zero)
    }
    
    // Extract final 3-bit mantissa (positions 5-7 of mant_result)
    uint8_t final_mantissa = (mant_result >> 5) & 0x07;
    
    // Perform rounding to nearest, ties to even 
    bool guard_bit = (mant_result & (1 << 4)) != 0;  // Position 4
    bool round_bit = (mant_result & (1 << 3)) != 0;  // Position 3
    bool sticky_bit = false;
    for (int i = 0; i < 3; i++) {
        if (mant_result & (1 << i)) {
            sticky_bit = true;
            break;
        }
    }
    
    // Apply round-to-nearest, ties to even rule
    bool do_round_up = false;
    if (guard_bit) {
        if (round_bit || sticky_bit) {
            do_round_up = true;  // Round up if guard bit and any other bits
        } else {
            // Exactly halfway, round to even
            do_round_up = (final_mantissa & 1) != 0;  // Round up if mantissa is odd
        }
    }
    
    if (do_round_up) {
        final_mantissa++;
        // Check for overflow in mantissa
        if (final_mantissa == 8) {
            final_mantissa = 0;
            exp_result++;
            // Check for overflow in exponent
            if (exp_result > 7) {
                return sign_result ? 0xF8 : 0x78;  // +/- infinity
            }
        }
    }
    
    // Combine fields
    uint8_t biased_exp = exp_result + 7;
    uint8_t result = (sign_result ? 0x80 : 0x00) | ((biased_exp & 0x0F) << 3) | final_mantissa;
    
    return result;
}

// Function to unpack and display intermediate hardware calculation steps
std::string explain_hardware_addition(uint8_t a, uint8_t b) {
    std::stringstream explanation;
    
    // Extract components
    bool sign_a = (a & 0x80) != 0;
    uint8_t exp_a = (a & 0x78) >> 3;
    uint8_t mant_a = a & 0x07;
    
    bool sign_b = (b & 0x80) != 0;
    uint8_t exp_b = (b & 0x78) >> 3;
    uint8_t mant_b = b & 0x07;
    
    explanation << "Hardware Addition Steps:\n";
    explanation << "------------------------\n";
    explanation << "Input A: " << (sign_a ? "-" : "+") << " exp=" << static_cast<int>(exp_a) 
                << " mant=" << static_cast<int>(mant_a) << "\n";
    explanation << "Input B: " << (sign_b ? "-" : "+") << " exp=" << static_cast<int>(exp_b) 
                << " mant=" << static_cast<int>(mant_b) << "\n";
    
    // Check for special cases
    bool a_is_zero = (exp_a == 0 && mant_a == 0);
    bool b_is_zero = (exp_b == 0 && mant_b == 0);
    
    if (a_is_zero || b_is_zero) {
        explanation << "Special case: one input is zero\n";
        return explanation.str();
    }
    
    // Unbiased exponents and extended mantissas
    int32_t exp_unbiased_a = exp_a - 7;
    int32_t exp_unbiased_b = exp_b - 7;
    
    explanation << "Unbiased exponents: A=" << exp_unbiased_a << " B=" << exp_unbiased_b << "\n";
    
    // Create 9-bit mantissas (1 implicit bit + 3 explicit bits + 5 guard bits)
    uint32_t mant_extended_a = ((uint32_t)1 << 8) | ((uint32_t)mant_a << 5);
    uint32_t mant_extended_b = ((uint32_t)1 << 8) | ((uint32_t)mant_b << 5);
    
    explanation << "Extended mantissas:\n";
    explanation << "A: " << std::bitset<12>(mant_extended_a) << "\n";
    explanation << "B: " << std::bitset<12>(mant_extended_b) << "\n";
    
    // Align exponents
    int32_t exp_result = std::max(exp_unbiased_a, exp_unbiased_b);
    int32_t shift_a = exp_result - exp_unbiased_a;
    int32_t shift_b = exp_result - exp_unbiased_b;
    
    explanation << "Alignment: max_exp=" << exp_result << " shift_A=" << shift_a 
                << " shift_B=" << shift_b << "\n";
    
    // Shift mantissas according to exponent difference
    if (shift_a > 0) {
        mant_extended_a >>= shift_a;
        explanation << "Shifted A: " << std::bitset<12>(mant_extended_a) << "\n";
    }
    
    if (shift_b > 0) {
        mant_extended_b >>= shift_b;
        explanation << "Shifted B: " << std::bitset<12>(mant_extended_b) << "\n";
    }
    
    // Handle signs and perform addition
    uint32_t mant_result;
    bool sign_result;
    
    if (sign_a == sign_b) {
        mant_result = mant_extended_a + mant_extended_b;
        sign_result = sign_a;
        explanation << "Same signs, adding mantissas\n";
    } else {
        if (mant_extended_a >= mant_extended_b) {
            mant_result = mant_extended_a - mant_extended_b;
            sign_result = sign_a;
            explanation << "Different signs, A>=B, subtracting B from A\n";
        } else {
            mant_result = mant_extended_b - mant_extended_a;
            sign_result = sign_b;
            explanation << "Different signs, B>A, subtracting A from B\n";
        }
    }
    
    explanation << "Pre-normalized result: " << (sign_result ? "-" : "+") << " exp=" 
                << exp_result << " mant=" << std::bitset<12>(mant_result) << "\n";
    
    // Normalize result - find leading 1
    int leading_bit_pos = -1;
    for (int i = 11; i >= 0; i--) {
        if (mant_result & ((uint32_t)1 << i)) {
            leading_bit_pos = i;
            break;
        }
    }
    
    int32_t normalization_shift = 8 - leading_bit_pos;
    explanation << "Normalization shift: " << normalization_shift << "\n";
    
    uint32_t pre_round_mantissa = mant_result;
    
    if (normalization_shift < 0) {
        // Right shift (overflow)
        uint32_t rounding_bits = pre_round_mantissa & ((1 << (-normalization_shift)) - 1);
        pre_round_mantissa >>= (-normalization_shift);
        exp_result += (-normalization_shift);
        
        explanation << "Overflow, right shift by " << (-normalization_shift) << "\n";
        explanation << "Rounding bits: " << std::bitset<8>(rounding_bits) << "\n";
    } else if (normalization_shift > 0) {
        // Left shift (underflow)
        pre_round_mantissa <<= normalization_shift;
        exp_result -= normalization_shift;
        explanation << "Underflow, left shift by " << normalization_shift << "\n";
    }
    
    explanation << "Pre-round: " << (sign_result ? "-" : "+") << " exp=" 
                << exp_result << " mant=" << std::bitset<12>(pre_round_mantissa) << "\n";
    
    // Extract rounding bits
    bool guard_bit = (pre_round_mantissa & (1 << 4)) != 0;
    bool round_bit = (pre_round_mantissa & (1 << 3)) != 0;
    bool sticky_bit = false;
    for (int i = 0; i < 3; i++) {
        if (pre_round_mantissa & (1 << i)) {
            sticky_bit = true;
            break;
        }
    }
    
    uint8_t final_mantissa = (pre_round_mantissa >> 5) & 0x07;
    
    explanation << "Rounding bits: guard=" << guard_bit << " round=" << round_bit 
                << " sticky=" << sticky_bit << "\n";
    explanation << "Final mantissa before rounding: " << static_cast<int>(final_mantissa) << "\n";
    
    // Apply round-to-nearest, ties to even rule
    bool do_round_up = false;
    if (guard_bit) {
        if (round_bit || sticky_bit) {
            do_round_up = true;
            explanation << "Rounding up (guard=1 and other bits set)\n";
        } else {
            // Exactly halfway, round to even
            do_round_up = (final_mantissa & 1) != 0;
            explanation << "Exactly halfway, rounding " << (do_round_up ? "up to even" : "down to even") << "\n";
        }
    }
    
    if (do_round_up) {
        final_mantissa++;
        explanation << "Rounded mantissa: " << static_cast<int>(final_mantissa) << "\n";
        
        // Check for overflow in mantissa
        if (final_mantissa == 8) {
            final_mantissa = 0;
            exp_result++;
            explanation << "Rounding caused mantissa overflow, incrementing exponent to " << exp_result << "\n";
        }
    }
    
    // Final result check
    if (exp_result > 7) {
        explanation << "Exponent overflow, returning infinity\n";
    } else if (exp_result < -6) {
        explanation << "Exponent underflow, flushing to zero\n";
    } else {
        uint8_t biased_exp = exp_result + 7;
        explanation << "Final biased exponent: " << static_cast<int>(biased_exp) << "\n";
        explanation << "Final result: " << (sign_result ? "-" : "+") << " exp=" 
                    << static_cast<int>(biased_exp) << " mant=" << static_cast<int>(final_mantissa) << "\n";
    }
    
    return explanation.str();
}

int main() {
    std::vector<uint8_t> array1 = {
        5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35,
        37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63, 1
    };
    std::vector<uint8_t> array2 = {
        0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45,
        48, 51, 54, 57, 60, 63, 66, 69, 72, 75, 78, 81, 84, 87, 90
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
        
        // Use hardware-like implementation
        uint8_t result = add_e4m3_hardware(num1, num2);
        float float_result = e4m3_to_float(result);

        std::cout << std::dec << std::setw(4) << static_cast<int>(num1) << " | "
                  << std::dec << std::setw(4) << static_cast<int>(num2) << " | "
                  << std::bitset<8>(num1) << " | "
                  << std::bitset<8>(num2) << " | "
                  << std::fixed << std::setprecision(6)
                  << std::setw(10) << float1 << " | "
                  << std::setw(10) << float2 << " | "
                  << std::setw(10) << float_result << " | "
                  << std::bitset<8>(result) << " | "
                  << std::dec << std::setw(4) << static_cast<int>(result) << "\n";
        
        // Optionally, for detailed explanation of specific cases:
        // if (i == 0) {  // Change this to examine specific rows
        //     std::cout << "\n" << explain_hardware_addition(num1, num2) << "\n";
        // }
    }
    
    // Uncomment to see detailed explanation for a specific pair
    // std::cout << "\n" << explain_hardware_addition(13, 12) << "\n";
    
    return 0;
}