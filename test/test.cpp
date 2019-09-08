#include <iostream>
#include "argparse.hpp"
#include <gtest/gtest.h>
#include <type_traits>
#include <vector>
#include <string>


TEST(Argparse, parses)
{
    char* argv[4] = {(char*)"filePath", (char*)"--check", (char*)"value"};
    int argc = 3;
    Argparse argparse("Description");
    argparse.add_argument("--test", "Testing", true);
    ASSERT_THROW(argparse.parse(argc, argv), invalid_input);
    ASSERT_THROW(Argparse argparse1(argc, argv), invalid_input);
    ASSERT_THROW(Argparse argparse1("Description", argc, argv), invalid_input);
}


TEST(Argparse, correct_commands1)
{
    char *argv[16] = {(char*)"filePath", (char *)"--check", (char *)"value", (char *)"Value2",
                    (char*)"--int_val", (char *)"2", (char *)"--double_val", (char*)"2.56", (char*)"4",
                    (char*)"--string_val", (char*)"i_am_a_string", (char*)"--vector_val", (char*)"1",
                    (char*)"2", (char*)"3"};
    int argc = sizeof(argv) / sizeof(char*);

    Argparse argparse("Description");
    argparse.add_argument("-c", "--check", "Description1", true);
    argparse.add_argument("-i", "--int_val", "Description2", true);
    argparse.add_argument("-d", "--double_val", "Description3", true);
    argparse.add_argument("-s", "--string_val", "Description4", true);
    argparse.add_argument("-v", "--vector_val", "Description5", true);
    argparse.parse(argc, argv);

    ASSERT_EQ(argparse.get<std::string>("c"), std::string("value"));
    ASSERT_EQ(argparse.get<std::string>("check"), std::string("value"));
    ASSERT_EQ(argparse.get<int>("i"), 2);
    ASSERT_EQ(argparse.get<int>("int_val"), 2);
    ASSERT_EQ(argparse.get<double>("d"), 2.56);
    ASSERT_EQ(argparse.get<double>("double_val"), 2.56);
    ASSERT_EQ(argparse.get<std::string>("s"), std::string("i_am_a_string"));
    ASSERT_EQ(argparse.get<std::string>("string_val"), std::string("i_am_a_string"));
    ASSERT_TRUE(argparse.get<bool>("int_val"));
    ASSERT_TRUE(!argparse.get<bool>("float_val"));
    auto v = argparse.getv<int>("v");
    std::vector<int> w = {1, 2, 3};
    for (int x : {0, 1, 2})
        ASSERT_EQ(v[x], w[x]);
}


TEST(Argparse, requirements)
{
    char* argv[6] = {(char*)"filepath", (char*)"-r", (char*)"required", (char*)"--not_required", (char*)"not importart"};
    int argc = 5;

    Argparse argparse("Description");
    argparse.add_argument("-r", "--required", "Description1", true);
    argparse.add_argument("-nr", "--not_required", "Description1", false);
    ASSERT_NO_THROW(argparse.parse(argc, argv));

    char* argv1[4] = {(char*)"filepath", (char*)"--not_required", (char*)"not importart"};
    int argc1 = 3;
    Argparse argparse1("Description");
    argparse1.add_argument("-r", "--required", "Description", true);
    argparse1.add_argument("-nr", "--not_required", "Description", false);
    ASSERT_THROW(argparse1.parse(argc1, argv1), invalid_input);
}


TEST(Argparse, parents)
{
    char* argv[5] = {(char*)"filepath", (char*)"-c", (char*)"-o", (char*)"filename"};
    int argc = 4;

    Argparse argparse("Description");
    argparse.add_argument("-c", "--compress", "Description", false);
    argparse.add_argument("-o", "--output", "Description", true, {"compress"});

    ASSERT_NO_THROW(argparse.parse(argc, argv));

    char* argv1[4] = {(char*)"filepath", (char*)"-o", (char*)"filename"};
    int argc1 = 3;

    Argparse argparse1("Description");
    argparse1.add_argument("-c", "--compress", "Description", false);
    argparse1.add_argument("-o", "--output", "Description", false, {"c"});

    ASSERT_THROW(argparse1.parse(argc1, argv1), invalid_input);

    char* argv2[4] = {(char*)"filepath", (char*)"-c", (char*)"filename"};
    int argc2 = 3;

    Argparse argparse2("Description");
    argparse2.add_argument("-c", "--compress", "Description", false);
    argparse2.add_argument("-o", "--output", "Description", false, {"c"});

    ASSERT_THROW(argparse1.parse(argc1, argv1), invalid_input);
}