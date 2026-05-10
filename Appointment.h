#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include <ostream>

// Represents a single scheduled appointment between a patient and a doctor.
// Fixed-size char arrays are used to avoid dynamic allocation.
class Appointment
{
private:
    int  id;
    int  patientId;
    int  doctorId;
    char date[11];      // format: DD-MM-YYYY
    char timeSlot[6];   // format: HH:MM
    char status[15];    // one of: pending / completed / cancelled / no-show

public:
    Appointment();
    Appointment(int id, int pid, int did, const char* date, const char* slot, const char* status);

    // Getters
    int getId() const;
    int getPatientId() const;
    int getDoctorId() const;
    const char* getDate() const;
    const char* getTimeSlot() const;
    const char* getStatus() const;

    // Setters
    void setId(int i);
    void setPatientId(int p);
    void setDoctorId(int d);
    void setDate(const char* d);
    void setTimeSlot(const char* s);
    void setStatus(const char* s);

    // Two appointments are equal if same doctor, date, slot and neither is cancelled
    bool operator==(const Appointment& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Appointment& a);
};

#endif