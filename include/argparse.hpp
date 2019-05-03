  /*
   * Copyright <2019> <Atchoglo, Komi Jules>

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
  following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
  disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
  disclaimer in the documentation and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
   */

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
    std::unordered_map<std::string, std::tuple<std::string, bool, std::vector<std::string>>> argsMap;
    std::vector<std::pair<std::string, std::string>> argsRel;
    std::unordered_map<std::string, bool> settingsMap{{"colors",    false},
                                                      {"help",      true},
                                                      {"auto_help", false},
                                                      {"version",   true},
                                                      {"error_msg", true}};
    std::unordered_map<std::string, std::string> configMap{{"description", ""}, {"path", ""}, {"version", "1.0.0"}};
    std::unordered_map<std::string, std::string> parsed;


protected:
    inline void error_message(const std::string& message)
    {
        if (settingsMap["error_msg"])
            throw invalid_input(message);
    }

    inline void arguments_printer(const std::vector<std::pair<std::string, std::string>>& container, int depth = 0)
    {
        std::vector<std::pair<std::string, std::string>> lCont;
        std::vector<std::pair<std::string, std::string>> rCont;
        std::partition_copy(container.begin(), container.end(), std::back_inserter(lCont), std::back_inserter(rCont),
                            [this](const std::pair<std::string, std::string> &in) {
                                return std::get<1>(argsMap[in.second]) && std::get<2>(argsMap[in.second]).empty();
                            });

        auto print = [this, &depth](const std::pair<std::string, std::string>& argPair){
            std::clog << std::setw(5 * depth) << "" << std::setw(10) << argPair.first << std::setw(20) << argPair.second
                      << std::setw(20) << std::get<0>(argsMap[argPair.second]) << '\n';
        };
        if (!lCont.empty())
        {
            std::clog << std::setw(5 * depth) << "" << "[Required]\n";
            for (const auto& v : lCont)
            {
                print(v);
                decltype(rCont.begin()) p;
                std::vector<std::pair<std::string, std::string>> dumpCont;
                if ((p = std::find_if(rCont.begin(), rCont.end(),
                                      [this, &v](const std::pair<std::string, std::string> &in) {
                                          if (std::get<2>(argsMap[in.second]).empty()){
                                              return false;
                                          } else{
                                              for (const auto& content : std::get<2>(argsMap[in.second]))
                                              {
                                                  if (content == v.second) return true;
                                              }
                                              return false;
                                          }
                                      })) != rCont.end()) {
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
                if ((p = std::find_if(lCont.begin(), lCont.end(),
                                      [this, &v](const std::pair<std::string, std::string> &in) {
                                          if (std::get<2>(argsMap[in.second]).empty()){
                                              return false;
                                          } else{
                                              for (const auto& content : std::get<2>(argsMap[in.second]))
                                              {
                                                  if (content == v.second) return true;
                                              }
                                              return false;
                                          }
                                      })) != lCont.end()) {

                    dumpCont.emplace_back(*p);
                    lCont.erase(p);
                }
                arguments_printer(dumpCont, depth + 1);
            }
        }
    }

    inline std::string getRel(const std::string& value)
    {
        auto g = std::find_if(argsRel.begin(), argsRel.end(), [&value](std::pair<std::string, std::string>& elements){
            return (elements.first == value || elements.second == value);
        });
        return g == argsRel.end()? "" : g->second;
    }

    inline std::string getRelMod(const std::string& value)
    {
        auto g = std::find_if(argsRel.begin(), argsRel.end(), [&value](std::pair<std::string, std::string>& elements){
            return (elements.first == "-"+value || elements.second == "--"+value);
        });
        return g == argsRel.end() ? "" : g->second;
    }


public:
    /// @brief Constructor knowing the description
    /// @class Argparse
    /// @param desc the description of the software. The description is added at the beginning of the help menu
    ///
    /// The purpose of this constructor is to be used to inialise the class, without parsing the arguments first.
    explicit Argparse(const std::string& desc) { configMap["description"] = desc;}
    /// @brief Constructor without description, for parsing the argument values
    /// @class Argparse
    /// @param argc the number of arguments received by the program. This value is usually gotten from the main() function
    /// @param argv the pointer of the pointers of the arguments given to the program, usally gotten from the main() function
    ///
    /// The purpose of this constructor is to allow to simply and directy parse the inputed values.
    Argparse(int argc, char** argv) { parse(argc, argv); }
    /// @brief Constructor knowing the description without any argument
    /// @class Argparse
    /// @param desc the description of the software. The description is added at the beginning of the help menu
    /// @param argc the number of arguments received by the program. This value is usually gotten from the main() function
    /// @param argv the pointer of the pointers of the arguments given to the program, usally gotten from the main() function
    ///
    /// The purpose of this constructor is to allow the initialization of the class, knowing the descriptionm, without any
    /// regards for the arguments.
    Argparse(const std::string& desc, int argc, char** argv) : Argparse(argc, argv) { configMap["description"] = desc;};
    /// @brief change the base settings of the parser
    /// @param values a list (here vector) of pairs, whereas the first denotes the setting to change and the second, the new setting
    /// @returns (void)
    inline void settings(const std::vector<std::pair<std::string, bool>>& values)
    {
        for (const auto& value : values){
            if (settingsMap.find(value.first) != settingsMap.end()) {
                settingsMap[value.first] = value.second;
            } else {
                throw std::invalid_argument(value.first + " is not a valid setting");
            }
        }
    }
    /// @brief Changes the configuration of the parser.
    /// @param values A string pair of std::string to configure. The first is the configuration to change and the second
    /// the configuration value.
    /// @returns (void)
    ///
    /// Currently, the following configurations can be changed: path, description and the version
    inline void configuration(const std::vector<std::pair<std::string, std::string>>& values)
    {
        for (const auto& value : values){
            if (configMap.find(value.first) != configMap.end()){
                configMap[value.first] = value.second;
            } else{
                throw std::invalid_argument(value.first + " is not a valid configuration");
            }
        }
    }
    /// @brief Parses the given arguments from add_arguments and the configurations parameters, by checking the validity
    /// of the inputed arguments and sets their values ready for the 'get' functions
    /// @param argc ArgumentsCount
    /// @param argv ArgumentsValues
    /// @returns (void)
    ///
    /// The arguments of this function are supposed to originate from the 'main' function of your project
    inline void parse(int argc, char** argv)
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
            auto pprintParents = [&](const std::pair<std::string, std::tuple<std::string, bool, std::vector<std::string>>>& arg) -> std::string{
                uint32_t myCounter = 0;
                std::stringstream sss;
                for (const auto& fl : std::get<2>(arg.second))
                {
                    myCounter++;
                    sss << getRelMod(fl);
                    if (myCounter == std::get<2>(arg.second).size() - 1 &&  std::get<2>(arg.second).size() != 1)
                        sss << " or ";
                    else if (myCounter == std::get<2>(arg.second).size())
                        break;
                    else
                        sss << ", ";
                }
                return sss.str();
            };
            std::string found{};
            for (auto& arg : argsMap)
            {
                if (!parsed.empty() &&
                    !std::get<2>(arg.second).empty() &&
                    parsed.find(arg.first) != parsed.end() &&
                    (std::all_of(std::get<2>(arg.second).begin(), std::get<2>(arg.second).end(), [&](const std::string& val){
                        if (getRelMod(val).empty() or parsed.find(getRelMod(val)) == parsed.end()) {return true;}
                        else{found = val; return false;
                        }
                    })))
                {
                    error_message(arg.first + " requires: " + pprintParents(arg));
                }

                if (!parsed.empty() && std::get<1>(arg.second) && std::get<2>(arg.second).empty() && parsed.find(arg.first) == parsed.end())
                {
                    error_message("Missing required argument: " + arg.first);
                }

                if (!parsed.empty() &&
                    std::get<1>(arg.second) &&
                    (!std::get<2>(arg.second).empty() &&
                    std::none_of(std::get<2>(arg.second).begin(), std::get<2>(arg.second).end(), [&](const std::string& val){return !std::get<1>(argsMap[getRelMod(val)]) or parsed.find(arg.first) != parsed.end();})))
                {
                    error_message("Missing required argument: " + arg.first + ", of parents: " + pprintParents(arg));
                }
            }
        }

        if (argc == 1 && settingsMap["auto_help"]){
            help();
        }

    }
    /// @brief Creates the help menu
    /// @returns void
    ///
    /// 'std::clog' is used to keep the output from interfering with the standard output stream
    inline void help()
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
    /// @brief Checks if the intent of the user is to get the help menu
    /// @returns True if the help menu is requested, else false
    ///
    /// This function checks if the user entered -h or --help and if only one argument was provided by the latter
    inline bool isHelp()
    {
        return parsed.size() == 1 && parsed.find("--help") != parsed.end();
    }
    /// @brief Checks if the intent of the user is to get the version
    /// @returns True if the version values are requested, else false
    ///
    /// This function checks if the user entered -v or --version and if only one argument was provided by the latter
    inline bool isVersion()
    {
        return parsed.size() == 1 && parsed.find("--version") != parsed.end();
    }
    /// @brief Used to add the arguments, or rather allowed flags for future parsing.
    /// @param arg the name of the short flag, i.e. -a
    /// @param long_arg the name of the long flag, i.e. --add
    /// @param desc description of the flag for the help menu
    /// @param required is the flag a must?
    /// @param parents a list (here: vector) of the parents of the current flag, i.e. "{"-r", "-p"} for parents "r" and "-p"
    /// @returns void
    inline void add_argument(const std::string& arg, const std::string& long_arg, const std::string& desc, bool required = false, const std::vector<std::string>& parents = {})
    {
        if (desc.empty())
            throw std::invalid_argument("Empty descriptions are not allowed");
        if (long_arg[0] != '-')
            throw std::invalid_argument(R"(Short flags start with '-'.)");
        if (long_arg[0] != '-' && long_arg[1] != '-')
            throw std::invalid_argument(R"(Long flags start with '--')");
        argsRel.emplace_back(arg, long_arg);
        argsMap[long_arg] = {desc, required, parents};
    }
    /// @brief Used to add the arguments, or rather allowed flags for future parsing.
    /// @param long_arg the name of the long flag, i.e. --add
    /// @param desc description of the flag for the help menu
    /// @param required is the flag a must?
    /// @param parents a list (here: vector) of the parents of the current flag, i.e. "{"-r", "-p"} for parents "r" and "-p"
    /// @returns void
    inline void add_argument(const std::string& long_arg, const std::string& desc, bool required = false, const std::vector<std::string>& parents = {})
    {
        add_argument("", long_arg, desc, required, parents);
    }
    /// @brief Fetch a flag's value
    /// @param value the flag name, from which the value should be fetched from
    /// @returns the fetched value
    ///
    /// if the >>> operator is not overloaded, only primitive types can be specified as template paramaters
    template <typename T>
    inline T get(const std::string& value)
    {
        std::stringstream ss;
        ss << parsed[getRelMod(value)];
        T val;
        ss >> val;
        return val;
    }
    /// @brief Fetch a vector of values from a flag
    /// @param value the flag name, from which the values should be fetched from
    /// @returns a vector of fetched values
    ///
    /// if the >>> operator is not overloaded, only primitive types can be specified as template paramaters
    template <typename T>
    inline std::vector<T> getv(const std::string& value)
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

/// @brief checks if a flag was inputed by the user
/// @param value the flag name to be checked for existance
/// @returns true if the flag was parsed, else false
template <>
inline bool Argparse::get<bool>(const std::string& value){
    return parsed.find(getRelMod(value)) != parsed.end();
}


#endif //ARGPARSECPP_ARGPARSE_HPP