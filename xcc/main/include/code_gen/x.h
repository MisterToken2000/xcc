#ifndef X_H
#define X_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <regex>
#include <vector>
#include <cctype>
#include <string>
#include <vector>
#include <sstream>
#include <regex>

using namespace std;

bool startswith(const string& str, const string& prefix)
{
    return str.rfind(prefix, 0) == 0;
}

string removeLastChars(string str, size_t count)
{
    if (count > str.size()) count = str.size();
    str.erase(str.end() - count, str.end());
    return str;
}

string deleteSpacesL(const string& source)
{
    size_t start = 0;
    while (start < source.size() && isspace(source[start]))
    {
        start++;
    }
    return source.substr(start);
}

vector<string> splitToLines(const string& source)
{
    vector<string> lines;
    size_t start = 0;
    size_t end = source.find('\n');

    while (end != string::npos)
    {
        lines.push_back(source.substr(start, end - start));
        start = end + 1;
        end = source.find('\n', start);
    }

    // Добавляем последнюю строку
    lines.push_back(source.substr(start));
    return lines;
}

struct CFunction {
    string globl;
    string name;
};

vector<CFunction> extractFunctions(const string& source) {
    vector<CFunction> functions;
    regex funcRegex(R"((int|void|char|float|double|long|short|unsigned|signed)\s+(\w+)\s*\([^)]*\)\s*\{)");
    smatch match;
    string temp = source;

    while (regex_search(temp, match, funcRegex)) {
        if (match.size() >= 3) {
            CFunction func;
            func.globl = ".global " + match[2].str() + "\n";
            func.name = match[2].str();
            functions.push_back(func);
        }
        temp = match.suffix();
    }

    return functions;
}

string getHost(void)
{
    system("uname -s -r -o -m > host");
    ifstream host_info("host");
    if (!host_info.is_open())
    {
        return "\x1B[31merror:\033[0m\tget host";
    }
    string host((istreambuf_iterator<char>(host_info)), istreambuf_iterator<char>());
    host_info.close();
    system("rm -rf host");
    return host;
}

#endif