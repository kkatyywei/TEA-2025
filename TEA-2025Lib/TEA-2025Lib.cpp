#include "pch.h"
#include "framework.h"
#include <windows.h>
#include <iostream>
#include <locale>
#include <cmath>    // если захочешь pow
#include <cstdio>   // для _snprintf_s
#include <cstring>  // для std::strcmp

// Вспомогательная функция для инициализации кодировки (вызывается один раз)
namespace
{
    static bool consoleInitialized = false;

    void initConsoleEncoding()
    {
        if (!consoleInitialized)
        {
            SetConsoleCP(1251);
            SetConsoleOutputCP(1251);
            setlocale(LC_ALL, "Russian");
            consoleInitialized = true;
        }
    }
}

extern "C"
{
    int __stdcall power_of(int base, int exp)
    {
        if (exp < 0)
        {
            if (base == 1) return 1;
            if (base == -1) return (exp % 2 == 0) ? 1 : -1;
            return 0;
        }

        if (base == 0) return (exp == 0) ? 1 : 0; 
        if (exp == 0) return 1;                   
        if (base == 1) return 1;
        if (base == -1) return (exp % 2 == 0) ? 1 : -1;

        // Используем 64-битный аккумулятор, чтобы не потерять переполнение
        long long res = 1;
        long long l_base = base;

        for (int i = 0; i < exp; ++i)
        {
            res *= l_base;

            if (res > INT_MAX || res < INT_MIN)
            {
                initConsoleEncoding();

                std::cout << "\nRuntime Error: Integer overflow in power_of operation!" << std::endl;
                std::cout << "Attempted: " << base << " ^ " << exp << std::endl;

                exit(-1);
            }
        }

        return (int)res;
    }

    const char* __stdcall to_str(int value)
    {
        static char buffer[32]; 
        _snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "%d", value);
        return buffer;
    }


    void __stdcall prints(const char* str)
    {
        if (str == nullptr) { std::cout << "(null)" << std::endl; return; }
        initConsoleEncoding();
        std::cout << str << std::endl;
    }


    void __stdcall printi(int value)
    {
        initConsoleEncoding();
        std::cout << value << std::endl;
    }


    void __stdcall printb(unsigned int value)
    {
        initConsoleEncoding();
        // Если 0 -> false, всё остальное -> true
        if (value == 0)
            std::cout << "false" << std::endl;
        else
            std::cout << "true" << std::endl;
    }
}