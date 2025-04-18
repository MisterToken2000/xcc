#ifndef X86_64_LINUX_H
#define X86_64_LINUX_H

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
#include "i386-linux.h"

using namespace std;

string x86_64_linux_stdio = ".global printf\n\nprintf:\n\tpush %rbp\n\tmov %rsp, %rbp\n\tmov %rdi, %rsi\n\txor %rcx, %rcx\n";

string x86_64_linux_include(string& code) {
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
                    libCode = x86_64_linux_stdio;
                }
            } 
        }
        cout << "\nIncluded: " << libPath;
        return "\n" + libCode + "\n";
    }

    // Если ни одного #include не найдено
    return "";
}

string x86_64_linux_base(void)
{
    string as = R"(.section .data
stdout      = 1
sys_write   = 1
sys_exit    = 60

.section .text
    
count_loop:
    cmpb $0, (%rsi, %rcx)
    je do_write
    inc %rcx
    jmp count_loop
    
do_write:
    mov $sys_write, %rax
    mov $stdout, %rdi
    mov %rcx, %rdx
    syscall
    
    pop %rbp
    ret

.global return
return:
    mov $sys_exit, %rax
    xor %rdi, %rdi
    syscall)";

    return as;
}

string x86_64_linux_writeText(const vector<string>& texts)
{
    stringstream as;
    
    // Пролог
    /* as << "\n.global _start\n"
       << ".extern printf\n"
       << "_start:\n";
    */
    // Вызов printf для каждой строки
    for (size_t i = 0; i < texts.size(); ++i)
    {
        as << "\n\tmovq $msg" << i << ", %rdi\n"
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