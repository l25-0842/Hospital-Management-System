#ifndef MYSTRING_H
#define MYSTRING_H

// Custom string utility namespace — replaces <cstring> and <cstdlib> functions
// to avoid unsafe library calls and stay compatible with the project's no-STL policy.
namespace MyStr
{
    int length(const char* s);
    void copy(char* dest, const char* src);
    void copyN(char* dest, const char* src, int n);         // bounded copy; always null-terminates
    bool equals(const char* a, const char* b);
    bool equalsIgnoreCase(const char* a, const char* b);
    void concat(char* dest, const char* src);                // appends src to dest in-place
    int toInt(const char* s);
    float toFloat(const char* s);
    void intToStr(int num, char* out);
    void floatToStr(float num, char* out, int precision = 2);
    void toLower(char* s);
    char myToLower(char c);
    bool isDigit(char c);
    bool isAllDigits(const char* s);

    // Splits line on delim into tokens[][100]; returns number of tokens found
    int split(const char* line, char delim, char tokens[][100], int maxTokens);
}

#endif