#ifndef ARM64_LINUX_H
#define ARM64_LINUX_H

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
#include "x86_64-apple-darwin.h"

using namespace std;

string arm64_linux_stdio = "\n.global printf\nprintf:\n\tstp x29, x30, [sp, -16]!\n\tmov x29, sp\n\tmov x1, x0\n\tmov x2, 0\n";

string arm64_linux_include(string& code) {
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
                    libCode = arm64_linux_stdio;
                }
            } 
        }
        cout << "\nIncluded: " << libPath;
        return "\n" + libCode + "\n";
    }

    // Если ни одного #include не найдено
    return "";
}

string arm64_linux_base(void)
{
    string as = R"(count_loop:
        ldrb w3, [x1, x2]
        cbz w3, do_write            
        add x2, x2, 1               
        b count_loop                

do_write:
        mov x0, 1                   
        mov x8, 64                 
        svc 0                       

        ldp x29, x30, [sp], 16     
        ret                         

.global _exit
_exit:
        mov x8, 93                  
        mov x0, 0                 
        svc 0)";

    return as;
}

string arm64_linux_writeText(const vector<string>& texts)
{
    stringstream as;
    
    for (size_t i = 0; i < texts.size(); ++i)
    {
        as << "\n\tadrp x0, msg" << i
           << "\n\tadd x0, x0, :lo12:msg" << i
           << "\n\tbl printf\n";
    }
    as << "\n\tbl _exit\n";
    
    
    // Секция данных
    as << ".section .rodata\n";
    
    // Определение строк
    for (size_t i = 0; i < texts.size(); ++i)
    {
        as << "msg" << i << ": .asciz \"" << texts[i] << "\"\n";
    }
    
    return as.str();
}


#endif