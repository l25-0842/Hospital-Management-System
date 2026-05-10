#ifndef GUI_H
#define GUI_H

#include <SFML/Graphics.hpp>
#include "HospitalSystem.h"

// ---- Palette: dark medical colour theme ----
namespace Pal
{
    inline sf::Color bg()   // window background
    {
        return sf::Color(18, 22, 36);
    }
    inline sf::Color panel()    // side/secondary panels
    {
        return sf::Color(26, 32, 50);
    }
    inline sf::Color card()   // list row background
    {
        return sf::Color(34, 42, 64);
    }
    inline sf::Color accent()   // primary blue
    {
        return sf::Color(56, 182, 255);
    }
    inline sf::Color accentDk()   // darker blue for variety
    {
        return sf::Color(30, 120, 190);
    }
    inline sf::Color success()   // green for paid/completed
    {
        return sf::Color(46, 204, 113);
    }
    inline sf::Color danger()   // red for errors/overdue
    {
        return sf::Color(231, 76, 60);
    }
    inline sf::Color warning()   // yellow for pending
    {
        return sf::Color(241, 196, 15);
    }
    inline sf::Color textPri()    // primary text
    {
        return sf::Color(230, 235, 255);
    }
    inline sf::Color textSec()    // secondary/dim text
    {
        return sf::Color(130, 145, 180);
    }
    inline sf::Color border()   // card/input borders
    {
        return sf::Color(50, 62, 90);
    }
    inline sf::Color inputBg()  // text input background
    {
        return sf::Color(22, 28, 44);
    }
    inline sf::Color hdr()   // top bar / table header
    {
        return sf::Color(14, 18, 30);
    }
}

// ---- Button: stores geometry and label as plain data (no sf::Text member) ----
struct Button
{
    sf::RectangleShape rect;
    char labelStr[80];
    sf::Color fillColor;
    sf::Color textColor;
    bool outline;      // true = transparent fill with coloured border
    const sf::Font* font;

    Button() : fillColor(sf::Color(56, 182, 255)), textColor(sf::Color::White), outline(false), font(nullptr)
    {
        labelStr[0] = '\0';
    }
};

// ---- TextBox: stores geometry, content, and display hints as plain data ----
struct TextBox
{
    sf::RectangleShape rect;
    char content[256];
    char placeholderStr[80];
    char label[60];
    int len;           // current number of typed characters
    bool active;        // true when this box has keyboard focus
    bool isPassword;    // true = show asterisks instead of characters
    const sf::Font* font;

    TextBox() : len(0), active(false), isPassword(false), font(nullptr)
    {
        content[0] = '\0';
        placeholderStr[0] = '\0';
        label[0] = '\0';
    }
};

// ---- Snapshot of mouse state taken once per frame to keep click detection consistent ----
struct InputState
{
    sf::Vector2f mousePos = sf::Vector2f(0.f, 0.f);
    bool mouseDown = false;
    bool mouseClicked = false;   // true only on the rising edge (press, not hold)
};

// ---- All navigable screens in the application ----
enum class Screen
{
    Login,
    PatientMenu, PatientBookAppt, PatientCancel, PatientViewAppts,
    PatientRecords, PatientBills, PatientPay, PatientTopUp,
    DoctorMenu, DoctorTodayAppts, DoctorComplete, DoctorNoShow,
    DoctorPrescription, DoctorPatientHistory,
    AdminMenu, AdminAddDoctor, AdminRemoveDoctor, AdminViewPatients,
    AdminViewDoctors, AdminViewAppts, AdminUnpaidBills, AdminDischarge,
    AdminSecurityLog, AdminDailyReport
};

//  GUI: owns the SFML window and all screen-rendering logic.
//  It holds a non-owning pointer to HospitalSystem for data access.

class GUI
{
private:
    sf::RenderWindow window;
    sf::Font font;
    HospitalSystem* sys;           // non-owning; lifetime managed by main()

    Screen  currentScreen;
    int currentRole;            // 0 = none, 1 = Patient, 2 = Doctor, 3 = Admin
    int currentUserId;
    int loginAttempts;
    bool locked;                 // true after 3 consecutive failed logins
    int selectedRole;           // role tab selected on the login screen

    TextBox loginIdBox;
    TextBox loginPwdBox;

    char message[300];         // status message shown in the bottom bar
    sf::Color messageColor;
    float messageFade;          // countdown in seconds; message disappears at zero

    TextBox formBoxes[10];          // general-purpose input fields for each screen
    int formBoxCount;           // how many formBoxes are active for the current screen

    float listScrollY;            // vertical scroll offset for scrollable list panels
    int bookSelectedDoctorId;   // doctor chosen during appointment booking

    InputState inp;                 // mouse snapshot updated once at top of each frame
    bool prevMouseDown;       // previous frame's mouse state for edge detection

public:
    GUI(HospitalSystem* s);
    ~GUI();
    void run();

private:
    // Frame lifecycle
    void processEvents();   // OS event pump (keyboard, scroll, window close)
    void updateInput();     // builds the per-frame InputState snapshot
    void render();          // dispatches to the correct screen renderer

    // Chrome drawing
    void drawBackground();
    void drawTopBar(const char* pageTitle);
    void drawStatusBar();

    // Primitive drawing helpers
    void drawText(const char* s, float x, float y, unsigned int size, sf::Color c, bool bold = false);
    float textWidth(const char* s, unsigned int size);
    bool drawButton(Button& b);        // returns true if clicked this frame
    void drawTextBox(TextBox& tb);
    void textBoxInput(TextBox& tb, uint32_t unicode);  // handles backspace and printable chars

    // Builder helpers — configure structs before drawing
    void makeButton(Button& b, float x, float y, float w, float h, const char* lbl,
        sf::Color fill = sf::Color(56, 182, 255), sf::Color txtCol = sf::Color::White);

    void makeOutlineButton(Button& b, float x, float y, float w, float h, const char* lbl,
        sf::Color border = sf::Color(56, 182, 255));

    void makeTextBox(TextBox& tb, float x, float y, float w, float h, const char* lbl, const char* ph, bool pwd = false);

    // Layout helpers
    void drawCard(float x, float y, float w, float h, sf::Color col = Pal::card());
    void drawSeparator(float y, float x1 = 30.f, float x2 = 1070.f);
    void drawBadge(const char* txt, float x, float y, sf::Color c);
    void drawRow(const char* txt, float y, bool alt = false);

    void setMessage(const char* m, bool success);
    void resetForm();   // clears all formBoxes and resets scroll position
    void doLogin();     // validates credentials and transitions to the correct menu

    // ---- Screen renderers (one per Screen enum value) ----
    void renderLogin();
    void renderPatientMenu();
    void renderPatientBookAppt();
    void renderPatientCancel();
    void renderPatientViewAppts();
    void renderPatientRecords();
    void renderPatientBills();
    void renderPatientPay();
    void renderPatientTopUp();

    void renderDoctorMenu();
    void renderDoctorToday();
    void renderDoctorComplete();
    void renderDoctorNoShow();
    void renderDoctorPrescription();
    void renderDoctorPatientHistory();

    void renderAdminMenu();
    void renderAdminAddDoctor();
    void renderAdminRemoveDoctor();
    void renderAdminViewPatients();
    void renderAdminViewDoctors();
    void renderAdminViewAppts();
    void renderAdminUnpaidBills();
    void renderAdminDischarge();
    void renderAdminSecurityLog();
    void renderAdminDailyReport();
};

#endif