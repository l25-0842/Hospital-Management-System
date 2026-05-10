#include "Prescription.h"
#include "MyString.h"

Prescription::Prescription()
    : id(0), appointmentId(0), patientId(0), doctorId(0)
{
    date[0]      = '\0';
    medicines[0] = '\0';
    notes[0]     = '\0';
}

Prescription::Prescription(int id_, int aid, int pid, int did, const char* d, const char* m, const char* n)
    : id(id_), appointmentId(aid), patientId(pid), doctorId(did)
{
    MyStr::copyN(date, d, 11);
    MyStr::copyN(medicines, m, 500);
    MyStr::copyN(notes, n, 300);
}

int Prescription::getId() const
{
    return id;
}
int Prescription::getAppointmentId() const
{
    return appointmentId;
}
int Prescription::getPatientId() const
{
    return patientId;
}
int Prescription::getDoctorId() const
{
    return doctorId;
}
const char* Prescription::getDate() const
{
    return date;
}
const char* Prescription::getMedicines() const
{
    return medicines;
}
const char* Prescription::getNotes() const
{
    return notes;
}

void Prescription::setId(int prescriptionId)
{
    id = prescriptionId;
}
void Prescription::setAppointmentId(int appointmentID)
{
    appointmentId = appointmentID;
}
void Prescription::setPatientId(int patientID)
{
    patientId = patientID;
}
void Prescription::setDoctorId(int doctorID)
{
    doctorId = doctorID;
}
void Prescription::setDate(const char* prescriptionDate)
{
    MyStr::copyN(date, prescriptionDate, 11);
}
void Prescription::setMedicines(const char* medicineDetails)
{
    MyStr::copyN(medicines, medicineDetails, 500);
}
void Prescription::setNotes(const char* prescriptionNotes)
{
    MyStr::copyN(notes, prescriptionNotes, 300);
}