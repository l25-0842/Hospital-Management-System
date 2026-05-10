#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Validator.h"
#include "MyString.h"
#include <ctime>

// Get current year
int Validator::getCurrentYear()
{
    time_t currentTime = time(nullptr);
    tm localTime = {};

#if defined(_MSC_VER)

    localtime_s(&localTime, &currentTime);

#else

    tm* localTimePointer = localtime(&currentTime);

    if (localTimePointer)
    {
        localTime = *localTimePointer;
    }

#endif

    return localTime.tm_year + 1900;
}

// Validate id
bool Validator::isValidId(const char* inputString)
{
    return MyStr::isAllDigits(inputString) && MyStr::toInt(inputString) > 0;
}

// Validate date
bool Validator::isValidDate(const char* inputDate)
{
    if (MyStr::length(inputDate) != 10)
    {
        return false;
    }

    if (inputDate[2] != '-' || inputDate[5] != '-')
    {
        return false;
    }

    for (int index = 0; index < 10; index++)
    {
        if (index == 2 || index == 5)
        {
            continue;
        }

        if (!MyStr::isDigit(inputDate[index]))
        {
            return false;
        }
    }

    int day = (inputDate[0] - '0') * 10 + (inputDate[1] - '0');

    int month = (inputDate[3] - '0') * 10 + (inputDate[4] - '0');

    int year = (inputDate[6] - '0') * 1000 + (inputDate[7] - '0') * 100 + (inputDate[8] - '0') * 10 
        + (inputDate[9] - '0');

    if (day < 1 || day > 31)
    {
        return false;
    }

    if (month < 1 || month > 12)
    {
        return false;
    }

    if (year < getCurrentYear())
    {
        return false;
    }

    return true;
}

// Validate time slot
bool Validator::isValidTimeSlot(const char* inputSlot)
{
    const char* validSlots[] = { "09:00", "10:00", "11:00", "12:00", "13:00", "14:00", "15:00", "16:00" };

    for (int index = 0; index < 8; index++)
    {
        if (MyStr::equals(inputSlot, validSlots[index]))
        {
            return true;
        }
    }

    return false;
}

// Validate contact
bool Validator::isValidContact(const char* contactNumber)
{
    return MyStr::length(contactNumber) == 11 && MyStr::isAllDigits(contactNumber);
}

// Validate password
bool Validator::isValidPassword(const char* password)
{
    return MyStr::length(password) >= 6;
}

// Validate positive float
bool Validator::isPositiveFloat(const char* inputString)
{
    int dotCount = 0;
    int index = 0;

    if (inputString[0] == '\0')
    {
        return false;
    }

    while (inputString[index] != '\0')
    {
        if (inputString[index] == '.')
        {
            dotCount++;

            if (dotCount > 1)
            {
                return false;
            }
        }
        else if (!MyStr::isDigit(inputString[index]))
        {
            return false;
        }

        index++;
    }

    return MyStr::toFloat(inputString) > 0.f;
}