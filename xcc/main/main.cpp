#include <iostream>
#include <fstream>
#include <cstdlib>
#include <regex>
#include <vector>
#include <cctype>
#include "include/code_gen/main.h"

int main(int argc, char *argv[])
{
    if (argc != 5 || string(argv[1]).rfind("-target=", 0) != 0 || string(argv[2]) != "-o")
    {
        cerr << "\x1B[31merror:\033[0m\tusage: ./xcc -target=<target-triple> -o <input.c> <output>\n";

        return 1;
    }
    
    string targetTriple = string(argv[1]).substr(8);
    string inputFile = argv[3];
    string outputFile = argv[4];

    bool x86_64_linux = false;
    bool i386_linux = false;
    bool arm64_linux = false;
    bool arm32_linux = false;
    bool x86_64_darwin = false;
    bool arm64_darwin = false;

    if (targetTriple == "x86_64-gnu-linux")
    {
        x86_64_linux = true;
    }
    else if (targetTriple == "i386-gnu-linux")
    {
        i386_linux = true;
    }
    else if (targetTriple == "arm64-gnu-linux")
    {
        arm64_linux = true;
    }
    else if (targetTriple == "arm32-gnu-linux")
    {
        arm32_linux = true;
    }
    else if (targetTriple == "x86_64-apple-darwin")
    {
        x86_64_darwin = true;
    }
    else if (targetTriple == "aarch64-apple-darwin")
    {
        arm64_darwin = true;
    }
    else
    {
        cerr << "\x1B[31merror:\033[0m\tunknown target triple: " << targetTriple << "\n\n";
        return 1;
    }

    ifstream file(inputFile);
    if (!file.is_open())
    {
        cerr << "Не удалось открыть файл: " << inputFile << "\n";
        return 1;
    }

    string source((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    
    /* host */
    cout << "host: " << getHost() << "\n";

    /* target */

    cout << "target: " << targetTriple << "\n\n";

    vector<string> lines = splitToLines(source);

    vector<string> texts;
    string as;
    for (string& l : lines)
    {
        l = deleteSpacesL(l);
        cout << l << "\n";
        if (startswith(l, "printf") != 0)
        {
            string code = string(l).substr(6);
            code = deleteSpacesL(code);
            code = string(code).substr(2);
            string text = removeLastChars(code, 3);
            cout << text << "\n";
            texts.push_back(text);
        }
    }
    if (x86_64_linux)
    {
                string call_main = "\n.global _start\n_start:\n\tcall main\n";
                vector<CFunction> functions = extractFunctions(source);
                string func;
                for (CFunction& function : functions)
                {
                    func += "\n" + function.globl + " " + function.name + ":\n";
                }
                as = x86_64_linux_include(source) + x86_64_linux_base() + as;
                func += x86_64_linux_writeText(texts);
                as += call_main + func;
                cout << "\n\n" << as << "\n";
                ofstream output("temp.s");
                if (output.is_open())
                {
                    output << as;
                    output.close();
                    string compilate = " x86_64-linux-gnu-as temp.s -o temp.o &&  x86_64-linux-gnu-ld temp.o -o " + outputFile + " && rm temp.o temp.s";
                    system(compilate.c_str());
                }
                else
                {
                    cerr << "\x1B[31merror:\033[0m\tcompilation error\n";
                    return 1;
                }
    }
    else if (i386_linux)
    {
                string call_main = "\n.global _start\n_start:\n\tcall main\n";  
                vector<CFunction> functions = extractFunctions(source);
                string func;
                for (CFunction& function : functions)
                {
                    func += "\n" + function.globl + " " + function.name + ":\n";
                }
                as = i386_linux_include(source) + i386_linux_base() + as;
                func += i386_linux_writeText(texts);
                as += call_main + func;
                cout << "\n\n" << as << "\n";
                ofstream output("temp.s");
                if (output.is_open())
                {
                    output << as;
                    output.close();
                    string compilate = "as --32 temp.s -o temp.o && ld -m elf_i386 temp.o -o " + outputFile + " && rm temp.o temp.s";
                    system(compilate.c_str());
                }
                else
                {
                    cerr << "\x1B[31merror:\033[0m\tcompilation error\n";
                    return 1;
                }
    }
    else if (arm64_linux)
    {
                string call_main = "\n.global _start\n_start:\n\tbl main\n";
                vector<CFunction> functions = extractFunctions(source);
                string func;
                for (CFunction& function : functions)
                {
                    func += "\n" + function.globl + " " + function.name + ":\n";
                }
                as = arm64_linux_include(source) + arm64_linux_base() + as;
                func += arm64_linux_writeText(texts);
                as += call_main + func;
                cout << "\n\n" << as << "\n";
                ofstream output("temp.s");
                if (output.is_open())
                {
                    output << as;
                    output.close();
                    string compilate = "aarch64-linux-gnu-as temp.s -o temp.o && aarch64-linux-gnu-ld temp.o -o " + outputFile + " && rm temp.o temp.s";
                    system(compilate.c_str());
                }
                else
                {
                    cerr << "\x1B[31merror:\033[0m\tcompilation error\n";
                    return 1;
                }
    }
    else if (arm32_linux)
    {
        string call_main = "\n.global _start\n_start:\n\tbl main\n";
        vector<CFunction> functions = extractFunctions(source);
        string func;
        for (CFunction& function : functions)
        {
            func += "\n" + function.globl + " " + function.name + ":\n";
        }
        as = arm32_linux_include(source) + arm32_linux_base() + as;
        func += arm32_linux_writeText(texts);
        as += call_main + func;
        cout << "\n\n" << as << "\n";
        ofstream output("temp.s");
        if (output.is_open())
        {
            output << as;
            output.close();
            string compilate = "arm-linux-gnueabi-as temp.s -o temp.o && arm-linux-gnueabi-ld temp.o -o " + outputFile + " && rm temp.o temp.s";
            system(compilate.c_str());
        }
        else
        {
            cerr << "\x1B[31merror:\033[0m\tcompilation error\n";
            return 1;
        }
    }
    else if (x86_64_darwin)
    {
        vector<CFunction> functions = extractFunctions(source);
        string func;
        for (CFunction& function : functions)
        {
            func += "\n" + function.globl + " " + function.name + ":\n";
        }
        as = x86_64_darwin_include(source) + x86_64_darwin_base() + as;
        func = x86_64_darwin_writeText(texts);
        as += "\n_main:\n" +  func;
        cout << "\n\n" << as << "\n";
        ofstream output("temp.s");
        if (output.is_open())
        {
            output << as;
            output.close();
            string compilate = "o64-clang temp.s -o " + outputFile;
            system(compilate.c_str());
        }
        else
        {
            cerr << "\x1B[31merror:\033[0m\tcompilation error\n";
            return 1;
        }
    }

    return 0;
}