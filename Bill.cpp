#include "Bill.h"
#include "MyString.h"

// Default constructor zeroes all numeric fields and clears char buffers
Bill::Bill() : id(0), patientId(0), appointmentId(0), amount(0.f)
{
    status[0] = '\0';
    date[0] = '\0';
}

// Full constructor; copies bounded strings for status and date
Bill::Bill(int id_, int pid, int aid, float amt, const char* st, const char* d)
    : id(id_), patientId(pid), appointmentId(aid), amount(amt)
{
    MyStr::copyN(status, st, 10);
    MyStr::copyN(date, d, 11);
}

// --- Getters ---

int Bill::getId() const
{
    return id;
}
int Bill::getPatientId() const
{
    return patientId;
}
int Bill::getAppointmentId() const
{
    return appointmentId;
}
float Bill::getAmount() const
{
    return amount;
}
const char* Bill::getStatus() const
{
    return status;
}
const char* Bill::getDate() const
{
    return date;
}

// --- Setters ---

void Bill::setId(int i)
{
    id = i;
}

void Bill::setPatientId(int p)
{
    patientId = p;
}

void Bill::setAppointmentId(int a)
{
    appointmentId = a;
}

void Bill::setAmount(float a)
{
    amount = a;
}

void Bill::setStatus(const char* s)
{
    MyStr::copyN(status, s, 10);
}

void Bill::setDate(const char* d)
{
    MyStr::copyN(date, d, 11);
}