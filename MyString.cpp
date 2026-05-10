#include "MyString.h"

namespace MyStr
{

    // Internal helper shared by length() and concat() to avoid a circular call
    static int length_impl(const char* s)
    {
        int n = 0;
        while (s && s[n] != '\0')
        {
            n++;
        }
        return n;
    }

    int length(const char* s)
    {
        return length_impl(s);
    }

    // Copies src into dest; dest must be large enough (no bounds check)
    void copy(char* dest, const char* src)
    {
        int i = 0;
        while (src[i] != '\0')
        {
            dest[i] = src[i];
            i++;
        }
        dest[i] = '\0';
    }

    // Copies at most n-1 characters from src into dest, then null-terminates
    void copyN(char* dest, const char* src, int n)
    {
        int i = 0;
        while (i < n - 1 && src[i] != '\0')
        {
            dest[i] = src[i];
            i++;
        }
        dest[i] = '\0';
    }

    bool equals(const char* a, const char* b)
    {
        int i = 0;
        while (a[i] != '\0' && b[i] != '\0')
        {
            if (a[i] != b[i])
            {
                return false;
            }
            i++;
        }
        return a[i] == b[i];   // both must reach '\0' at the same position
    }

    char myToLower(char c)
    {
        if (c >= 'A' && c <= 'Z')
        {
            return c + 32;
        }
        return c;
    }

    bool equalsIgnoreCase(const char* a, const char* b)
    {
        int i = 0;
        while (a[i] != '\0' && b[i] != '\0')
        {
            if (myToLower(a[i]) != myToLower(b[i]))
            {
                return false;
            }
            i++;
        }
        return a[i] == b[i];
    }

    // Appends src to the end of dest; dest must have enough remaining space
    void concat(char* dest, const char* src)
    {
        int destLen = length_impl(dest);
        int i = 0;
        while (src[i] != '\0')
        {
            dest[destLen + i] = src[i];
            i++;
        }
        dest[destLen + i] = '\0';
    }

    // Parses an ASCII decimal integer; handles an optional leading '-'
    int toInt(const char* s)
    {
        int result = 0;
        int i = 0;
        int sign = 1;

        if (s[0] == '-')
        {
            sign = -1;
            i = 1;
        }

        while (s[i] != '\0' && s[i] >= '0' && s[i] <= '9')
        {
            result = result * 10 + (s[i] - '0');
            i++;
        }
        return result * sign;
    }

    // Parses a decimal float with optional sign; stops at any non-digit after the decimal point
    float toFloat(const char* s)
    {
        float result = 0.f;
        int   i = 0;
        int   sign = 1;

        if (s[0] == '-')
        {
            sign = -1;
            i = 1;
        }

        // Integer part
        while (s[i] != '\0' && s[i] != '.')
        {
            if (s[i] >= '0' && s[i] <= '9')
            {
                result = result * 10.f + static_cast<float>(s[i] - '0');
            }
            i++;
        }

        // Fractional part
        if (s[i] == '.')
        {
            i++;
            float fractionalMultiplier = 0.1f;
            while (s[i] != '\0' && s[i] >= '0' && s[i] <= '9')
            {
                result += static_cast<float>(s[i] - '0') * fractionalMultiplier;
                fractionalMultiplier *= 0.1f;
                i++;
            }
        }
        return result * static_cast<float>(sign);
    }

    // Converts an integer to a null-terminated decimal string in out[]
    void intToStr(int num, char* out)
    {
        int  i = 0;
        bool isNeg = false;

        if (num < 0)
        {
            isNeg = true;
            num = -num;
        }

        if (num == 0)
        {
            out[i++] = '0';
        }

        // Build digits in reverse order, then flip
        char reverseDigits[20];
        int  digitCount = 0;
        while (num > 0)
        {
            reverseDigits[digitCount++] = static_cast<char>((num % 10) + '0');
            num /= 10;
        }

        if (isNeg)
        {
            out[i++] = '-';
        }

        for (int j = digitCount - 1; j >= 0; j--)
        {
            out[i++] = reverseDigits[j];
        }
        out[i] = '\0';
    }

    // Converts a float to string with the given number of decimal places
    void floatToStr(float num, char* out, int precision)
    {
        int integerPart = static_cast<int>(num);
        intToStr(integerPart, out);

        int  currentLen = length_impl(out);
        out[currentLen++] = '.';

        float fractionalPart = num - static_cast<float>(integerPart);
        if (fractionalPart < 0.f)
        {
            fractionalPart = -fractionalPart;
        }

        for (int i = 0; i < precision; i++)
        {
            fractionalPart *= 10.f;
            int digit = static_cast<int>(fractionalPart);
            out[currentLen++] = static_cast<char>(digit + '0');
            fractionalPart -= static_cast<float>(digit);
        }
        out[currentLen] = '\0';
    }

    void toLower(char* s)
    {
        int i = 0;
        while (s[i] != '\0')
        {
            s[i] = myToLower(s[i]);
            i++;
        }
    }

    bool isDigit(char c)
    {
        return c >= '0' && c <= '9';
    }

    // Returns true only if the string is non-empty and every character is a digit
    bool isAllDigits(const char* s)
    {
        if (s[0] == '\0')
        {
            return false;
        }

        int i = 0;
        while (s[i] != '\0')
        {
            if (!isDigit(s[i]))
            {
                return false;
            }
            i++;
        }
        return true;
    }

    // Splits line on delim and stores up to maxTokens tokens in tokens[][100].
    // Returns the number of tokens produced.
    int split(const char* line, char delim, char tokens[][100], int maxTokens)
    {
        int tokenIndex = 0;
        int charIndex = 0;
        int sourceIndex = 0;

        while (line[sourceIndex] != '\0' && tokenIndex < maxTokens)
        {
            if (line[sourceIndex] == delim)
            {
                tokens[tokenIndex][charIndex] = '\0';
                tokenIndex++;
                charIndex = 0;
            }
            else
            {
                if (charIndex < 99)
                {
                    tokens[tokenIndex][charIndex++] = line[sourceIndex];
                }
            }
            sourceIndex++;
        }

        // Terminate the final token
        if (tokenIndex < maxTokens)
        {
            tokens[tokenIndex][charIndex] = '\0';
            tokenIndex++;
        }
        return tokenIndex;
    }

}