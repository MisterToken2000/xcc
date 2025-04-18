#ifndef X86_64_DARWIN
#define X86_64_DARWIN

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
#include "x.h"

string x86_64_darwin_stdio = "\n.globl printf\n";

string x86_64_darwin_include(string& code) {
    // Разбиваем код на строки (если нужно)
    vector<string> lines = splitToLines(code);
    
    for (string& line : lines) {
        line = deleteSpacesL(line);
        
        // Пропускаем строки без #include
        if (!startswith(line, "#include")) {
            continue;
        }

        // Извлекаем имя библиотеки (формат: #include <stdio.h>)
        line = line.substr(8); // Удаляем "#include"
        line = deleteSpacesL(line);
        
        // Проверяем формат: <...> или "..."
        if (line.empty() || (line[0] != '<' && line[0] != '"')) {
            cerr << "\x1B[31merror:\033[0m\tmalformed #include: " << line << "\n";
            return "";
        }

        char closing_char = (line[0] == '<') ? '>' : '"';
        size_t end_pos = line.find(closing_char, 1);
        
        if (end_pos == string::npos) {
            cerr << "\x1B[31merror:\033[0m\tunterminated #include: " << line << "\n";
            return "";
        }

        string lib = line.substr(1, end_pos - 1);
        string libPath = "xlibc/" + lib;

        // Открываем файл библиотеки
        ifstream libraryFile(libPath);
        if (!libraryFile.is_open()) {
            cerr << "\x1B[31merror:\033[0m\tcannot open library: " << libPath << "\n";
            return "";
        }


        // Читаем содержимое файла
        string libCode((istreambuf_iterator<char>(libraryFile)), 
                      istreambuf_iterator<char>());
        libraryFile.close();
        if (lib == "stdio.h")
        {
            vector<string> lines = splitToLines(libCode);
            for (string& line : lines)
            {
                if (startswith(line, "__global__ int printf(const char* format)"))
                {
                    libCode = "";
                }
            } 
        }
        cout << "\nIncluded: " << libPath;
        return "\n" + libCode + "\n";
    }

    // Если ни одного #include не найдено
    return "";
}

string x86_64_darwin_base(void)
{
    string as = R"(.section __TEXT, __text
.global _main)";
    return as;
}

string x86_64_darwin_writeText(const vector<string>& texts)
{
    stringstream as;
    as << "\n\tmov $0x2000004, %rax\n"
       << "\n\tmov $1, %rdi\n";
    for (size_t i = 0; i < texts.size(); ++i)
    {
        as << "\n\tlea msg" << i << "(%rip), %rsi\n"
           << "\n\tmov $len" << i << ", %rdx\n"
           << "\tsyscall\n";
    }
    as << "\n\tmov $0x000001, %rax\n"
       << "\txor %rdi, %rdi\n"
       << "\tsyscall\n";
    
    
    // Секция данных
    as << ".section __DATA,__data\n";
    
    // Определение строк
    for (size_t i = 0; i < texts.size(); ++i)
    {
        as << "msg" << i << ": .asciz \"" << texts[i] << "\"\n";
        as << "len" << i << " = . - msg" << i;
    }
    
    return as.str();
}

#endif