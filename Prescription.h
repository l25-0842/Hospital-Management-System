#ifndef PRESCRIPTION_H
#define PRESCRIPTION_H

class Prescription
{
private:
    int id;
    int appointmentId;
    int patientId;
    int doctorId;
    char date[11];
    char medicines[500];
    char notes[300];

public:
    Prescription();
    Prescription(int id, int aid, int pid, int did, const char* date, const char* meds, const char* notes);

    int getId() const;
    int getAppointmentId() const;
    int getPatientId() const;
    int getDoctorId() const;
    const char* getDate() const;
    const char* getMedicines() const;
    const char* getNotes() const;

    void setId(int i);
    void setAppointmentId(int a);
    void setPatientId(int p);
    void setDoctorId(int d);
    void setDate(const char* d);
    void setMedicines(const char* m);
    void setNotes(const char* n);
};

#endif
