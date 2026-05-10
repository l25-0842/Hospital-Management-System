#ifndef ADMIN_H
#define ADMIN_H

#include "Person.h"

// Admin is a Person with full system access; stores only role identity
class Admin : public Person
{
public:
    Admin();
    Admin(int id, const char* name, const char* password);
    ~Admin() override;

    // Prints admin ID and name to the given output stream
    void displayInfo(std::ostream& os) const override;

    // Returns the string "Admin" to identify this role
    const char* getRole() const override;
};

#endif