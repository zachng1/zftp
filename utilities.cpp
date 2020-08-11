#include "utilities.hpp"

namespace ZUtil{
std::vector<std::string> stringSplit(std::string string, std::string delim) {
    std::vector<std::string> result;
    std::string token;
    size_t pos;
    while (!string.empty()) {
        pos = string.find(delim);
        if (pos == string.npos) {
            result.push_back(string);
            break;
        }
        result.push_back(string.substr(0, pos));
        string = string.substr(pos+1);
    }
    return result;
}

bool endsWith(std::string string, std::string token) {
    if (string.substr(string.length() - token.length()) != token) return false;
    else return true;
}
}