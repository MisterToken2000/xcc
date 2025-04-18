#ifndef ARM32_LINUX_H
#define ARM32_LINUX_H

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
#include "arm64-linux.h"

using namespace std;

string arm32_linux_stdio = "\n.global printf\nprintf:\n\tpush {r11, lr}\n\tmov r11, sp\n\tmov r1, r0\n\tmov r2, #0\n";

string arm32_linux_include(string& code) {
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
                    libCode = arm32_linux_stdio;
                }
            } 
        }
        cout << "\nIncluded: " << libPath;
        return "\n" + libCode + "\n";
    }

    // Если ни одного #include не найдено
    return "";
}

string arm32_linux_base(void)
{
    string as = R"(count_loop:
        ldrb r3, [r1, r2]    @ Загружаем байт из строки
        cmp r3, #0            @ Проверяем на нуль-терминатор
        beq do_write          @ Если конец строки — переходим к выводу
        add r2, r2, #1        @ Увеличиваем счетчик
        b count_loop          @ Повторяем цикл

do_write:
        mov r0, #1            @ Файловый дескриптор (stdout = 1)
        mov r7, #4            @ Номер syscall для write (в ARM32 это 4)
        swi #0                @ Системный вызов (аналог `svc` в AArch64)

        pop {r11, lr}         @ Восстанавливаем R11 и LR
        bx lr                @ Возврат из функции (аналог `ret`)

.global _exit
_exit:
        mov r7, #1            @ Номер syscall для exit (в ARM32 это 1)
        mov r0, #0            @ Код возврата 0
        swi #0)";

    return as;
}

string arm32_linux_writeText(const vector<string>& texts)
{
    stringstream as;
    
    for (size_t i = 0; i < texts.size(); ++i)
    {
        as << "\n\tldr r0, =msg" << i
           << "\n\tbl printf";
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