#ifndef DOCTOR_H
#define DOCTOR_H

#include "Person.h"

// Doctor extends Person with a medical specialization and consultation fee
class Doctor : public Person
{
private:
    char  specialization[51];   // e.g. "Cardiology", max 50 chars
    float fee;                  // consultation fee in PKR

public:
    Doctor();
    Doctor(int id, const char* name, const char* spec, const char* contact, const char* password, float fee);
    ~Doctor() override;

    // Getters
    const char* getSpecialization() const;
    float getFee() const;

    // Setters
    void setSpecialization(const char* s);
    void setFee(float f);

    // Prints full doctor info including specialization and fee
    void displayInfo(std::ostream& os) const override;

    // Returns the string "Doctor" for role identification
    const char* getRole() const override;

    bool operator==(const Doctor& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Doctor& d);
};

#endif