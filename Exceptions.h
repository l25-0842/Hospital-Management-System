#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

// Base class for all hospital-domain exceptions.
// Uses a fixed-size char array instead of std::string to stay dependency-free.
class HospitalException
{
protected:
    char message[200];   // human-readable error description

public:
    HospitalException(const char* msg);
    virtual ~HospitalException();

    // Returns the stored error message; compatible with standard catch-and-print patterns
    virtual const char* what() const;
};

// Thrown when a required data file cannot be opened for reading
class FileNotFoundException : public HospitalException
{
public:
    FileNotFoundException(const char* fname);
};

// Thrown when a patient's wallet balance is too low to complete a payment
class InsufficientFundsException : public HospitalException
{
public:
    InsufficientFundsException(const char* msg);
};

// Thrown when user-supplied data fails validation (bad ID, date, etc.)
class InvalidInputException : public HospitalException
{
public:
    InvalidInputException(const char* msg);
};

// Thrown when an appointment slot is already taken by another booking
class SlotUnavailableException : public HospitalException
{
public:
    SlotUnavailableException(const char* msg);
};

#endif