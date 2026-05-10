#ifndef PATIENT_H
#define PATIENT_H

#include "Person.h"

class Patient : public Person
{
private:

    int age;
    char gender;
    float balance;

public:

    // Constructors
    Patient();
    Patient(int id, const char* name, int age, char gender, const char* contact, const char* password, float balance);
    ~Patient() override;

    // Getters
    int getAge() const;
    char getGender() const;
    float getBalance() const;

    // Setters
    void setAge(int a);
    void setGender(char g);
    void setBalance(float b);

    // Display patient info
    void displayInfo(std::ostream& os) const override;

    // Returns role
    const char* getRole() const override;

    // Balance operators
    Patient& operator+=(float amount);
    Patient& operator-=(float amount);

    // Compare patients
    bool operator==(const Patient& other) const;

    friend std::ostream& operator<<(std::ostream& os, const Patient& p);
};

#endif