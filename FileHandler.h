#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include "Storage.h"
#include "Patient.h"
#include "Doctor.h"
#include "Admin.h"
#include "Appointment.h"
#include "Bill.h"
#include "Prescription.h"

// Pure-static utility class that handles all file I/O for the system.
// Each entity type has three operations: load (bulk read), save (full overwrite),
// and append (single-record write used after new entries are created at runtime).
class FileHandler
{
public:
    // --- Bulk loaders: read entire file into the given Storage container ---
    static void loadPatients(Storage<Patient>* s);
    static void loadDoctors(Storage<Doctor>* s);
    static void loadAdmins(Storage<Admin>* s);
    static void loadAppointments(Storage<Appointment>* s);
    static void loadBills(Storage<Bill>* s);
    static void loadPrescriptions(Storage<Prescription>* s);

    // --- Full savers: overwrite the entire file with current in-memory data ---
    static void savePatients(Storage<Patient>* s);
    static void saveDoctors(Storage<Doctor>* s);
    static void saveAppointments(Storage<Appointment>* s);
    static void saveBills(Storage<Bill>* s);
    static void savePrescriptions(Storage<Prescription>* s);

    // --- Appenders: add a single new record to the end of the file ---
    static void appendPatient(const Patient* p);
    static void appendDoctor(const Doctor* d);
    static void appendAppointment(const Appointment* a);
    static void appendBill(const Bill* b);
    static void appendPrescription(const Prescription* p);

    // Writes a discharged patient to a separate archive file
    static void appendDischargedPatient(const Patient* p);

    // Security log: write one login event, then read all logged events back
    static void logSecurity(const char* role, const char* enteredId, const char* result);
    static void readSecurityLog(char lines[][256], int* lineCount, int maxLines);
};

#endif