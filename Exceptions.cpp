#include "Exceptions.h"
#include "MyString.h"

// Copies the message into the fixed buffer; truncates silently if over 200 chars
HospitalException::HospitalException(const char* msg)
{
    MyStr::copyN(message, msg, 200);
}

HospitalException::~HospitalException()
{
}

const char* HospitalException::what() const
{
    return message;
}

// Prepends "File not found: " then appends the filename that caused the error
FileNotFoundException::FileNotFoundException(const char* fname) : HospitalException("File not found: ")
{
    MyStr::concat(message, fname);
}

InsufficientFundsException::InsufficientFundsException(const char* msg) : HospitalException(msg)
{
}

InvalidInputException::InvalidInputException(const char* msg) : HospitalException(msg)
{
}

SlotUnavailableException::SlotUnavailableException(const char* msg) : HospitalException(msg)
{
}