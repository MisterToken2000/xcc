#ifndef I386_LINUX_H
#define I386_LINUX_H

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
#include "arm32-linux.h"

using namespace std;

string i386_linux_stdio = "\n.global printf\nprintf:\n\tpushl %ebp\n\tmovl %esp, %ebp\n\tmovl 8(%ebp), %esi\n\txorl %ecx, %ecx\n";

string i386_linux_include(string& code) {
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
                    libCode = i386_linux_stdio;
                }
            } 
        }
        cout << "\nIncluded: " << libPath;
        return "\n" + libCode + "\n";
    }

    // Если ни одного #include не найдено
    return "";
}

string i386_linux_base(void)
{
    string as = R"(count_loop:
        cmpb $0, (%esi, %ecx)
        je do_write
        incl %ecx
        jmp count_loop

do_write:
        movl $4, %eax         # sys_write (32-bit)
        movl $1, %ebx         # stdout
        movl %ecx, %edx       # length
        movl %esi, %ecx       # buffer
        int $0x80

        popl %ebp
        ret

.global return
return:
        movl $1, %eax         # sys_exit
        xorl %ebx, %ebx       # exit code 0
        int $0x80)";

    return as;
}

string i386_linux_writeText(const vector<string>& texts)
{
    stringstream as;
    
    for (size_t i = 0; i < texts.size(); ++i)
    {
        as << "\n\tpush $msg" << i
           << "\n\tcall printf\n";
    }
    as << "\n\tcall return\n";
    
    
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