#ifndef BILL_H
#define BILL_H

// Represents a financial record linked to one appointment and one patient.
// Status is one of: unpaid / paid / cancelled.
class Bill
{
private:
    int id;
    int patientId;
    int appointmentId;
    float amount;
    char status[10];   // unpaid / paid / cancelled
    char date[11];     // date the bill was issued: DD-MM-YYYY

public:
    Bill();
    Bill(int id, int pid, int aid, float amt, const char* st, const char* d);

    // Getters
    int getId() const;
    int getPatientId() const;
    int getAppointmentId() const;
    float getAmount() const;
    const char* getStatus() const;
    const char* getDate() const;

    // Setters
    void setId(int i);
    void setPatientId(int p);
    void setAppointmentId(int a);
    void setAmount(float a);
    void setStatus(const char* s);
    void setDate(const char* d);
};

#endif