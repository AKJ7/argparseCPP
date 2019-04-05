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
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <sstream>

class Argparse
{
    // TODO: ADD SUPPORT FOR EQUAL
    // TODO: ADD BETTER ERROR HANDLING
    // TODO: ADD HELP
    // TODO: ADD HIERACHY
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
    std::vector<std::pair<std::string, std::string>> argsRel;
    std::unordered_map<std::string, bool> settingsMap {{"colors", false}, {"help", true}, {"version", true}};
    std::unordered_map<std::string, std::string> configMap{{"description", ""}, {"path", ""}, {"version", "1.0.0"}};
    std::unordered_map<std::string, std::string> parsed;

public:
    explicit Argparse(const std::string& desc) { configMap["description"] = desc;}
    Argparse(int argc, char** argv) { parse(argc, argv); }
    Argparse(const std::string& desc, int argc, char** argv) : Argparse(argc, argv) { configMap["description"] = desc;};
    void settings(std::vector<std::pair<std::string, bool>>& values)
    {
        for (const auto& value : values){
            if (settingsMap.find(value.first) != settingsMap.end()) {
                settingsMap[value.first] = value.second;
            } else {
                throw std::invalid_argument(value.first + " is not a valid setting");
            }
        }
    }
    void configuration(std::vector<std::pair<std::string, std::string>>& values)
    {
        for (const auto& value : values){
            if (configMap.find(value.first) != configMap.end()){
                configMap[value.first] = value.second;
            } else{
                throw std::invalid_argument(value.first + " is not a valid configuration");
            }
        }
    }
    void parse(int argc, char** argv)
    {
        if (configMap["path"].empty()){
            auto cleanPath = [](std::string& path) {
                path.erase(0, path.find_last_of('/') + 1);
            };
            configMap["path"] = argv[0];
            cleanPath(configMap["path"]);
        }
        if (settingsMap["version"])
            add_argument("-v", "--version", "Version", false);
        if (settingsMap["help"])
            add_argument("-h", "--help", "Help", false);

        std::string flag;
        auto isFlag = [](const char* value) {
            return value[0] == '-' || value[0] == (value[1] == '-');
        };
        std::stringstream ss;
        argv++;
        while (*argv) {
            if (isFlag(*argv)){
                flag = *argv;
                argv++;
                parsed[(getRel(flag))] = {};
                do {
                    if (!*argv){
                        break;
                    }
                    parsed[getRel(flag)] += *argv;
                    argv++;
                    parsed[getRel(flag)] += " ";
                } while (*argv && !isFlag(*argv));
            } else {
                ++argv;
            }
        }
    }
    void help()
    {
        if (!configMap["description"].empty())
            std::clog << configMap["description"] << '\n';
        std::clog << "Usage: " << configMap["path"] << '\n';
        std::clog.setf(std::ios::left);
        for (const auto& arg : argsRel){
            std::clog << std::setw(10) <<  arg.first << std::setw(20) << arg.second << std::setw(20) << argsMap[arg.second].first << std::setw(20) << (argsMap[arg.second].second? "Required" : "Optional") << '\n';
        }
    }
    void add_argument(const std::string& arg, const std::string& long_arg, const std::string& desc, bool required = false, const std::string& parent = "")
    {
        if (desc.empty())
            throw std::invalid_argument("Empty descriptions are not allowed");
        if (long_arg[0] != '-')
            throw std::invalid_argument(R"(Simple arguments start with '-'.)");
        if (long_arg[0] != '-' && long_arg[1] != '-')
            throw std::invalid_argument(R"(Long arguments start with '--')");
        argsRel.emplace_back(arg, long_arg);

        argsMap[long_arg] = {desc, required}; // TODO: Change to tuple and add parent
    }
    void add_argument(const std::string& long_arg, const std::string& desc, bool required = false, const std::string& parent = "")
    {
        add_argument("", long_arg, desc, required, parent);
    }

    template <typename T>
    T get(const std::string& value)
    {
        std::stringstream ss;
        ss << parsed[getRelMod(value)];
        T val;
        ss >> val;
        return val;
    }

    template <typename T>
    std::vector<T> getv(const std::string& value) {
        std::stringstream ss;
        ss << parsed[getRelMod(value)];
        std::vector<T> dump;
        T toResend;
        while (!ss.eof())
        {
            ss >> toResend;
            if (ss.peek() == std::char_traits<char>::eof())
                break;
            dump.emplace_back(toResend);
        }
        return dump;
    }

    std::string getRel(const std::string& value)
    {
        auto g = std::find_if(argsRel.begin(), argsRel.end(), [&value](std::pair<std::string, std::string>& elements){
            return (elements.first == value || elements.second == value);
        });
        return g->second;
    }

    std::string getRelMod(const std::string& value)
    {
        auto g = std::find_if(argsRel.begin(), argsRel.end(), [&value](std::pair<std::string, std::string>& elements){
            return (elements.first == "-"+value || elements.second == "--"+value);
        });
        return g->second;
    }
};

template <>
bool Argparse::get<bool>(const std::string& value){
    return parsed.find(getRelMod(value)) != parsed.end();
}


#endif //ARGPARSECPP_ARGPARSE_HPP
