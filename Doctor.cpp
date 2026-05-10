#include "Doctor.h"
#include "MyString.h"

// Default constructor initialises fee to zero and clears specialization
Doctor::Doctor() : Person(), fee(0.f)
{
    specialization[0] = '\0';
}

// Full constructor; note parameter order: id, name, specialization, contact, password, fee
Doctor::Doctor(int id, const char* n, const char* s, const char* c, const char* p, float f) : Person(id, n, p, c), fee(f)
{
    MyStr::copyN(specialization, s, 51);
}

Doctor::~Doctor()
{
}

// --- Getters ---

const char* Doctor::getSpecialization() const
{
    return specialization;
}
float Doctor::getFee() const 
{
    return fee;
}

// --- Setters ---

void Doctor::setSpecialization(const char* s)
{
    MyStr::copyN(specialization, s, 51);
}

void Doctor::setFee(float f)
{
    fee = f;
}

// Prints a formatted one-line summary used in admin and patient views
void Doctor::displayInfo(std::ostream& os) const
{
    os << "Doctor ID: " << id
        << " | Name: " << name
        << " | Specialization: " << specialization
        << " | Fee: PKR " << fee;
}

// Returns "Doctor" to distinguish from Patient and Admin in role checks
const char* Doctor::getRole() const
{
    return "Doctor";
}

// Equality is based solely on ID — used by Storage::findById internally
bool Doctor::operator==(const Doctor& other) const
{
    return id == other.id;
}

std::ostream& operator<<(std::ostream& os, const Doctor& d)
{
    d.displayInfo(os);
    return os;
}