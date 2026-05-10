#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "FileHandler.h"
#include "MyString.h"
#include "Exceptions.h"
#include <fstream>
#include <ctime>

// Reads one line from the stream into buf, stripping '\r' for Windows compatibility.
// Returns true if any characters were read; false at end-of-file.
static bool readLine(std::ifstream& in, char* buf, int max)
{
    int  i = 0;
    char c;
    while (in.get(c))
    {
        if (c == '\n')
        {
            buf[i] = '\0';
            return true;
        }
        if (c == '\r')
        {
            continue;   // skip carriage return
        }
        if (i < max - 1)
        {
            buf[i++] = c;
        }
    }
    buf[i] = '\0';
    return i > 0;   // true if partial last line with no trailing newline
}

//  Loaders

// Reads patients.txt; creates an empty file if it does not exist
void FileHandler::loadPatients(Storage<Patient>* s)
{
    std::ifstream in("data/patients.txt");
    if (!in.is_open())
    {
        std::ofstream o("data/patients.txt");
        return;
    }

    char line[512];
    while (readLine(in, line, 512))
    {
        if (line[0] == '\0')
        {
            continue;   // skip blank lines
        }

        // CSV columns: id, name, age, gender, contact, password, balance
        char tok[7][100];
        int  tokenCount = MyStr::split(line, ',', tok, 7);
        if (tokenCount < 7)
        {
            continue;   // malformed row
        }

        Patient p(MyStr::toInt(tok[0]), tok[1], MyStr::toInt(tok[2]), tok[3][0], tok[4], tok[5], MyStr::toFloat(tok[6]));
        s->add(p);
    }
}

// Reads doctors.txt; creates an empty file if it does not exist
void FileHandler::loadDoctors(Storage<Doctor>* s)
{
    std::ifstream in("data/doctors.txt");
    if (!in.is_open())
    {
        std::ofstream o("data/doctors.txt");
        return;
    }

    char line[512];
    while (readLine(in, line, 512))
    {
        if (line[0] == '\0')
        {
            continue;
        }

        // CSV columns: id, name, specialization, contact, password, fee
        char tok[6][100];
        int  tokenCount = MyStr::split(line, ',', tok, 6);
        if (tokenCount < 6)
        {
            continue;
        }

        Doctor d(MyStr::toInt(tok[0]), tok[1], tok[2], tok[3], tok[4], MyStr::toFloat(tok[5]));
        s->add(d);
    }
}

// Reads admin.txt; seeds a default admin account if the file is missing
void FileHandler::loadAdmins(Storage<Admin>* s)
{
    std::ifstream in("data/admin.txt");
    if (!in.is_open())
    {
        // Create the file and write the default admin credentials
        std::ofstream o("data/admin.txt");
        o << "1,Admin,admin123\n";
        o.close();
        Admin a(1, "Admin", "admin123");
        s->add(a);
        return;
    }

    char line[512];
    while (readLine(in, line, 512))
    {
        if (line[0] == '\0')
        {
            continue;
        }

        // CSV columns: id, name, password
        char tok[3][100];
        int  tokenCount = MyStr::split(line, ',', tok, 3);
        if (tokenCount < 3)
        {
            continue;
        }

        Admin a(MyStr::toInt(tok[0]), tok[1], tok[2]);
        s->add(a);
    }
}

// Reads appointments.txt; creates an empty file if it does not exist
void FileHandler::loadAppointments(Storage<Appointment>* s)
{
    std::ifstream in("data/appointments.txt");
    if (!in.is_open())
    {
        std::ofstream o("data/appointments.txt");
        return;
    }

    char line[512];
    while (readLine(in, line, 512))
    {
        if (line[0] == '\0')
        {
            continue;
        }

        // CSV columns: id, patientId, doctorId, date, timeSlot, status
        char tok[6][100];
        int  tokenCount = MyStr::split(line, ',', tok, 6);
        if (tokenCount < 6)
        {
            continue;
        }

        Appointment a(MyStr::toInt(tok[0]), MyStr::toInt(tok[1]), MyStr::toInt(tok[2]), tok[3], tok[4], tok[5]);
        s->add(a);
    }
}

