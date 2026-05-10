#include "Person.h"
#include "MyString.h"

Person::Person() : id(0)
{
    name[0] = '\0';
    password[0] = '\0';
    contact[0] = '\0';
}

Person::Person(int id, const char* n, const char* p, const char* c) : id(id)
{
    MyStr::copyN(name, n, 51);
    MyStr::copyN(password, p, 51);
    MyStr::copyN(contact, c, 12);
}

Person::~Person() {}

int Person::getId() const
{
    return id;
}

const char* Person::getName() const
{
    return name;
}

const char* Person::getPassword() const
{
    return password;
}

const char* Person::getContact() const
{
    return contact;
}

void Person::setId(int i)
{
    id = i;
}

void Person::setName(const char* n)
{
    MyStr::copyN(name, n, 51);
}

void Person::setPassword(const char* p)
{
    MyStr::copyN(password, p, 51);
}

void Person::setContact(const char* c)
{
    MyStr::copyN(contact, c, 12);
}