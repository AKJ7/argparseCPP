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



struct invalid_input : std::exception
{
private:
    std::string message;
public:
    explicit invalid_input(const std::string& msg) : message{msg + ". Type -h or --help for help"} {}
    const char* what() const noexcept override {return (message).c_str() ;}
};

class Argparse
{
private:
    std::unordered_map<std::string, std::tuple<std::string, bool, std::string>> argsMap;
    std::vector<std::pair<std::string, std::string>> argsRel;
    std::unordered_map<std::string, bool> settingsMap {{"colors", false}, {"help", true}, {"auto_help", false}, {"version", true}, {"error_msg", true}};
    std::unordered_map<std::string, std::string> configMap{{"description", ""}, {"path", ""}, {"version", "1.0.0"}};
    std::unordered_map<std::string, std::string> parsed;


protected:
    void error_message(const std::string& message)
    {
        if (settingsMap["error_msg"])
            throw invalid_input(message);
    }

    void arguments_printer(const std::vector<std::pair<std::string, std::string>>& container, int depth = 0)
    {
        std::vector<std::pair<std::string, std::string>> lCont;
        std::vector<std::pair<std::string, std::string>> rCont;
        std::partition_copy(container.begin(), container.end(), std::back_inserter(lCont), std::back_inserter(rCont), [this](const std::pair<std::string, std::string>& in){
            return std::get<1>(argsMap[in.second]) && std::get<2>(argsMap[in.second]).empty();
        });

        auto print = [this, &depth](const std::pair<std::string, std::string>& argPair){
            std::clog << std::setw(5 * depth)<< "" << std::setw(10) << argPair.first << std::setw(20) << argPair.second << std::setw(20) << std::get<0>(argsMap[argPair.second]) << '\n';
        };
        if (!lCont.empty())
        {
            std::clog << std::setw(5 * depth) << "" << "[Required]\n";
            for (const auto& v : lCont)
            {
                print(v);
                decltype(rCont.begin()) p;
                std::vector<std::pair<std::string, std::string>> dumpCont;
                if ((p = std::find_if(rCont.begin(), rCont.end(), [this, &v](const std::pair<std::string, std::string>& in){return !std::get<2>(argsMap[in.second]).empty() ? getRelMod(std::get<2>(argsMap[in.second])) == v.second : false;})) != rCont.end())
                {
                    dumpCont.emplace_back(*p);
                    rCont.erase(p);
                }
                arguments_printer(dumpCont, depth + 1);
            }
        }
        if (!rCont.empty())
        {
            std::clog << std::setw(5 * depth) << "" << "[Optional]\n";
            for (const auto& v : rCont)
            {
                print(v);
                decltype(lCont.begin()) p;
                std::vector<std::pair<std::string, std::string>> dumpCont;
                if ((p = std::find_if(lCont.begin(), lCont.end(), [this, &v](const std::pair<std::string, std::string>& in){ return !std::get<2>(argsMap[in.second]).empty() ? getRelMod(std::get<2>(argsMap[in.second])) == v.second : false;})) != lCont.end())
                {
                    dumpCont.emplace_back(*p);
                    lCont.erase(p);
                }
                arguments_printer(dumpCont, depth + 1);
            }
        }
    }

    std::string getRel(const std::string& value)
    {
        auto g = std::find_if(argsRel.begin(), argsRel.end(), [&value](std::pair<std::string, std::string>& elements){
            return (elements.first == value || elements.second == value);
        });
        return g == argsRel.end()? "" : g->second;
    }

