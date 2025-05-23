#include "compadre.hpp"
#include <cstdint>
#include <expected>
#include <optional>
#include <ranges>
#include <fstream>
#include <streambuf>
#include <print>

auto collect_args(int argc, const char * argv[]) -> std::vector<std::string_view> { // NOLINT(modernize-avoid-c-arrays)
    auto args = std::vector<std::string_view>();
    for (int i = 0; i < argc; i++) {
        args.emplace_back(argv[i]);
    }

    return args;
}

enum class UserOption: uint8_t {
    InputFile,
    OutputFile,
    Compression,
    Decompression,
};

auto match_option(std::string_view user_input) -> std::optional<UserOption> {
    if (user_input == "-i") {
        return UserOption::InputFile;
    } else if (user_input == "-o") {
        return UserOption::OutputFile;
    } else if (user_input == "-c") {
        return UserOption::Compression;
    } else if (user_input == "-d") {
        return UserOption::Decompression;
    }

    return std::nullopt;
}

void print_usage() {
    std::println("Usage: compadre [options]\n"
                 "Available options:\n"
                 "  -i <file-name>    Specify the input file\n"
                 "  -o <file-name>    Specify the output file\n"
                 "  -c                Enable file compression\n"
                 "  -d                Enable file decompression");
}

void invalid_options_usage() {
    std::println("Invalid usage!");
    print_usage();
    std::exit(0);
}

struct UserInput {
    std::optional<std::string> input_filename;
    std::string output_filename = "out.comp";
    bool compression_mode;
    bool decompression_mode;

    UserInput() = default;
};

// TODO: return struct with the options selected by the user
auto treat_args(const std::vector<std::string_view>& args) -> UserInput {
    auto user_input = UserInput();

    for (auto [arg_index, arg]: std::views::enumerate(args)) {
        auto user_option = match_option(arg);
        if (user_option.has_value()) {
            switch (user_option.value()) {
                case UserOption::InputFile: 
                    {
                        // Check if the next index is valid
                        if (std::size_t(arg_index+1) < args.size()) {
                            auto input_filename = args.at(arg_index+1);
                            // TODO: check if it is a valid file
                            user_input.input_filename = input_filename;
                        } else {
                            invalid_options_usage();
                        }
                    }
                    break;
                case UserOption::OutputFile:
                    {
                        // Check if the next index is valid
                        if (std::size_t(arg_index+1) < args.size()) {
                            auto output_filename = args.at(arg_index+1);
                            // TODO: check if it is a valid file
                            user_input.output_filename = output_filename;
                        } else {
                            invalid_options_usage();
                        }
                    }
                    break;
                case UserOption::Compression:
                    {
                        user_input.compression_mode = true;
                    }
                    break;
                case UserOption::Decompression:
                    {
                        user_input.decompression_mode = true;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    if (user_input.compression_mode == user_input.decompression_mode) {
        std::println("Select one mode: compression (-c) or decompression (-d)!");
        invalid_options_usage();
    }

    if (not user_input.input_filename.has_value()) {
        std::println("Missing input file!");
        invalid_options_usage();
    }

    return user_input;
}

int main(int argc, const char * argv[]) {
    auto args = collect_args(argc, argv);
    auto user_input = treat_args(args);

    using ProbabilityModel = compadre::PPM<compadre::HuffmanSymbol, 2>;
    using CodingAlgorithm = compadre::Huffman;


    if (user_input.compression_mode) {
        auto t = std::ifstream(user_input.input_filename.value());
        auto input_text = std::string(
                std::istreambuf_iterator<char>(t),
                std::istreambuf_iterator<char>()
                );
        auto preproc = compadre::PreprocessedPortugueseText(input_text);
        auto compressor =
            compadre::Compressor<ProbabilityModel, CodingAlgorithm>();
        auto compressed_data = compressor.compress_preprocessed_portuguese_text(preproc);

        // Write output
        auto outbuff = outbit::BitBuffer();
        outbuff.read_from_vector(compressed_data);
        outbuff.write_as_file(user_input.output_filename);
    } else if (user_input.decompression_mode) {
        auto inbuff = outbit::BitBuffer();
        inbuff.read_from_file(user_input.input_filename.value());
        auto data = inbuff.buffer();

        auto compressor =
            compadre::Compressor<ProbabilityModel, CodingAlgorithm>();
        auto decompressed_text = compressor.decompress_preprocessed_portuguese_text(data);

        auto decompressed_data = std::vector<outbit::u8>();
        std::copy(decompressed_text.as_string().begin(),
                decompressed_text.as_string().end(), std::back_inserter(decompressed_data));

        // Write output
        auto outbuff = outbit::BitBuffer();
        outbuff.read_from_vector(decompressed_data);
        outbuff.write_as_file(user_input.output_filename);
    }

    return 0;
}
