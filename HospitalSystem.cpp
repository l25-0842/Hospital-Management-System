#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "HospitalSystem.h"
#include "FileHandler.h"
#include "MyString.h"
#include <ctime>

// Allocates all six storage containers on the heap
HospitalSystem::HospitalSystem()
{
    patients = new Storage<Patient>();
    doctors = new Storage<Doctor>();
    admins = new Storage<Admin>();
    appointments = new Storage<Appointment>();
    bills = new Storage<Bill>();
    prescriptions = new Storage<Prescription>();
}

// Frees every storage container; no data is written back here
HospitalSystem::~HospitalSystem()
{
    delete patients;
    delete doctors;
    delete admins;
    delete appointments;
    delete bills;
    delete prescriptions;
}

// Delegates to FileHandler to populate all stores from the data/ directory
void HospitalSystem::loadAll()
{
    FileHandler::loadPatients(patients);
    FileHandler::loadDoctors(doctors);
    FileHandler::loadAdmins(admins);
    FileHandler::loadAppointments(appointments);
    FileHandler::loadBills(bills);
    FileHandler::loadPrescriptions(prescriptions);
}

// --- Storage accessors ---

Storage<Patient>* HospitalSystem::getPatients() const 
{
    return patients;
}
Storage<Doctor>* HospitalSystem::getDoctors() const 
{
    return doctors; 
}
Storage<Admin>* HospitalSystem::getAdmins() const 
{
    return admins;
}
Storage<Appointment>* HospitalSystem::getAppointments() const 
{
    return appointments; 
}
Storage<Bill>* HospitalSystem::getBills() const 
{
    return bills;
}
Storage<Prescription>* HospitalSystem::getPrescriptions() const 
{
    return prescriptions;
}

// --- Login helpers ---

// Finds the patient by ID and compares passwords; returns nullptr if either fails
Patient* HospitalSystem::loginPatient(int id, const char* pwd) const
{
    Patient* p = patients->findById(id);
    if (p && MyStr::equals(p->getPassword(), pwd))
    {
        return p;
    }
    return nullptr;
}

Doctor* HospitalSystem::loginDoctor(int id, const char* pwd) const
{
    Doctor* d = doctors->findById(id);
    if (d && MyStr::equals(d->getPassword(), pwd))
    {
        return d;
    }
    return nullptr;
}

Admin* HospitalSystem::loginAdmin(int id, const char* pwd) const
{
    Admin* a = admins->findById(id);
    if (a && MyStr::equals(a->getPassword(), pwd))
    {
        return a;
    }
    return nullptr;
}

// --- ID generators: scan for the current maximum and return max + 1 ---

int HospitalSystem::nextAppointmentId() const
{
    int maxId = 0;
    for (int i = 0; i < appointments->size(); i++)
    {
        if ((*appointments)[i].getId() > maxId)
        {
            maxId = (*appointments)[i].getId();
        }
    }
    return maxId + 1;
}

int HospitalSystem::nextBillId() const
{
    int maxId = 0;
    for (int i = 0; i < bills->size(); i++)
    {
        if ((*bills)[i].getId() > maxId)
        {
            maxId = (*bills)[i].getId();
        }
    }
    return maxId + 1;
}

int HospitalSystem::nextPrescriptionId() const
{
    int maxId = 0;
    for (int i = 0; i < prescriptions->size(); i++)
    {
        if ((*prescriptions)[i].getId() > maxId)
        {
            maxId = (*prescriptions)[i].getId();
        }
    }
    return maxId + 1;
}

int HospitalSystem::nextDoctorId() const
{
    int maxId = 0;
    for (int i = 0; i < doctors->size(); i++)
    {
        if ((*doctors)[i].getId() > maxId)
        {
            maxId = (*doctors)[i].getId();
        }
    }
    return maxId + 1;
}

// Fills out[] with today's date in DD-MM-YYYY format (needs 11 bytes including null)
void HospitalSystem::getTodayDate(char* out) const
{
    time_t currentTime = time(nullptr);
    tm localTime = {};
#if defined(_MSC_VER)
    localtime_s(&localTime, &currentTime);
#else
    tm* localTimePtr = localtime(&currentTime);
    if (localTimePtr)
    {
        localTime = *localTimePtr;
    }
#endif
    strftime(out, 11, "%d-%m-%Y", &localTime);
}

// Parses dateStr (DD-MM-YYYY) into a tm, converts to time_t, then subtracts
// from now to get the number of elapsed days — used for overdue bill detection
int HospitalSystem::daysBetween(const char* dateStr) const
{
    int day = (dateStr[0] - '0') * 10 + (dateStr[1] - '0');
    int month = (dateStr[3] - '0') * 10 + (dateStr[4] - '0');
    int year = (dateStr[6] - '0') * 1000 + (dateStr[7] - '0') * 100 + (dateStr[8] - '0') * 10 + (dateStr[9] - '0');

    tm billDate;
    memset(&billDate, 0, sizeof(tm));
    billDate.tm_mday = day;
    billDate.tm_mon = month - 1;   // tm_mon is 0-based
    billDate.tm_year = year - 1900; // tm_year is years since 1900
    billDate.tm_isdst = -1;

    time_t billTime = mktime(&billDate);
    time_t nowTime = time(nullptr);

    return static_cast<int>(difftime(nowTime, billTime) / (60.0 * 60.0 * 24.0));
}