#ifndef PERSON_H
#define PERSON_H

#include <ostream>

class Person
{
protected:
    int  id;
    char name[51];
    char password[51];
    char contact[12];

public:
    Person();
    Person(int id, const char* name, const char* password, const char* contact);
    virtual ~Person();


    int getId() const;
    const char* getName()  const;
    const char* getPassword() const;
    const char* getContact() const;

    void setId(int id);
    void setName(const char* n);
    void setPassword(const char* p);
    void setContact(const char* c);

    virtual void displayInfo(std::ostream& os) const = 0;
    virtual const char* getRole() const = 0;
};

#endif
