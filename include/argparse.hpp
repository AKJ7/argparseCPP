//
// Created by HP Pavilion on 04.04.2019.
//

#ifndef ARGPARSECPP_ARGPARSE_HPP
#define ARGPARSECPP_ARGPARSE_HPP

#include <iostream>
#include <stdexcept>
#include <string>
#include <cstring>
#include <unordered_map>
#include <cassert>
#include <vector>


class Argparse
{
private:
    class invalid_input : std::exception
    {
    private:
        std::string message;
    public:
        explicit invalid_input(std::string& msg) : message{msg} {}
        const char* what() const noexcept override {return (message + " is invalid! Type -h or --help for help.").c_str();}
    };

    std::unordered_map<std::string, std::pair<std::string, bool>> argsMap;
    std::unordered_map<std::string, std::string> argsRel;
    std::unordered_map<std::string, bool> gSettingsMap {{"colors", false}, {"help", true}};
    std::unordered_map<std::string, std::string> sSettingsMap{{"description", ""}, {"path", ""}};

public:
    explicit Argparse(std::string& desc) { sSettingsMap["description"] = desc;}
    Argparse(int argc, char** argv) { parse(argc, argv); }
    Argparse(std::string& desc, int argc, char** argv) : Argparse(argc, argv) { sSettingsMap["description"] = desc;};
    void general_settings(std::vector<std::pair<std::string, bool>>& values)
    {
        for (const auto& value : values){
            if (gSettingsMap.find(value.first) != gSettingsMap.end()) {
                gSettingsMap[value.first] = value.second;
            } else {
                throw std::invalid_argument(value.first + " is not a valid general setting");
            }
        }
    }
    void system_settings(std::vector<std::pair<std::string, std::string>>& values)
    {
        for (const auto& value : values){
            if (sSettingsMap.find(value.first) != sSettingsMap.end()){
                sSettingsMap[value.first] = value.second;
            } else{
                throw std::invalid_argument(value.first + " is not a valid system setting");
            }
        }
    }

    void parse(int argc, char** argv) {}
    void help()
    {
        std::clog << "Usage: " << sSettingsMap["path"];
    }
    void add_argument(std::string& arg, std::string& long_arg, std::string& desc, bool required = false, std::string parent = "")
    {
        argsRel[arg] = long_arg;
        if (arg[0] != '-')
            throw std::invalid_argument(R"(Simple arguments start with '-'.)");
        add_argument(long_arg, desc, required, parent);
    }
    void add_argument(std::string& long_arg, std::string& desc, bool required = false, std::string parent = "")
    {
        if (desc.empty())
            throw std::invalid_argument("Empty descriptions are not allowed");
        if (long_arg[0] != '-' && long_arg[1] != '-')
            throw std::invalid_argument(R"(Long arguments start with '--')");
        argsMap[long_arg] = {desc, required}; // TODO: Change to tuple and add parent
    }
};


#endif //ARGPARSECPP_ARGPARSE_HPP
