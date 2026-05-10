#include "Patient.h"
#include "MyString.h"

// Default constructor zeroes numeric fields; Person default handles name/contact
Patient::Patient() : Person(), age(0), gender('M'), balance(0.f)
{
}

// Full constructor; note Person takes (id, name, password, contact) order
Patient::Patient(int id, const char* n, int age_, char g, const char* c, const char* p, float b)
    : Person(id, n, p, c), age(age_), gender(g), balance(b)
{
}

Patient::~Patient()
{
}

// --- Getters ---

int Patient::getAge() const 
{
    return age; 
}
char Patient::getGender() const 
{
    return gender; 
}
float Patient::getBalance() const 
{
    return balance; 
}

// --- Setters ---

void Patient::setAge(int a) 
{
    age = a;
}
void Patient::setGender(char g) 
{
    gender = g; 
}
void Patient::setBalance(float b)
{
    balance = b; 
}

// Prints a one-line patient summary including wallet balance
void Patient::displayInfo(std::ostream& os) const
{
    os << "Patient ID: " << id
        << " | Name: " << name
        << " | Age: " << age
        << " | Gender: " << gender
        << " | Balance: PKR " << balance;
}

// Returns "Patient" used by the login and role-check system
const char* Patient::getRole() const
{
    return "Patient";
}

// Adds funds to the wallet; used when the patient tops up or receives a refund
Patient& Patient::operator+=(float amount)
{
    balance += amount;
    return *this;
}

// Deducts funds from the wallet; used when booking or paying a bill
Patient& Patient::operator-=(float amount)
{
    balance -= amount;
    return *this;
}

// Equality is based solely on ID — consistent with Storage::findById
bool Patient::operator==(const Patient& other) const
{
    return id == other.id;
}

std::ostream& operator<<(std::ostream& os, const Patient& p)
{
    p.displayInfo(os);
    return os;
}