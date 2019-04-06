#include <iostream>
#include <algorithm>
#include <cassert>

#include "argparse.hpp"


int main(int argc, char** argv)
{
    Argparse argparse("UnitTesting Software @ 2019 Test Company");

    argparse.add_argument("--cook", "I cook", true);
    argparse.add_argument("-e", "--eat", "I eat", false);
    argparse.add_argument("-d", "--drink", "I drink", false);
    argparse.add_argument("-s", "--sleep", "I sleep", false);
    argparse.add_argument("-c", "--chew", "I chew", true, "e");
    argparse.add_argument("-r", "--rest", "I rest", false);

    argparse.parse(argc, argv);

    if (argc == 1)
    {
        argparse.help();
        return 0;
    }

    if (argparse.isHelp())
    {
        argparse.help();
        return 0;
    }

    auto q = argparse.getv<std::string>("e");

    std::cout << "I ate " << (q.empty() ? "Nothing!" : "");
    std::for_each(q.begin(), q.end(), [](const std::string& food) {std::cout << food << " ";});
    std::cout  <<(q.empty() ? "" : "and chewed " + argparse.get<std::string>("c")) + ".\n";

    bool r = argparse.get<bool>("r");
    std::cout <<(r ? "I have rested afterwards": "No time to rest. Time is money!") << '\n';

    auto liters = argparse.getv<int>("d");
    if (not liters.empty())
    {
        std::cout << "I drunk: ";
        for (const auto& l: liters)
        {
            std::cout << l << " ";
        }
        std::cout << "liters of wine\n";
    }

    return 0;
}