    std::string getRelMod(const std::string& value)
    {
        auto g = std::find_if(argsRel.begin(), argsRel.end(), [&value](std::pair<std::string, std::string>& elements){
            return (elements.first == "-"+value || elements.second == "--"+value);
        });
        return g == argsRel.end() ? "" : g->second;
    }


public:
    explicit Argparse(const std::string& desc) { configMap["description"] = desc;}
    Argparse(int argc, char** argv) { parse(argc, argv); }
    Argparse(const std::string& desc, int argc, char** argv) : Argparse(argc, argv) { configMap["description"] = desc;};
    void settings(const std::vector<std::pair<std::string, bool>>& values)
    {
        for (const auto& value : values){
            if (settingsMap.find(value.first) != settingsMap.end()) {
                settingsMap[value.first] = value.second;
            } else {
                throw std::invalid_argument(value.first + " is not a valid setting");
            }
        }
    }
    void configuration(const std::vector<std::pair<std::string, std::string>>& values)
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

        auto cutArg = [](char* value) -> char* {
            while (*value)
            {
                if (*value == '='){
                    *value = 0;
                    return value + 1;
                }
                ++value;
            }
            return nullptr;
        };
        const char* value;

        while (*argv) {
            if (isFlag(*argv)){
                if (getRel(*argv).empty())
                {
                    error_message(*argv + std::string(" is an invalid input"));
                    return;
                }
                if ((value = cutArg(*argv))){
                 flag = *argv;
                 parsed[(getRel(flag))] += value;
                 parsed[(getRel(flag))] += " ";
                 ++argv;
                } else{
                    flag = *argv;
                    parsed[(getRel(flag))] = {};
                    ++argv;
                    while (*argv && !isFlag(*argv)) {
                        parsed[getRel(flag)] += *argv;
                        parsed[getRel(flag)] += " ";
                        ++argv;
                    }
                }
            } else {
                ++argv;
            }
        }
        if (!isHelp() && !isVersion())
        {
            for (auto& arg : argsMap)
            {
                if (!parsed.empty() && !std::get<2>(arg.second).empty() && parsed.find(arg.first) != parsed.end() && (getRelMod(std::get<2>(arg.second)).empty() ||parsed.find(getRelMod(std::get<2>(arg.second))) == parsed.end()))
                {
                    error_message(std::string(arg.first) + " requires: " + getRelMod(std::get<2>(arg.second)));
                }
                if (!parsed.empty() && std::get<1>(arg.second) && (getRelMod(std::get<2>(arg.second)).empty() || parsed.find(getRelMod(std::get<2>(arg.second))) != parsed.end()) && parsed.find(arg.first) == parsed.end())
                {
                    error_message("Missing required argument: " + std::string(arg.first));
                }
            }
        }

        if (argc == 1 && settingsMap["auto_help"]){
            help();
        }

    }

    void help()
    {
        if (!configMap["description"].empty())
            std::clog << configMap["description"] << '\n';
        std::clog << "Usage: " << configMap["path"] << " ";
        uint32_t someCounter{};

        for (const auto& arg : argsMap)
        {
            if (std::get<1>(arg.second)){
                if ( std::get<2>(arg.second).empty()){
                    auto q = std::find_if(argsRel.begin(), argsRel.end(), [&arg](const std::pair<std::string, std::string>& argsPair){
                        return arg.first == argsPair.first || arg.first == argsPair.second;
                    });
                    std::clog << "["<< (q->first.empty() ? q->second : q->first) << "] ";
                }
                ++someCounter;
            }
        }
        if (someCounter < argsMap.size())
        {
            std::clog << "[options] ";
        }
        std::clog << '\n';
        std::clog.setf(std::ios::left);
        arguments_printer(argsRel, 0);
        std::clog.unsetf(std::ios::left);
    }

    bool isHelp()
    {
        return parsed.size() == 1 && parsed.find("--help") != parsed.end();
    }

    bool isVersion()
    {
        return parsed.size() == 1 && parsed.find("--version") != parsed.end();
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
        argsMap[long_arg] = {desc, required, parent};
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
    std::vector<T> getv(const std::string& value)
    {
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
};

template <>
bool Argparse::get<bool>(const std::string& value){
    return parsed.find(getRelMod(value)) != parsed.end();
}


#endif //ARGPARSECPP_ARGPARSE_HPP