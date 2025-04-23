#include <cstdint>
#include <iostream>
#include <iomanip>

uint64_t byteMultiply(uint64_t a, uint64_t b) {
    uint64_t result = 0;
    
    // Process each byte position independently
    for (int i = 0; i < 8; i++) {
        // Extract bytes from the same position in both numbers
        uint8_t byte_a = (a >> (i * 8)) & 0xFF;
        uint8_t byte_b = (b >> (i * 8)) & 0xFF;
        
        // Multiply the bytes
        uint8_t product = byte_a * byte_b;
        
        // Place the result in the corresponding position
        result |= (static_cast<uint64_t>(product) << (i * 8));
    }
    
    return result;
}

void printBytes(uint64_t num) {
    std::cout << "0x" << std::hex << std::setfill('0') << std::setw(16) << num << std::dec << "\n";
    std::cout << "Individual bytes: ";
    for (int i = 7; i >= 0; i--) {
        uint8_t byte = (num >> (i * 8)) & 0xFF;
        std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) 
                  << static_cast<int>(byte) << " ";
    }
    std::cout << std::dec << "\n";
}

void testByteMultiply() {
    // Test Case 1: Simple numbers
    uint64_t a1 = 0x0102030405060708;
    uint64_t b1 = 0x0807060504030201;
    
    std::cout << "Test Case 1:\n";
    std::cout << "Input a: ";
    printBytes(a1);
    std::cout << "Input b: ";
    printBytes(b1);
    std::cout << "Result: ";
    printBytes(byteMultiply(a1, b1));
    std::cout << "\n";
    
    // Test Case 2: All bytes same value
    uint64_t a2 = 0x0202020202020202;
    uint64_t b2 = 0x0303030303030303;
    
    std::cout << "Test Case 2:\n";
    std::cout << "Input a: ";
    printBytes(a2);
    std::cout << "Input b: ";
    printBytes(b2);
    std::cout << "Result: ";
    printBytes(byteMultiply(a2, b2));
    std::cout << "\n";
    
    // Test Case 3: Some zero bytes
    uint64_t a3 = 0x0100000000000001;
    uint64_t b3 = 0x0200000000000002;
    
    std::cout << "Test Case 3:\n";
    std::cout << "Input a: ";
    printBytes(a3);
    std::cout << "Input b: ";
    printBytes(b3);
    std::cout << "Result: ";
    printBytes(byteMultiply(a3, b3));
    std::cout << "\n";
}

int main() {
    testByteMultiply();
    return 0;
}