// Reads bills.txt; creates an empty file if it does not exist
void FileHandler::loadBills(Storage<Bill>* s)
{
    std::ifstream in("data/bills.txt");
    if (!in.is_open())
    {
        std::ofstream o("data/bills.txt");
        return;
    }

    char line[512];
    while (readLine(in, line, 512))
    {
        if (line[0] == '\0')
        {
            continue;
        }

        // CSV columns: id, patientId, appointmentId, amount, status, date
        char tok[6][100];
        int  tokenCount = MyStr::split(line, ',', tok, 6);
        if (tokenCount < 6)
        {
            continue;
        }

        Bill b(MyStr::toInt(tok[0]), MyStr::toInt(tok[1]), MyStr::toInt(tok[2]), MyStr::toFloat(tok[3]), tok[4], tok[5]);
        s->add(b);
    }
}

// Reads prescriptions.txt; uses a custom field splitter because medicine/notes
// fields may be long and cannot share the same small token array
void FileHandler::loadPrescriptions(Storage<Prescription>* s)
{
    std::ifstream in("data/prescriptions.txt");
    if (!in.is_open())
    {
        std::ofstream o("data/prescriptions.txt");
        return;
    }

    char line[1024];
    while (readLine(in, line, 1024))
    {
        if (line[0] == '\0')
        {
            continue;
        }

        // Split manually into 7 fields to accommodate large medicine/notes columns
        char fields[7][500];
        for (int i = 0; i < 7; i++)
        {
            fields[i][0] = '\0';
        }

        int fieldIndex = 0;
        int charIndex = 0;
        int sourceIndex = 0;

        while (line[sourceIndex] != '\0' && fieldIndex < 7)
        {
            if (line[sourceIndex] == ',')
            {
                fields[fieldIndex][charIndex] = '\0';
                fieldIndex++;
                charIndex = 0;
            }
            else
            {
                if (charIndex < 499)
                {
                    fields[fieldIndex][charIndex++] = line[sourceIndex];
                }
            }
            sourceIndex++;
        }
        fields[fieldIndex][charIndex] = '\0';
        fieldIndex++;

        if (fieldIndex < 7)
        {
            continue;   // incomplete record
        }

        // CSV columns: id, appointmentId, patientId, doctorId, date, medicines, notes
        Prescription p(MyStr::toInt(fields[0]), MyStr::toInt(fields[1]),
            MyStr::toInt(fields[2]), MyStr::toInt(fields[3]), fields[4], fields[5], fields[6]);
        s->add(p);
    }
}

//  Full savers (overwrite entire file)

void FileHandler::savePatients(Storage<Patient>* s)
{
    std::ofstream out("data/patients.txt");
    for (int i = 0; i < s->size(); i++)
    {
        Patient& p = (*s)[i];
        char balanceStr[20];
        MyStr::floatToStr(p.getBalance(), balanceStr, 2);
        out << p.getId() << ","
            << p.getName() << ","
            << p.getAge() << ","
            << p.getGender() << ","
            << p.getContact() << ","
            << p.getPassword() << ","
            << balanceStr << "\n";
    }
}

void FileHandler::saveDoctors(Storage<Doctor>* s)
{
    std::ofstream out("data/doctors.txt");
    for (int i = 0; i < s->size(); i++)
    {
        Doctor& d = (*s)[i];
        char feeStr[20];
        MyStr::floatToStr(d.getFee(), feeStr, 2);
        out << d.getId() << ","
            << d.getName() << ","
            << d.getSpecialization() << ","
            << d.getContact() << ","
            << d.getPassword() << ","
            << feeStr << "\n";
    }
}

void FileHandler::saveAppointments(Storage<Appointment>* s)
{
    std::ofstream out("data/appointments.txt");
    for (int i = 0; i < s->size(); i++)
    {
        Appointment& a = (*s)[i];
        out << a.getId() << ","
            << a.getPatientId() << ","
            << a.getDoctorId() << ","
            << a.getDate() << ","
            << a.getTimeSlot() << ","
            << a.getStatus() << "\n";
    }
}

void FileHandler::saveBills(Storage<Bill>* s)
{
    std::ofstream out("data/bills.txt");
    for (int i = 0; i < s->size(); i++)
    {
        Bill& b = (*s)[i];
        char amountStr[20];
        MyStr::floatToStr(b.getAmount(), amountStr, 2);
        out << b.getId() << ","
            << b.getPatientId() << ","
            << b.getAppointmentId() << ","
            << amountStr << ","
            << b.getStatus() << ","
            << b.getDate() << "\n";
    }
}

