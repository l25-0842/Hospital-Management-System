#ifndef HOSPITALSYSTEM_H
#define HOSPITALSYSTEM_H

#include "Storage.h"
#include "Patient.h"
#include "Doctor.h"
#include "Admin.h"
#include "Appointment.h"
#include "Bill.h"
#include "Prescription.h"

// Central facade that owns all in-memory data stores and exposes
// high-level operations used by the GUI (login, ID generation, date helpers).
class HospitalSystem
{
private:
    Storage<Patient>* patients;
    Storage<Doctor>* doctors;
    Storage<Admin>* admins;
    Storage<Appointment>* appointments;
    Storage<Bill>* bills;
    Storage<Prescription>* prescriptions;

public:
    HospitalSystem();
    ~HospitalSystem();

    // Loads all entity files from disk into the in-memory stores
    void loadAll();

    // --- Accessors for the storage containers ---
    Storage<Patient>* getPatients() const;
    Storage<Doctor>* getDoctors() const;
    Storage<Admin>* getAdmins() const;
    Storage<Appointment>* getAppointments() const;
    Storage<Bill>* getBills() const;
    Storage<Prescription>* getPrescriptions() const;

    // --- Login helpers: return a pointer on success, nullptr on failure ---
    Patient* loginPatient(int id, const char* pwd) const;
    Doctor* loginDoctor(int id, const char* pwd) const;
    Admin* loginAdmin(int id, const char* pwd) const;

    // --- Auto-increment ID generators (max existing + 1) ---
    int nextAppointmentId() const;
    int nextBillId() const;
    int nextPrescriptionId() const;
    int nextDoctorId() const;

    // Writes today's date (DD-MM-YYYY) into the provided 11-char buffer
    void getTodayDate(char* out) const;

    // Returns how many full calendar days have passed since dateStr
    int  daysBetween(const char* dateStr) const;
};

#endif