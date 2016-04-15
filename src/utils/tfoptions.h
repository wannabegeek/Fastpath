/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef options_h
#define options_h

#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <sstream>

namespace tf {
    class option_exception : public std::invalid_argument {
    public:
        explicit option_exception(const std::string &str) : std::invalid_argument(str) {}
    };

    class option {
    private:
        const std::string m_key;
        const std::string m_description;
        const bool m_required;
        const bool m_requiresArg;
        const std::string m_longName;
        const char m_shortName;
    public:
        explicit option(std::string key, std::string description, bool required, bool requiresArg, std::string longName, char shortName) : m_key(key), m_description(description), m_required(required), m_requiresArg(requiresArg), m_longName(longName), m_shortName(shortName){}

        const std::string &key() const {
            return m_key;
        }

        const std::string &description() const {
            return m_description;
        }

        const char shortName() const {
            return m_shortName;
        }

        const std::string &longName() const {
            return m_longName;
        }

        const bool requiresArg() const {
            return m_requiresArg;
        }

        const bool required() const {
            return m_required;
        }
    };

    class option_result {
    public:
        using args_type = std::vector<std::string>;
    private:
        const tf::option *m_option;
        args_type m_args;
    public:
        explicit option_result(const tf::option *option) : m_option(option) {}

        const tf::option *option() const {
            return m_option;
        }

        const args_type &args() const {
            return m_args;
        }

        void add_arg(const std::string &arg) {
            m_args.push_back(arg);
        }
    };

    class options {
    private:
        using OptionsType = std::vector<option>;
        OptionsType m_options;
        using MappedType = std::map<const std::string, option_result>;
        MappedType m_args;

    public:
        options() {}

        void register_option(const option &option) {
            m_options.push_back(option);
        }

        void printUsage() const {
            std::for_each(m_options.begin(), m_options.end(), [&](const option &option) {
                std::cout << "  -" << option.shortName() << ", --" << std::left << std::setw(25) << option.longName() << option.description() << std::endl;
            });
        }

        void parse(int argc, char *argv[]) throw (option_exception) {
            option_result *current = nullptr;
            for (int i = 1; i < argc; i++) {
                const std::string current_arg = std::string(argv[i]);
                if (current_arg[0] == '-') {
                    if (current_arg.size() <= 1) {
                        throw option_exception("'-' on it's own doesn't make sense");
                    }
                    if (current_arg == "--") {
                        // i.e. we have received '--' to terminate all subsequent args
                        break;
                    }

                    // we must be starting a tag
                    if (current_arg.compare(0, 2, "--") == 0) {
                        // we have along name
                        auto it = std::find_if(m_options.begin(), m_options.end(), [&](const option &option) {
                            return current_arg.substr(2) == option.longName();
                        });
                        if (it == m_options.end()) {
                            throw option_exception(std::string("Invalid option '") + current_arg + "'");
                        } else {
                            option_result result(&(*it));
                            auto item = m_args.emplace(it->key(), std::move(result));
                            current = &item.first->second;
                        }
                    } else {
                        // we have a short name
                        auto it = std::find_if(m_options.begin(), m_options.end(), [&](const option &option) {
                            return current_arg[1] == option.shortName();
                        });
                        if (it == m_options.end()) {
                            throw option_exception(std::string("Invalid option '") + current_arg + "'");
                        } else {
                            option_result result(&(*it));
                            auto item = m_args.emplace(it->key(), std::move(result));
                            current = &item.first->second;
                        }
                    }
                } else if (current != nullptr) {
                    current->add_arg(current_arg);
                } else {
                    throw option_exception("First argument must start with '-'");
                }
            }

            // validate that we have all required arguments
            std::for_each(m_options.begin(), m_options.end(), [&](const option &option) {
                if (option.required()) {
                    auto it = m_args.find(option.key());
                    if (it == m_args.end()) {
                        throw option_exception(std::string("-") + option.shortName() + ", --" + option.longName() + " is required");
                    }
                }

                if (option.requiresArg()) {
                    auto it = m_args.find(option.key());
                    if (it != m_args.end()) {
                        if (it->second.args().size() == 0) {
                            throw option_exception(std::string("-") + option.shortName() + ", --" + option.longName() + " requires an argument");
                        }
                    }
                }
            });
        }

        const void printContent() const {
            std::for_each(m_args.begin(), m_args.end(), [&](const MappedType::value_type &value) {
                std::cout << value.first << " ==> ";
                option_result::args_type args = value.second.args();
                std::for_each(args.begin(), args.end(), [&](const option_result::args_type::value_type &arg) {
                    std::cout << arg << " ";
                });
                std::cout << std::endl;
            });
        }

        const bool get(const std::string &key) const {
            auto it = m_args.find(key);
            return it != m_args.end();
        }

        const bool get(const std::string &key, option_result::args_type &result) const {
            auto it = m_args.find(key);
            if (it != m_args.end()) {
                result = it->second.args();
                return true;
            }

            return false;
        }

        const bool get(const std::string &key, std::string &result, const size_t index = 0) const {
            auto it = m_args.find(key);
            if (it != m_args.end()) {
                const option_result::args_type &v = it->second.args();
                if (index < v.size()) {
                    result = it->second.args().at(index);
                    return true;
                }
            }

            return false;
        }

        const std::string getWithDefault(const std::string &key, const std::string &defaultValue, const size_t index = 0) const {
            std::string result = defaultValue;
            if (this->get(key, result, index)) {
                return result;
            }
            return defaultValue;
        }

        template <typename T> const T get(const std::string &key, const T &defaultValue, const size_t index = 0) const {
            std::string result;
            if (this->get(key, result, index)) {
                T val;
                std::istringstream ss(result);
                ss >> val;
                return val;
            }
            return defaultValue;
        }
    };
}

#endif