void FileHandler::savePrescriptions(Storage<Prescription>* s)
{
    std::ofstream out("data/prescriptions.txt");
    for (int i = 0; i < s->size(); i++)
    {
        Prescription& p = (*s)[i];
        out << p.getId() << ","
            << p.getAppointmentId() << ","
            << p.getPatientId() << ","
            << p.getDoctorId() << ","
            << p.getDate() << ","
            << p.getMedicines() << ","
            << p.getNotes() << "\n";
    }
}

//  Appenders (add one record to end of file)

void FileHandler::appendPatient(const Patient* p)
{
    std::ofstream out("data/patients.txt", std::ios::app);
    char balanceStr[20];
    MyStr::floatToStr(p->getBalance(), balanceStr, 2);
    out << p->getId() << ","
        << p->getName() << ","
        << p->getAge() << ","
        << p->getGender() << ","
        << p->getContact() << ","
        << p->getPassword() << ","
        << balanceStr << "\n";
}

void FileHandler::appendDoctor(const Doctor* d)
{
    std::ofstream out("data/doctors.txt", std::ios::app);
    char feeStr[20];
    MyStr::floatToStr(d->getFee(), feeStr, 2);
    out << d->getId() << ","
        << d->getName() << ","
        << d->getSpecialization() << ","
        << d->getContact() << ","
        << d->getPassword() << ","
        << feeStr << "\n";
}

void FileHandler::appendAppointment(const Appointment* a)
{
    std::ofstream out("data/appointments.txt", std::ios::app);
    out << a->getId() << ","
        << a->getPatientId() << ","
        << a->getDoctorId() << ","
        << a->getDate() << ","
        << a->getTimeSlot() << ","
        << a->getStatus() << "\n";
}

void FileHandler::appendBill(const Bill* b)
{
    std::ofstream out("data/bills.txt", std::ios::app);
    char amountStr[20];
    MyStr::floatToStr(b->getAmount(), amountStr, 2);
    out << b->getId() << ","
        << b->getPatientId() << ","
        << b->getAppointmentId() << ","
        << amountStr << ","
        << b->getStatus() << ","
        << b->getDate() << "\n";
}

void FileHandler::appendPrescription(const Prescription* p)
{
    std::ofstream out("data/prescriptions.txt", std::ios::app);
    out << p->getId() << ","
        << p->getAppointmentId() << ","
        << p->getPatientId() << ","
        << p->getDoctorId() << ","
        << p->getDate() << ","
        << p->getMedicines() << ","
        << p->getNotes() << "\n";
}

// Writes discharged patient to a separate archive so records are not lost
void FileHandler::appendDischargedPatient(const Patient* p)
{
    std::ofstream out("data/discharged.txt", std::ios::app);
    char balanceStr[20];
    MyStr::floatToStr(p->getBalance(), balanceStr, 2);
    out << p->getId() << ","
        << p->getName() << ","
        << p->getAge() << ","
        << p->getGender() << ","
        << p->getContact() << ","
        << p->getPassword() << ","
        << balanceStr << "\n";
}

//  Security log

// Appends one timestamped login event: timestamp, role, enteredId, result
void FileHandler::logSecurity(const char* role, const char* enteredId, const char* result)
{
    std::ofstream out("data/security_log.txt", std::ios::app);

    // Get local time for the timestamp
    time_t currentTime = time(nullptr);
    tm     localTime = {};
#if defined(_MSC_VER)
    localtime_s(&localTime, &currentTime);
#else
    tm* localTimePtr = localtime(&currentTime);
    if (localTimePtr)
    {
        localTime = *localTimePtr;
    }
#endif

    char timestampBuf[32];
    strftime(timestampBuf, 32, "%d-%m-%Y %H:%M:%S", &localTime);
    out << timestampBuf << "," << role << "," << enteredId << "," << result << "\n";
}

// Reads up to maxLines entries from security_log.txt into the caller's array
void FileHandler::readSecurityLog(char lines[][256], int* lineCount, int maxLines)
{
    *lineCount = 0;
    std::ifstream in("data/security_log.txt");
    if (!in.is_open())
    {
        return;
    }

    char line[256];
    while (*lineCount < maxLines && readLine(in, line, 256))
    {
        if (line[0] == '\0')
        {
            continue;
        }

        MyStr::copyN(lines[*lineCount], line, 256);
        (*lineCount)++;
    }
}