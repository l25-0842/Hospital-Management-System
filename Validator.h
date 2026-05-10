#ifndef VALIDATOR_H
#define VALIDATOR_H

class Validator
{
public:
    static bool isValidId(const char* s);
    static bool isValidDate(const char* date);
    static bool isValidTimeSlot(const char* slot);
    static bool isValidContact(const char* c);
    static bool isValidPassword(const char* p);
    static bool isPositiveFloat(const char* s);
    static bool isMenuChoice(const char* s, int min, int max);
    static int  getCurrentYear();
};

#endif
