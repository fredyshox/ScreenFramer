//
// Created by Kacper Rączy on 26/07/2020.
//

#ifndef SCREENFRAMER_UTILITY_HPP
#define SCREENFRAMER_UTILITY_HPP

#include <iostream>

#ifdef DEBUG
#define DEBUG_PRINTLN(str) do { std::cerr << str << std::endl; } while (false)
#define DEBUG_PRINT(str) do { std::cerr << str; } while (false)
#else
#define DEBUG_PRINTLN(str) do { } while (false)
#define DEBUG_PRINT(str) do { } while (false)
#endif

#endif //SCREENFRAMER_UTILITY_HPP
