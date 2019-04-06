#include <iostream>
#include "include/argparse.hpp"

int main(int argc, char** argv) {

    // Run with: ./argparseCPP --cook rice -e pie fish rice -c 10 -d water

    Argparse argparse("TestProgram v1.0.0");

    argparse.add_argument("--cook", "I cook", true);                // Requires --cook argument
    argparse.add_argument("-e", "--eat", "I eat", false);           // -e or --eat is optional
    argparse.add_argument("-d", "--drink", "I drink", false);       // -d or --drink is optional
    argparse.add_argument("-c", "--chew", "I chew", true, "e");     // -c or --chew is required, if -e or --eat is given


    argparse.parse(argc, argv);

    // Prints help if no parameter was given or if -h or --help is received as argument
    if (argc == 1 || argparse.isHelp())
    {
        argparse.help();
        return 0;
    }

    if (argparse.isVersion())
    {
        std::cout << "v1.0.0" << std::endl;
        return 0;
    }

    std::string dish = argparse.get<std::string>("cook");
    std::cout << dish << '\n';                             // outputs: rice

    auto foods = argparse.getv<std::string>("e");           // required -c (or --chew) arguments
    for (const auto& food : foods)
        std::cout << food << ' ';                          // outputs: pie, fish, rice
    std::cout << '\n';
    auto numberofChews = argparse.get<int>("c");
    std::cout << numberofChews << '\n';                // outputs: 10

    bool drank = argparse.get<bool>("d");
    std::cout << (drank ? "I drank" : "I didn't drink") << std::endl;   // outputs: I drank

    return EXIT_SUCCESS;
}