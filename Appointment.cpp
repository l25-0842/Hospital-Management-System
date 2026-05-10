#include "Appointment.h"
#include "MyString.h"

// Default constructor zeroes all IDs and empties every char buffer
Appointment::Appointment() : id(0), patientId(0), doctorId(0)
{
    date[0] = '\0';
    timeSlot[0] = '\0';
    status[0] = '\0';
}

// Full constructor copies strings with bounded copy to prevent overflow
Appointment::Appointment(int id_, int pid, int did, const char* d, const char* s, const char* st)
    : id(id_), patientId(pid), doctorId(did)
{
    MyStr::copyN(date, d, 11);
    MyStr::copyN(timeSlot, s, 6);
    MyStr::copyN(status, st, 15);
}

// --- Getters ---

int Appointment::getId() const
{
    return id; 
}
int Appointment::getPatientId() const
{
    return patientId;
}
int Appointment::getDoctorId() const 
{
    return doctorId; 
}
const char* Appointment::getDate() const
{
    return date;
}
const char* Appointment::getTimeSlot() const 
{
    return timeSlot; 
}
const char* Appointment::getStatus() const
{
    return status;
}

// --- Setters ---

void Appointment::setId(int i)
{
    id = i;
}

void Appointment::setPatientId(int p)
{
    patientId = p;
}

void Appointment::setDoctorId(int d)
{
    doctorId = d;
}

void Appointment::setDate(const char* d)
{
    MyStr::copyN(date, d, 11);
}

void Appointment::setTimeSlot(const char* s)
{
    MyStr::copyN(timeSlot, s, 6);
}

void Appointment::setStatus(const char* s)
{
    MyStr::copyN(status, s, 15);
}

// Collision check: same doctor, same date, same slot, and neither side is cancelled.
// Used before booking to detect slot conflicts.
bool Appointment::operator==(const Appointment& other) const
{
    if (doctorId != other.doctorId)
    {
        return false;
    }

    if (!MyStr::equals(date, other.date))
    {
        return false;
    }

    if (!MyStr::equals(timeSlot, other.timeSlot))
    {
        return false;
    }

    // Cancelled appointments do not block a slot
    if (MyStr::equals(status, "cancelled") || MyStr::equals(other.status, "cancelled"))
    {
        return false;
    }

    return true;
}

std::ostream& operator<<(std::ostream& os, const Appointment& a)
{
    os << "Appt ID: " << a.id
        << " | Status: " << a.status
        << " | Patient: " << a.patientId
        << " | Doctor: " << a.doctorId
        << " | Date: " << a.date
        << " | Slot: " << a.timeSlot;
    return os;
}