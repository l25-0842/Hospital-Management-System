#include "Admin.h"

// Default constructor delegates to Person default
Admin::Admin() : Person()
{
}

// Parameterised constructor; contact is not needed for admin so passed as empty
Admin::Admin(int id, const char* n, const char* p) : Person(id, n, p, "")
{
}

Admin::~Admin()
{
}

// Displays a one-line summary of this admin to the provided stream
void Admin::displayInfo(std::ostream& os) const
{
    os << "Admin ID: " << id << " | Name: " << name;
}

// Returns a fixed role label used by the login system
const char* Admin::getRole() const
{
    return "Admin";
}