#include <iostream>
#include "include/argparse.hpp"

int main(int argc, char** argv) {

    Argparse argparse("Testing");

    argparse.add_argument("-c", "--compare", "To compare", false);
    argparse.add_argument("-d", "--decompress", "to Compress", true);
    argparse.add_argument("-o", "--output", "Output Stream", true, "d");
    argparse.add_argument("-t", "--tongue", "To tun", true);
    argparse.parse(argc, argv);
    if (argc == 1) {
        argparse.help();
        return 0;
    }

//    std::cout << argparse.get<int>("compare") << std::endl;
    for (auto& ele : argparse.getv<int>("compare"))
        std::cout << ele << std::endl;

    return 0;
}