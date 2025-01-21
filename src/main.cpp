#include <outbit/BitBuffer.hpp>
#include <cstdint>

int main() {
    // Create an empty buffer
    auto bitbuffer = outbit::BitBuffer();

    // Write integer to the buffer as raw bytes
    int32_t var = -42;
    bitbuffer.write(var);

    uint32_t first_value = 1324;
    char second_value = '@';

    // Write the first 11 bits of 'first_value' in the buffer
    bitbuffer.write_bits(first_value, 11);
    // Write the first 5 bits of 'second_value' in the buffer
    bitbuffer.write_bits(second_value, 5);

    // Save the buffer to a file
    bitbuffer.write_as_file("custom.file");
}
