#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "GUI.h"
#include "FileHandler.h"
#include "Validator.h"
#include "Exceptions.h"
#include "MyString.h"
#include <algorithm>
#include <ctime>

 
//  Colour helpers
 

// Returns a brightened version of c by adding amt to each RGB channel (clamped to 255)
static sf::Color brighten(sf::Color c, int amt)
{
    return sf::Color(
        static_cast<uint8_t>(std::min(255, static_cast<int>(c.r) + amt)),
        static_cast<uint8_t>(std::min(255, static_cast<int>(c.g) + amt)),
        static_cast<uint8_t>(std::min(255, static_cast<int>(c.b) + amt)),
        c.a);
}

// Returns a copy of c with alpha replaced by a (clamped to 0-255)
static sf::Color withAlpha(sf::Color c, int a)
{
    c.a = static_cast<uint8_t>(std::max(0, std::min(255, a)));
    return c;
}

 
//  Constructor
 

GUI::GUI(HospitalSystem* s)
    : window(sf::VideoMode({ 1200u, 780u }),
        "MediCore — Hospital Management System"),
    sys(s),
    currentScreen(Screen::Login),
    currentRole(0), currentUserId(0),
    loginAttempts(0), locked(false),
    selectedRole(1), formBoxCount(0),
    listScrollY(0.f), bookSelectedDoctorId(0),
    prevMouseDown(false), messageFade(0.f)
{
    window.setFramerateLimit(60);

    // Try multiple font paths for cross-platform compatibility (Windows then Linux)
    const char* fontPaths[] = {
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "C:/Windows/Fonts/tahoma.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "arial.ttf", nullptr
    };
    for (int i = 0; fontPaths[i]; i++)
    {
        if (font.openFromFile(fontPaths[i]))
        {
            break;
        }
    }

    message[0] = '\0';
    messageColor = Pal::success();
    inp.mousePos = sf::Vector2f(0.f, 0.f);
    inp.mouseDown = false;
    inp.mouseClicked = false;

    // Pre-build the login form text boxes so they persist across frames
    makeTextBox(loginIdBox, 400.f, 320.f, 320.f, 48.f,
        "User ID", "Enter your numeric ID");
    makeTextBox(loginPwdBox, 400.f, 390.f, 320.f, 48.f,
        "Password", "Enter password", true);
}

GUI::~GUI()
{
}

 
//  Main loop — input snapshot taken ONCE per frame
 

void GUI::run()
{
    sf::Clock frameClock;
    while (window.isOpen())
    {
        float deltaTime = frameClock.restart().asSeconds();

        processEvents();    // handle OS events (text input, scroll, close)
        updateInput();      // build per-frame mouse snapshot

        // Count down and clear the status-bar message when it expires
        if (messageFade > 0.f)
        {
            messageFade -= deltaTime;
        }
        if (messageFade <= 0.f && message[0] != '\0')
        {
            message[0] = '\0';
        }

        render();
    }
}

// Drains the SFML event queue; handles window close, scroll, text entry, and click-to-focus
void GUI::processEvents()
{
    while (auto ev = window.pollEvent())
    {
        if (ev->is<sf::Event::Closed>())
        {
            window.close();
            return;
        }

        // Mouse wheel scrolls the active list panel
        if (const auto* scrollEvent = ev->getIf<sf::Event::MouseWheelScrolled>())
        {
            listScrollY -= scrollEvent->delta * 22.f;
            if (listScrollY < 0.f)
            {
                listScrollY = 0.f;
            }
        }

        // Route keyboard characters to whichever text box is currently active
        if (const auto* textEvent = ev->getIf<sf::Event::TextEntered>())
        {
            if (loginIdBox.active)
            {
                textBoxInput(loginIdBox, textEvent->unicode);
            }
            if (loginPwdBox.active)
            {
                textBoxInput(loginPwdBox, textEvent->unicode);
            }
            for (int i = 0; i < formBoxCount; i++)
            {
                if (formBoxes[i].active)
                {
                    textBoxInput(formBoxes[i], textEvent->unicode);
                }
            }
        }

        // Left-click sets focus to whichever text box contains the cursor
        if (const auto* mouseButtonEvent = ev->getIf<sf::Event::MouseButtonPressed>())
        {
            if (mouseButtonEvent->button == sf::Mouse::Button::Left)
            {
                sf::Vector2f clickPos(static_cast<float>(mouseButtonEvent->position.x),
                    static_cast<float>(mouseButtonEvent->position.y));
                loginIdBox.active = loginIdBox.rect.getGlobalBounds().contains(clickPos);
                loginPwdBox.active = loginPwdBox.rect.getGlobalBounds().contains(clickPos);
                for (int i = 0; i < formBoxCount; i++)
                {
                    formBoxes[i].active = formBoxes[i].rect.getGlobalBounds().contains(clickPos);
                }
            }
        }
    }
}

// Captures mouse position and button state once; computes a rising-edge click flag
void GUI::updateInput()
{
    bool currentlyDown = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    inp.mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
    inp.mouseDown = currentlyDown;
    inp.mouseClicked = currentlyDown && !prevMouseDown;   // true only on first frame of press
    prevMouseDown = currentlyDown;
}

 
//  Render dispatcher
 

void GUI::render()
{
    window.clear(Pal::bg());
    drawBackground();   // left accent strip

    switch (currentScreen)
    {
    case Screen::Login:                renderLogin();               break;
    case Screen::PatientMenu:          renderPatientMenu();         break;
    case Screen::PatientBookAppt:      renderPatientBookAppt();     break;
    case Screen::PatientCancel:        renderPatientCancel();       break;
    case Screen::PatientViewAppts:     renderPatientViewAppts();    break;
    case Screen::PatientRecords:       renderPatientRecords();      break;
    case Screen::PatientBills:         renderPatientBills();        break;
    case Screen::PatientPay:           renderPatientPay();          break;
    case Screen::PatientTopUp:         renderPatientTopUp();        break;
    case Screen::DoctorMenu:           renderDoctorMenu();          break;
    case Screen::DoctorTodayAppts:     renderDoctorToday();         break;
    case Screen::DoctorComplete:       renderDoctorComplete();      break;
    case Screen::DoctorNoShow:         renderDoctorNoShow();        break;
    case Screen::DoctorPrescription:   renderDoctorPrescription();  break;
    case Screen::DoctorPatientHistory: renderDoctorPatientHistory(); break;
    case Screen::AdminMenu:            renderAdminMenu();           break;
    case Screen::AdminAddDoctor:       renderAdminAddDoctor();      break;
    case Screen::AdminRemoveDoctor:    renderAdminRemoveDoctor();   break;
    case Screen::AdminViewPatients:    renderAdminViewPatients();   break;
    case Screen::AdminViewDoctors:     renderAdminViewDoctors();    break;
    case Screen::AdminViewAppts:       renderAdminViewAppts();      break;
    case Screen::AdminUnpaidBills:     renderAdminUnpaidBills();    break;
    case Screen::AdminDischarge:       renderAdminDischarge();      break;
    case Screen::AdminSecurityLog:     renderAdminSecurityLog();    break;
    case Screen::AdminDailyReport:     renderAdminDailyReport();    break;
    }

    drawStatusBar();    // overlaid message bar at the bottom
    window.display();
}

 
//  Layout chrome
 

// Draws the thin vertical accent strip on the left edge
void GUI::drawBackground()
{
    sf::RectangleShape accentStrip(sf::Vector2f(4.f, 780.f));
    accentStrip.setFillColor(Pal::accent());
    window.draw(accentStrip);
}

// Draws the top bar with the brand name, a vertical separator, and the page title
void GUI::drawTopBar(const char* pageTitle)
{
    sf::RectangleShape topBar(sf::Vector2f(1200.f, 64.f));
    topBar.setFillColor(Pal::hdr());
    window.draw(topBar);

    drawText("MEDICORE", 22.f, 16.f, 22, Pal::accent(), true);
    drawText("Hospital Management System", 22.f, 40.f, 11, Pal::textSec());

    // Thin vertical rule between brand and page title
    sf::RectangleShape verticalRule(sf::Vector2f(1.f, 40.f));
    verticalRule.setPosition(sf::Vector2f(220.f, 12.f));
    verticalRule.setFillColor(Pal::border());
    window.draw(verticalRule);

    drawText(pageTitle, 236.f, 20.f, 20, Pal::textPri(), true);

    // Horizontal bottom border of the top bar
    sf::RectangleShape bottomBorder(sf::Vector2f(1200.f, 1.f));
    bottomBorder.setPosition(sf::Vector2f(0.f, 64.f));
    bottomBorder.setFillColor(Pal::border());
    window.draw(bottomBorder);
}

// Draws the fading status bar at the bottom with the last success/error message
void GUI::drawStatusBar()
{
    if (message[0] == '\0')
    {
        return;
    }

    // Fade alpha decreases over the last 0.5 seconds of the 4-second display window
    float alpha = std::min(1.f, messageFade / 0.5f);
    int   alphaInt = static_cast<int>(alpha * 240.f);

    bool isSuccessMessage = (messageColor.r == Pal::success().r &&
        messageColor.g == Pal::success().g &&
        messageColor.b == Pal::success().b);

    sf::RectangleShape statusBackground(sf::Vector2f(1200.f, 36.f));
    statusBackground.setPosition(sf::Vector2f(0.f, 744.f));
    statusBackground.setFillColor(withAlpha(
        isSuccessMessage ? sf::Color(20, 60, 35) : sf::Color(60, 20, 20), alphaInt));
    window.draw(statusBackground);

    // Small coloured dot acting as a success/error indicator
    sf::CircleShape indicatorDot(5.f);
    indicatorDot.setPosition(sf::Vector2f(18.f, 757.f));
    indicatorDot.setFillColor(withAlpha(messageColor, alphaInt));
    window.draw(indicatorDot);

    sf::Text messageText(font);
    messageText.setString(message);
    messageText.setCharacterSize(14);
    messageText.setFillColor(withAlpha(messageColor, alphaInt));
    messageText.setPosition(sf::Vector2f(34.f, 752.f));
    window.draw(messageText);
}

 
//  Primitive helpers
 

// Constructs an sf::Text on the fly and draws it; bold parameter is accepted but
// currently unused (SFML 3 requires a separate bold-font asset)
void GUI::drawText(const char* s, float x, float y,
    unsigned int size, sf::Color c, bool /*bold*/)
{
    sf::Text textObj(font);
    textObj.setString(s);
    textObj.setCharacterSize(size);
    textObj.setFillColor(c);
    textObj.setPosition(sf::Vector2f(x, y));
    window.draw(textObj);
}

// Returns the rendered pixel width of a string at the given character size
float GUI::textWidth(const char* s, unsigned int size)
{
    sf::Text textObj(font);
    textObj.setString(s);
    textObj.setCharacterSize(size);
    return textObj.getLocalBounds().size.x;
}

// Draws a button and returns true if it was clicked this frame (hover + rising edge)
bool GUI::drawButton(Button& b)
{
    bool isHovered = b.rect.getGlobalBounds().contains(inp.mousePos);

    sf::RectangleShape buttonShape = b.rect;
    sf::Color          fillColor = b.fillColor;

    if (isHovered)
    {
        fillColor = brighten(fillColor, 30);   // lighten on hover
    }

    if (b.outline)
    {
        // Outline style: transparent fill, coloured border; semi-transparent fill on hover
        buttonShape.setFillColor(isHovered
            ? withAlpha(b.fillColor, 40)
            : sf::Color::Transparent);
        buttonShape.setOutlineThickness(1.5f);
        buttonShape.setOutlineColor(b.fillColor);
    }
    else
    {
        buttonShape.setFillColor(fillColor);
    }
    window.draw(buttonShape);

    // Centre the label text within the button rectangle
    if (b.font)
    {
        sf::Text labelText(*b.font);
        labelText.setString(b.labelStr);
        labelText.setCharacterSize(15);
        labelText.setFillColor(b.outline ? b.fillColor : b.textColor);
        sf::FloatRect textBounds = labelText.getLocalBounds();
        sf::FloatRect buttonBounds = buttonShape.getGlobalBounds();
        labelText.setPosition(sf::Vector2f(
            buttonBounds.position.x + (buttonBounds.size.x - textBounds.size.x) / 2.f - textBounds.position.x,
            buttonBounds.position.y + (buttonBounds.size.y - textBounds.size.y) / 2.f - textBounds.position.y));
        window.draw(labelText);
    }

    return isHovered && inp.mouseClicked;
}

// Draws a text input box with its label, placeholder, active border highlight, and blinking cursor
void GUI::drawTextBox(TextBox& tb)
{
    bool isFocused = tb.active;

    // Label drawn above the box
    if (tb.label[0] != '\0')
    {
        drawText(tb.label,
            tb.rect.getPosition().x,
            tb.rect.getPosition().y - 20.f,
            12, Pal::textSec());
    }

    sf::RectangleShape inputRect = tb.rect;
    inputRect.setFillColor(Pal::inputBg());
    inputRect.setOutlineThickness(isFocused ? 1.5f : 1.f);
    inputRect.setOutlineColor(isFocused ? Pal::accent() : Pal::border());
    window.draw(inputRect);

    float textX = tb.rect.getPosition().x + 10.f;
    float textY = tb.rect.getPosition().y + 13.f;

    if (tb.len == 0)
    {
        // Show placeholder when the box is empty
        if (tb.font)
        {
            sf::Text placeholderText(*tb.font);
            placeholderText.setString(tb.placeholderStr);
            placeholderText.setCharacterSize(15);
            placeholderText.setFillColor(withAlpha(Pal::textSec(), 140));
            placeholderText.setPosition(sf::Vector2f(textX, textY));
            window.draw(placeholderText);
        }
    }
    else if (tb.font)
    {
        // Show asterisks for password fields; plain text otherwise
        sf::Text contentText(*tb.font);
        if (tb.isPassword)
        {
            char maskedContent[257];
            for (int i = 0; i < tb.len; i++)
            {
                maskedContent[i] = '*';
            }
            maskedContent[tb.len] = '\0';
            contentText.setString(maskedContent);
        }
        else
        {
            contentText.setString(tb.content);
        }
        contentText.setCharacterSize(15);
        contentText.setFillColor(Pal::textPri());
        contentText.setPosition(sf::Vector2f(textX, textY));
        window.draw(contentText);
    }

    // Blinking cursor drawn only when the box is focused
    if (isFocused && tb.font)
    {
        static sf::Clock blinkClock;
        float blinkTime = blinkClock.getElapsedTime().asSeconds();
        if (static_cast<int>(blinkTime * 2) % 2 == 0)
        {
            float cursorX = textX;
            if (tb.len > 0)
            {
                // Position cursor after the last character
                sf::Text dummyText(*tb.font);
                char     displayedContent[257];
                if (tb.isPassword)
                {
                    for (int i = 0; i < tb.len; i++)
                    {
                        displayedContent[i] = '*';
                    }
                    displayedContent[tb.len] = '\0';
                }
                else
                {
                    MyStr::copyN(displayedContent, tb.content, 257);
                }
                dummyText.setString(displayedContent);
                dummyText.setCharacterSize(15);
                cursorX += dummyText.getLocalBounds().size.x + 2.f;
            }
            sf::RectangleShape cursor(sf::Vector2f(1.5f, 20.f));
            cursor.setPosition(sf::Vector2f(cursorX, textY + 1.f));
            cursor.setFillColor(Pal::accent());
            window.draw(cursor);
        }
    }
}

// Handles one Unicode character: backspace (8) deletes the last char; printable chars are appended
void GUI::textBoxInput(TextBox& tb, uint32_t unicode)
{
    if (unicode == 8)
    {
        if (tb.len > 0)
        {
            tb.len--;
            tb.content[tb.len] = '\0';
        }
    }
    else if (unicode >= 32 && unicode < 127)
    {
        if (tb.len < 255)
        {
            tb.content[tb.len++] = static_cast<char>(unicode);
            tb.content[tb.len] = '\0';
        }
    }
}

 
//  Builder helpers
 

// Configures a filled button; call before drawButton() each frame
void GUI::makeButton(Button& b, float x, float y, float w, float h,
    const char* lbl, sf::Color fill, sf::Color txtCol)
{
    b.rect.setPosition(sf::Vector2f(x, y));
    b.rect.setSize(sf::Vector2f(w, h));
    b.fillColor = fill;
    b.textColor = txtCol;
    b.outline = false;
    b.font = &font;
    MyStr::copyN(b.labelStr, lbl, 80);
}

// Configures a transparent-fill outline button
void GUI::makeOutlineButton(Button& b, float x, float y, float w, float h,
    const char* lbl, sf::Color borderColor)
{
    makeButton(b, x, y, w, h, lbl, borderColor, borderColor);
    b.outline = true;
}

// Configures a text input box; call once when formBoxCount == 0 to set up the screen's form
void GUI::makeTextBox(TextBox& tb, float x, float y, float w, float h,
    const char* lbl, const char* ph, bool pwd)
{
    tb.rect.setPosition(sf::Vector2f(x, y));
    tb.rect.setSize(sf::Vector2f(w, h));
    MyStr::copyN(tb.label, lbl, 60);
    MyStr::copyN(tb.placeholderStr, ph, 80);
    tb.isPassword = pwd;
    tb.font = &font;
    tb.len = 0;
    tb.content[0] = '\0';
    tb.active = false;
}

// ---- Layout helpers ----

void GUI::drawCard(float x, float y, float w, float h, sf::Color col)
{
    sf::RectangleShape cardShape(sf::Vector2f(w, h));
    cardShape.setPosition(sf::Vector2f(x, y));
    cardShape.setFillColor(col);
    cardShape.setOutlineThickness(1.f);
    cardShape.setOutlineColor(Pal::border());
    window.draw(cardShape);
}

void GUI::drawSeparator(float y, float x1, float x2)
{
    sf::RectangleShape separatorLine(sf::Vector2f(x2 - x1, 1.f));
    separatorLine.setPosition(sf::Vector2f(x1, y));
    separatorLine.setFillColor(Pal::border());
    window.draw(separatorLine);
}

// Draws a small pill-shaped badge with a semi-transparent background
void GUI::drawBadge(const char* txt, float x, float y, sf::Color c)
{
    float             badgeWidth = textWidth(txt, 12) + 14.f;
    sf::RectangleShape badgeBg(sf::Vector2f(badgeWidth, 20.f));
    badgeBg.setPosition(sf::Vector2f(x, y));
    badgeBg.setFillColor(withAlpha(c, 40));
    badgeBg.setOutlineThickness(1.f);
    badgeBg.setOutlineColor(withAlpha(c, 160));
    window.draw(badgeBg);
    drawText(txt, x + 7.f, y + 3.f, 12, c);
}

// Draws one alternating-row background stripe and its text label
void GUI::drawRow(const char* txt, float y, bool alt)
{
    if (alt)
    {
        sf::RectangleShape altRowBackground(sf::Vector2f(1140.f, 28.f));
        altRowBackground.setPosition(sf::Vector2f(30.f, y - 4.f));
        altRowBackground.setFillColor(withAlpha(Pal::panel(), 120));
        window.draw(altRowBackground);
    }
    drawText(txt, 38.f, y, 14, Pal::textPri());
}

// Sets the status bar message and resets the fade timer
void GUI::setMessage(const char* m, bool success)
{
    MyStr::copyN(message, m, 300);
    messageColor = success ? Pal::success() : Pal::danger();
    messageFade = 4.f;   // display for 4 seconds
}

// Clears all general-purpose form boxes and resets the scroll position
void GUI::resetForm()
{
    formBoxCount = 0;
    listScrollY = 0.f;
    for (int i = 0; i < 10; i++)
    {
        formBoxes[i].len = 0;
        formBoxes[i].content[0] = '\0';
        formBoxes[i].active = false;
    }
}

 
//  LOGIN SCREEN
 

void GUI::renderLogin()
{
    drawCard(300.f, 100.f, 600.f, 560.f, Pal::panel());

    // Dark header band inside the login card
    sf::RectangleShape loginHeaderBg(sf::Vector2f(600.f, 80.f));
    loginHeaderBg.setPosition(sf::Vector2f(300.f, 100.f));
    loginHeaderBg.setFillColor(Pal::hdr());
    window.draw(loginHeaderBg);

    drawText("MEDICORE", 380.f, 114.f, 28, Pal::accent(), true);
    drawText("Hospital Management System", 380.f, 148.f, 13, Pal::textSec());

    // Decorative circle in the header acting as a lock icon placeholder
    sf::CircleShape decorativeIcon(18.f);
    decorativeIcon.setPosition(sf::Vector2f(556.f, 108.f));
    decorativeIcon.setFillColor(withAlpha(Pal::accent(), 30));
    decorativeIcon.setOutlineThickness(1.5f);
    decorativeIcon.setOutlineColor(Pal::accent());
    window.draw(decorativeIcon);

    // Show lockout notice and stop rendering the rest of the login form
    if (locked)
    {
        drawCard(320.f, 200.f, 560.f, 60.f, withAlpha(Pal::danger(), 30));
        drawText("Account locked after 3 failed attempts.", 360.f, 222.f, 15, Pal::danger());
        return;
    }

    // Role selector tab row
    drawText("Login As", 380.f, 205.f, 13, Pal::textSec());
    const char* roleNames[] = { "Patient", "Doctor", "Admin" };
    float       tabXPositions[] = { 315.f, 465.f, 615.f };
    for (int i = 0; i < 3; i++)
    {
        Button roleTab;
        bool   isSelected = selectedRole == i + 1;
        makeButton(roleTab, tabXPositions[i], 224.f, 135.f, 36.f, roleNames[i],
            isSelected ? Pal::accent() : Pal::card(),
            isSelected ? sf::Color::White : Pal::textSec());
        if (drawButton(roleTab))
        {
            selectedRole = i + 1;
        }
    }

    drawTextBox(loginIdBox);
    drawTextBox(loginPwdBox);

    Button signInButton;
    makeButton(signInButton, 400.f, 462.f, 320.f, 46.f, "Sign In", Pal::accent());
    if (drawButton(signInButton))
    {
        doLogin();
    }

    // Show failed-attempt counter below the sign-in button
    if (loginAttempts > 0)
    {
        char attemptsLabel[60] = "Failed attempts: ";
        char attemptsNum[4];
        MyStr::intToStr(loginAttempts, attemptsNum);
        MyStr::concat(attemptsLabel, attemptsNum);
        MyStr::concat(attemptsLabel, " / 3");
        drawText(attemptsLabel, 430.f, 520.f, 12, Pal::warning());
    }

    drawText("MediCore v2.0  |  Secure Login",
        380.f, 620.f, 11, withAlpha(Pal::textSec(), 100));
}

// Validates the entered credentials, logs the attempt, and transitions to the correct menu
void GUI::doLogin()
{
    if (loginIdBox.len == 0 || loginPwdBox.len == 0)
    {
        setMessage("Please enter ID and password.", false);
        return;
    }
    if (!Validator::isValidId(loginIdBox.content))
    {
        setMessage("Invalid ID format — numeric only.", false);
        return;
    }

    int         enteredId = MyStr::toInt(loginIdBox.content);
    bool        loginOk = false;
    const char* roleName = "Patient";

    if (selectedRole == 1)
    {
        Patient* foundPatient = sys->loginPatient(enteredId, loginPwdBox.content);
        if (foundPatient)
        {
            loginOk = true;
            currentUserId = foundPatient->getId();
            currentScreen = Screen::PatientMenu;
        }
    }
    else if (selectedRole == 2)
    {
        roleName = "Doctor";
        Doctor* foundDoctor = sys->loginDoctor(enteredId, loginPwdBox.content);
        if (foundDoctor)
        {
            loginOk = true;
            currentUserId = foundDoctor->getId();
            currentScreen = Screen::DoctorMenu;
        }
    }
    else
    {
        roleName = "Admin";
        Admin* foundAdmin = sys->loginAdmin(enteredId, loginPwdBox.content);
        if (foundAdmin)
        {
            loginOk = true;
            currentUserId = foundAdmin->getId();
            currentScreen = Screen::AdminMenu;
        }
    }

    // Every login attempt is recorded in security_log.txt regardless of outcome
    FileHandler::logSecurity(roleName, loginIdBox.content, loginOk ? "SUCCESS" : "FAILED");

    if (loginOk)
    {
        currentRole = selectedRole;
        loginAttempts = 0;
        loginIdBox.len = 0; loginIdBox.content[0] = '\0';
        loginPwdBox.len = 0; loginPwdBox.content[0] = '\0';
        setMessage("Login successful. Welcome!", true);
        resetForm();
    }
    else
    {
        loginAttempts++;
        if (loginAttempts >= 3)
        {
            locked = true;
            setMessage("Account locked.", false);
        }
        else
        {
            setMessage("Invalid credentials.", false);
        }
    }
}

 
//  PATIENT MENU
 

void GUI::renderPatientMenu()
{
    Patient* currentPatient = sys->getPatients()->findById(currentUserId);
    if (!currentPatient)
    {
        currentScreen = Screen::Login;
        return;
    }

    drawTopBar("Patient Dashboard");

    // Profile summary card (top-left)
    drawCard(20.f, 76.f, 340.f, 120.f);
    drawText("PATIENT", 36.f, 86.f, 11, Pal::textSec());
    drawText(currentPatient->getName(), 36.f, 102.f, 20, Pal::textPri(), true);
    char idLabel[40] = "ID: ";
    char idNum[10] = {};
    MyStr::intToStr(currentPatient->getId(), idNum);
    MyStr::concat(idLabel, idNum);
    drawText(idLabel, 36.f, 130.f, 13, Pal::textSec());
    char balanceLabel[40] = "Balance  PKR ";
    char balanceNum[20];
    MyStr::floatToStr(currentPatient->getBalance(), balanceNum, 2);
    MyStr::concat(balanceLabel, balanceNum);
    drawText(balanceLabel, 36.f, 152.f, 14, Pal::success());

    // Action grid: 8 cards in a 2-column layout
    const char* actionLabels[] = {
        "Book Appointment", "Cancel Appointment",
        "View Appointments", "Medical Records",
        "View Bills",        "Pay Bill",
        "Top Up Balance",    "Logout"
    };
    Screen actionTargets[] = {
        Screen::PatientBookAppt,  Screen::PatientCancel,
        Screen::PatientViewAppts, Screen::PatientRecords,
        Screen::PatientBills,     Screen::PatientPay,
        Screen::PatientTopUp,     Screen::Login
    };
    sf::Color actionColors[] = {
        Pal::accent(),   Pal::warning(),
        Pal::accentDk(), Pal::accentDk(),
        Pal::accentDk(), Pal::success(),
        Pal::success(),  Pal::danger()
    };

    for (int i = 0; i < 8; i++)
    {
        float cardX = 20.f + static_cast<float>(i % 2) * 380.f;
        float cardY = 210.f + static_cast<float>(i / 2) * 80.f;
        drawCard(cardX, cardY, 360.f, 64.f);

        // Colour stripe on the left edge of each action card
        sf::RectangleShape leftStripe(sf::Vector2f(4.f, 64.f));
        leftStripe.setPosition(sf::Vector2f(cardX, cardY));
        leftStripe.setFillColor(actionColors[i]);
        window.draw(leftStripe);

        drawText(actionLabels[i], cardX + 18.f, cardY + 22.f, 16, Pal::textPri());

        Button openButton;
        makeButton(openButton, cardX + 290.f, cardY + 16.f, 60.f, 30.f, "Open",
            withAlpha(actionColors[i], 180));
        if (drawButton(openButton))
        {
            resetForm();
            currentScreen = actionTargets[i];
            if (i == 7)   // Logout clears the session
            {
                currentRole = 0;
                currentUserId = 0;
            }
        }
    }
}

 
//  PATIENT – BOOK APPOINTMENT
 

void GUI::renderPatientBookAppt()
{
    drawTopBar("Book Appointment");

    // Left panel: search form
    drawCard(20.f, 76.f, 420.f, 640.f);
    drawText("Search & Book", 36.f, 90.f, 16, Pal::textPri(), true);
    drawSeparator(114.f, 30.f, 430.f);

    // Build form boxes once per screen visit
    if (formBoxCount == 0)
    {
        makeTextBox(formBoxes[0], 36.f, 148.f, 388.f, 44.f, "Specialization", "e.g. Cardiology");
        makeTextBox(formBoxes[1], 36.f, 234.f, 388.f, 44.f, "Doctor ID", "Enter doctor numeric ID");
        makeTextBox(formBoxes[2], 36.f, 320.f, 388.f, 44.f, "Date", "DD-MM-YYYY");
        makeTextBox(formBoxes[3], 36.f, 406.f, 388.f, 44.f, "Time Slot", "e.g. 09:00");
        formBoxCount = 4;
    }
    for (int i = 0; i < formBoxCount; i++)
    {
        drawTextBox(formBoxes[i]);
    }

    Button searchButton, bookButton, backButton;
    makeButton(searchButton, 36.f, 470.f, 185.f, 42.f, "Search Doctors", Pal::accentDk());
    makeButton(bookButton, 36.f, 524.f, 388.f, 44.f, "Confirm Booking", Pal::success());
    makeOutlineButton(backButton, 36.f, 580.f, 185.f, 38.f, "Back", Pal::danger());

    // Right panel: search results and slot grid
    drawCard(454.f, 76.f, 726.f, 640.f);
    drawText("Available Doctors", 470.f, 90.f, 16, Pal::textPri(), true);
    drawSeparator(114.f, 464.f, 1170.f);

    // matchedIds persists between frames so the results stay visible
    static int matchedDoctorIds[20] = {};
    static int matchedDoctorCount = 0;

    if (drawButton(searchButton))
    {
        matchedDoctorCount = 0;
        Storage<Doctor>* allDoctors = sys->getDoctors();
        for (int i = 0; i < allDoctors->size() && matchedDoctorCount < 20; i++)
        {
            if (MyStr::equalsIgnoreCase((*allDoctors)[i].getSpecialization(), formBoxes[0].content))
            {
                matchedDoctorIds[matchedDoctorCount++] = (*allDoctors)[i].getId();
            }
        }
        setMessage(matchedDoctorCount == 0
            ? "No doctors found for that specialization."
            : "Doctors loaded. Select a doctor ID below.",
            matchedDoctorCount > 0);
    }

    if (drawButton(backButton))
    {
        currentScreen = Screen::PatientMenu;
        resetForm();
        return;
    }

    // Render a card for each matched doctor
    for (int i = 0; i < matchedDoctorCount; i++)
    {
        Doctor* matchedDoctor = sys->getDoctors()->findById(matchedDoctorIds[i]);
        if (!matchedDoctor)
        {
            continue;
        }
        float doctorCardY = 126.f + static_cast<float>(i) * 60.f;
        drawCard(464.f, doctorCardY, 706.f, 50.f, Pal::card());

        char doctorIdStr[10], feeStr[20];
        MyStr::intToStr(matchedDoctor->getId(), doctorIdStr);
        MyStr::floatToStr(matchedDoctor->getFee(), feeStr, 2);
        char doctorInfoLine[200];
        MyStr::copy(doctorInfoLine, "ID:");
        MyStr::concat(doctorInfoLine, doctorIdStr);
        MyStr::concat(doctorInfoLine, "  ");
        MyStr::concat(doctorInfoLine, matchedDoctor->getName());
        MyStr::concat(doctorInfoLine, "  |  ");
        MyStr::concat(doctorInfoLine, matchedDoctor->getSpecialization());
        MyStr::concat(doctorInfoLine, "  |  PKR ");
        MyStr::concat(doctorInfoLine, feeStr);
        drawText(doctorInfoLine, 476.f, doctorCardY + 16.f, 14, Pal::textPri());
    }

    // Slot availability grid: shown only when a doctor ID and valid date are entered
    if (formBoxes[1].len > 0 && Validator::isValidDate(formBoxes[2].content))
    {
        int selectedDoctorId = MyStr::toInt(formBoxes[1].content);
        const char* timeSlots[] = { "09:00","10:00","11:00","12:00", "13:00","14:00","15:00","16:00" };
        drawText("Time Slots", 470.f, 490.f, 14, Pal::textSec());
        Storage<Appointment>* allAppointments = sys->getAppointments();

        for (int i = 0; i < 8; i++)
        {
            // Check if this slot is already taken by any non-cancelled appointment
            bool slotTaken = false;
            for (int j = 0; j < allAppointments->size(); j++)
            {
                Appointment& appt = (*allAppointments)[j];
                if (appt.getDoctorId() == selectedDoctorId
                    && MyStr::equals(appt.getDate(), formBoxes[2].content)
                    && MyStr::equals(appt.getTimeSlot(), timeSlots[i])
                    && !MyStr::equals(appt.getStatus(), "cancelled"))
                {
                    slotTaken = true;
                    break;
                }
            }
            float slotX = 464.f + static_cast<float>(i % 4) * 180.f;
            float slotY = 510.f + static_cast<float>(i / 4) * 40.f;
            drawCard(slotX, slotY, 168.f, 30.f,
                slotTaken ? withAlpha(Pal::danger(), 40) : withAlpha(Pal::success(), 40));
            sf::RectangleShape slotBorder(sf::Vector2f(168.f, 30.f));
            slotBorder.setPosition(sf::Vector2f(slotX, slotY));
            slotBorder.setOutlineThickness(1.f);
            slotBorder.setOutlineColor(slotTaken ? withAlpha(Pal::danger(), 120)
                : withAlpha(Pal::success(), 120));
            slotBorder.setFillColor(sf::Color::Transparent);
            window.draw(slotBorder);
            drawText(timeSlots[i], slotX + 60.f, slotY + 7.f, 13,
                slotTaken ? Pal::danger() : Pal::success());
        }
    }

    if (drawButton(bookButton))
    {
        try
        {
            if (!Validator::isValidId(formBoxes[1].content))
            {
                throw InvalidInputException("Invalid Doctor ID.");
            }
            int     selectedDoctorId = MyStr::toInt(formBoxes[1].content);
            Doctor* selectedDoctor = sys->getDoctors()->findById(selectedDoctorId);
            if (!selectedDoctor)
            {
                throw InvalidInputException("Doctor not found.");
            }
            if (!Validator::isValidDate(formBoxes[2].content))
            {
                throw InvalidInputException("Invalid date (DD-MM-YYYY, future only).");
            }
            if (!Validator::isValidTimeSlot(formBoxes[3].content))
            {
                throw InvalidInputException("Invalid time slot (09:00-16:00).");
            }

            // Use operator== on a temporary appointment to detect slot conflicts
            Appointment tempAppt(0, currentUserId, selectedDoctorId,
                formBoxes[2].content, formBoxes[3].content, "pending");
            Storage<Appointment>* allAppointments = sys->getAppointments();
            for (int j = 0; j < allAppointments->size(); j++)
            {
                if ((*allAppointments)[j] == tempAppt)
                {
                    throw SlotUnavailableException("That slot is already booked.");
                }
            }

            Patient* bookingPatient = sys->getPatients()->findById(currentUserId);
            if (bookingPatient->getBalance() < selectedDoctor->getFee())
            {
                throw InsufficientFundsException("Insufficient balance. Please top up.");
            }

            // Deduct fee from wallet, persist the new appointment, and create its bill
            *bookingPatient -= selectedDoctor->getFee();
            int         newApptId = sys->nextAppointmentId();
            Appointment newAppt(newApptId, currentUserId, selectedDoctorId,
                formBoxes[2].content, formBoxes[3].content, "pending");
            sys->getAppointments()->add(newAppt);
            FileHandler::appendAppointment(&newAppt);

            int  newBillId = sys->nextBillId();
            Bill newBill(newBillId, currentUserId, newApptId,
                selectedDoctor->getFee(), "unpaid", formBoxes[2].content);
            sys->getBills()->add(newBill);
            FileHandler::appendBill(&newBill);
            FileHandler::savePatients(sys->getPatients());

            char successMsg[100] = "Appointment booked! ID: ";
            char apptIdStr[10];
            MyStr::intToStr(newApptId, apptIdStr);
            MyStr::concat(successMsg, apptIdStr);
            setMessage(successMsg, true);
        }
        catch (HospitalException& ex)
        {
            setMessage(ex.what(), false);
        }
    }
}

 
//  PATIENT – CANCEL APPOINTMENT
 

void GUI::renderPatientCancel()
{
    drawTopBar("Cancel Appointment");
    drawCard(20.f, 76.f, 760.f, 640.f);
    drawText("Pending Appointments", 36.f, 90.f, 16, Pal::textPri(), true);
    drawSeparator(114.f, 30.f, 770.f);

    if (formBoxCount == 0)
    {
        makeTextBox(formBoxes[0], 800.f, 140.f, 360.f, 44.f,
            "Appointment ID", "Enter appointment ID");
        formBoxCount = 1;
    }

    // List all pending appointments belonging to this patient
    Storage<Appointment>* allAppointments = sys->getAppointments();
    float listY = 124.f;
    int   rowCount = 0;
    for (int i = 0; i < allAppointments->size(); i++)
    {
        Appointment& appt = (*allAppointments)[i];
        if (appt.getPatientId() != currentUserId
            || !MyStr::equals(appt.getStatus(), "pending"))
        {
            continue;
        }
        Doctor* apptDoctor = sys->getDoctors()->findById(appt.getDoctorId());
        drawCard(30.f, listY, 740.f, 42.f, rowCount % 2 == 0 ? Pal::card() : Pal::panel());
        char rowBuf[300], apptIdStr[10];
        MyStr::intToStr(appt.getId(), apptIdStr);
        MyStr::copy(rowBuf, "#");
        MyStr::concat(rowBuf, apptIdStr);
        MyStr::concat(rowBuf, "  Dr.");
        MyStr::concat(rowBuf, apptDoctor ? apptDoctor->getName() : "?");
        MyStr::concat(rowBuf, "  |  ");
        MyStr::concat(rowBuf, appt.getDate());
        MyStr::concat(rowBuf, "  ");
        MyStr::concat(rowBuf, appt.getTimeSlot());
        drawText(rowBuf, 46.f, listY + 12.f, 14, Pal::textPri());
        drawBadge("PENDING", 680.f, listY + 11.f, Pal::warning());
        listY += 46.f;
        rowCount++;
    }
    if (rowCount == 0)
    {
        drawText("No pending appointments found.", 46.f, 150.f, 15, Pal::textSec());
    }

    // Right panel: cancellation form
    drawCard(790.f, 76.f, 390.f, 640.f);
    drawText("Cancel Selected", 806.f, 90.f, 16, Pal::textPri(), true);
    drawSeparator(114.f, 800.f, 1170.f);

    drawTextBox(formBoxes[0]);
    Button cancelButton, backButton;
    makeButton(cancelButton, 800.f, 210.f, 360.f, 44.f, "Cancel Appointment", Pal::danger());
    makeOutlineButton(backButton, 800.f, 266.f, 170.f, 38.f, "Back", Pal::textSec());

    if (drawButton(backButton))
    {
        currentScreen = Screen::PatientMenu;
        resetForm();
        return;
    }

    if (drawButton(cancelButton))
    {
        if (!Validator::isValidId(formBoxes[0].content))
        {
            setMessage("Invalid appointment ID.", false);
            return;
        }
        int          enteredApptId = MyStr::toInt(formBoxes[0].content);
        Appointment* targetAppt = sys->getAppointments()->findById(enteredApptId);
        if (!targetAppt || targetAppt->getPatientId() != currentUserId
            || !MyStr::equals(targetAppt->getStatus(), "pending"))
        {
            setMessage("Invalid appointment ID.", false);
            return;
        }

        // Cancel the appointment and refund the fee to the patient's wallet
        targetAppt->setStatus("cancelled");
        Doctor* apptDoctor = sys->getDoctors()->findById(targetAppt->getDoctorId());
        Patient* apptPatient = sys->getPatients()->findById(currentUserId);
        if (apptDoctor && apptPatient)
        {
            *apptPatient += apptDoctor->getFee();
        }

        // Also cancel the associated bill so it no longer appears as unpaid
        Storage<Bill>* allBills = sys->getBills();
        for (int i = 0; i < allBills->size(); i++)
        {
            if ((*allBills)[i].getAppointmentId() == enteredApptId)
            {
                (*allBills)[i].setStatus("cancelled");
            }
        }
        FileHandler::saveAppointments(sys->getAppointments());
        FileHandler::saveBills(sys->getBills());
        FileHandler::savePatients(sys->getPatients());

        char refundMsg[100] = "Cancelled. Refunded PKR ";
        char refundAmtStr[20] = {};
        MyStr::floatToStr(apptDoctor ? apptDoctor->getFee() : 0.f, refundAmtStr, 2);
        MyStr::concat(refundMsg, refundAmtStr);
        setMessage(refundMsg, true);
    }
}

 
//  PATIENT – VIEW APPOINTMENTS
 

void GUI::renderPatientViewAppts()
{
    drawTopBar("My Appointments");
    drawCard(20.f, 76.f, 1160.f, 640.f);

    // Collect indices of this patient's appointments
    Storage<Appointment>* allAppointments = sys->getAppointments();
    int sortedIndices[100], apptCount = 0;
    for (int i = 0; i < allAppointments->size(); i++)
    {
        if ((*allAppointments)[i].getPatientId() == currentUserId)
        {
            sortedIndices[apptCount++] = i;
        }
    }

    // Bubble sort ascending by date (YYYYMMDD numeric key derived from DD-MM-YYYY)
    for (int i = 0; i < apptCount - 1; i++)
    {
        for (int j = 0; j < apptCount - i - 1; j++)
        {
            const char* dateA = (*allAppointments)[sortedIndices[j]].getDate();
            const char* dateB = (*allAppointments)[sortedIndices[j + 1]].getDate();
            long keyA = static_cast<long>((dateA[6] - '0') * 1000 + (dateA[7] - '0') * 100 + (dateA[8] - '0') * 10 + (dateA[9] - '0')) * 10000L
                + ((dateA[3] - '0') * 10 + (dateA[4] - '0')) * 100 + (dateA[0] - '0') * 10 + (dateA[1] - '0');
            long keyB = static_cast<long>((dateB[6] - '0') * 1000 + (dateB[7] - '0') * 100 + (dateB[8] - '0') * 10 + (dateB[9] - '0')) * 10000L
                + ((dateB[3] - '0') * 10 + (dateB[4] - '0')) * 100 + (dateB[0] - '0') * 10 + (dateB[1] - '0');
            if (keyA > keyB)
            {
                int tmp = sortedIndices[j];
                sortedIndices[j] = sortedIndices[j + 1];
                sortedIndices[j + 1] = tmp;
            }
        }
    }

    // Column header row
    drawCard(30.f, 84.f, 1140.f, 28.f, Pal::hdr());
    drawText("ID", 46.f, 89.f, 12, Pal::textSec());
    drawText("Doctor", 150.f, 89.f, 12, Pal::textSec());
    drawText("Specialization", 380.f, 89.f, 12, Pal::textSec());
    drawText("Date", 620.f, 89.f, 12, Pal::textSec());
    drawText("Time", 780.f, 89.f, 12, Pal::textSec());
    drawText("Status", 920.f, 89.f, 12, Pal::textSec());

    if (apptCount == 0)
    {
        drawText("No appointments found.", 46.f, 140.f, 15, Pal::textSec());
    }

    // Scrollable clipped view for the list body
    float topY = 116.f;
    float rowHeight = 38.f;
    float visibleH = 580.f;
    float maxScroll = std::max(0.f, static_cast<float>(apptCount) * rowHeight - visibleH);
    if (listScrollY > maxScroll)
    {
        listScrollY = maxScroll;
    }

    sf::View listView(sf::Vector2f(570.f, visibleH / 2.f + listScrollY),
        sf::Vector2f(1140.f, visibleH));
    listView.setViewport(sf::FloatRect(
        sf::Vector2f(30.f / 1200.f, topY / 780.f),
        sf::Vector2f(1140.f / 1200.f, visibleH / 780.f)));
    window.setView(listView);

    for (int i = 0; i < apptCount; i++)
    {
        Appointment& appt = (*allAppointments)[sortedIndices[i]];
        Doctor* apptDoc = sys->getDoctors()->findById(appt.getDoctorId());
        float        rowY = static_cast<float>(i) * rowHeight;
        if (i % 2 == 0)
        {
            sf::RectangleShape altRow(sf::Vector2f(1140.f, rowHeight));
            altRow.setPosition(sf::Vector2f(0.f, rowY));
            altRow.setFillColor(withAlpha(Pal::card(), 80));
            window.draw(altRow);
        }
        char apptIdStr[10] = {};
        MyStr::intToStr(appt.getId(), apptIdStr);
        drawText(apptIdStr, 16.f, rowY + 10.f, 14, Pal::textSec());
        drawText(apptDoc ? apptDoc->getName() : "?", 120.f, rowY + 10.f, 14, Pal::textPri());
        drawText(apptDoc ? apptDoc->getSpecialization() : "?", 350.f, rowY + 10.f, 14, Pal::textSec());
        drawText(appt.getDate(), 590.f, rowY + 10.f, 14, Pal::textPri());
        drawText(appt.getTimeSlot(), 750.f, rowY + 10.f, 14, Pal::textPri());

        // Status colour coding
        sf::Color statusColor = Pal::textSec();
        if (MyStr::equals(appt.getStatus(), "completed")) { statusColor = Pal::success(); }
        else if (MyStr::equals(appt.getStatus(), "pending")) { statusColor = Pal::warning(); }
        else if (MyStr::equals(appt.getStatus(), "cancelled")) { statusColor = Pal::danger(); }
        else if (MyStr::equals(appt.getStatus(), "no-show")) { statusColor = Pal::danger(); }
        drawText(appt.getStatus(), 890.f, rowY + 10.f, 13, statusColor);
    }
    window.setView(window.getDefaultView());

    Button backButton;
    makeOutlineButton(backButton, 30.f, 706.f, 150.f, 38.f, "Back", Pal::textSec());
    if (drawButton(backButton))
    {
        currentScreen = Screen::PatientMenu;
        resetForm();
    }
}

 
//  PATIENT – MEDICAL RECORDS
 

void GUI::renderPatientRecords()
{
    drawTopBar("Medical Records");
    drawCard(20.f, 76.f, 1160.f, 650.f);

    // Collect prescriptions for this patient
    Storage<Prescription>* allPrescriptions = sys->getPrescriptions();
    int sortedPrescriptionIndices[100], prescriptionCount = 0;
    for (int i = 0; i < allPrescriptions->size(); i++)
    {
        if ((*allPrescriptions)[i].getPatientId() == currentUserId)
        {
            sortedPrescriptionIndices[prescriptionCount++] = i;
        }
    }

    // Sort descending by date so most recent records appear first
    for (int i = 0; i < prescriptionCount - 1; i++)
    {
        for (int j = 0; j < prescriptionCount - i - 1; j++)
        {
            const char* dateA = (*allPrescriptions)[sortedPrescriptionIndices[j]].getDate();
            const char* dateB = (*allPrescriptions)[sortedPrescriptionIndices[j + 1]].getDate();
            long keyA = static_cast<long>((dateA[6] - '0') * 1000 + (dateA[7] - '0') * 100 + (dateA[8] - '0') * 10 + (dateA[9] - '0')) * 10000L
                + ((dateA[3] - '0') * 10 + (dateA[4] - '0')) * 100 + (dateA[0] - '0') * 10 + (dateA[1] - '0');
            long keyB = static_cast<long>((dateB[6] - '0') * 1000 + (dateB[7] - '0') * 100 + (dateB[8] - '0') * 10 + (dateB[9] - '0')) * 10000L
                + ((dateB[3] - '0') * 10 + (dateB[4] - '0')) * 100 + (dateB[0] - '0') * 10 + (dateB[1] - '0');
            if (keyA < keyB)
            {
                int tmp = sortedPrescriptionIndices[j];
                sortedPrescriptionIndices[j] = sortedPrescriptionIndices[j + 1];
                sortedPrescriptionIndices[j + 1] = tmp;
            }
        }
    }

    if (prescriptionCount == 0)
    {
        drawText("No records.", 46.f, 140.f, 15, Pal::textSec());
    }

    float rowY = 90.f;
    for (int i = 0; i < prescriptionCount && rowY < 660.f; i++)
    {
        Prescription& rx = (*allPrescriptions)[sortedPrescriptionIndices[i]];
        Doctor* rxDoc = sys->getDoctors()->findById(rx.getDoctorId());
        drawCard(30.f, rowY, 1140.f, 70.f, i % 2 == 0 ? Pal::card() : Pal::panel());

        drawBadge(rx.getDate(), 46.f, rowY + 6.f, Pal::accent());
        drawText(rxDoc ? rxDoc->getName() : "Unknown", 200.f, rowY + 4.f, 14, Pal::textPri(), true);
        char medicinesLine[600] = "Meds: ";
        MyStr::concat(medicinesLine, rx.getMedicines());
        drawText(medicinesLine, 46.f, rowY + 36.f, 13, Pal::textSec());
        char notesLine[400] = "Notes: ";
        MyStr::concat(notesLine, rx.getNotes());
        drawText(notesLine, 500.f, rowY + 36.f, 13, withAlpha(Pal::textSec(), 180));
        rowY += 76.f;
    }

    Button backButton;
    makeOutlineButton(backButton, 30.f, 706.f, 150.f, 38.f, "Back", Pal::textSec());
    if (drawButton(backButton))
    {
        currentScreen = Screen::PatientMenu;
        resetForm();
    }
}

 
//  PATIENT – BILLS
 

void GUI::renderPatientBills()
{
    drawTopBar("My Bills");
    drawCard(20.f, 76.f, 900.f, 650.f);

    Storage<Bill>* allBills = sys->getBills();
    float          totalUnpaid = 0.f;
    float          totalPaid = 0.f;
    int            billRowYInt = 90;
    int            billRowCount = 0;

    for (int i = 0; i < allBills->size(); i++)
    {
        Bill& bill = (*allBills)[i];
        if (bill.getPatientId() != currentUserId)
        {
            continue;
        }
        float rowY = static_cast<float>(billRowYInt);
        drawCard(30.f, rowY, 880.f, 44.f, billRowCount % 2 == 0 ? Pal::card() : Pal::panel());

        char billIdStr[10] = {}, apptIdStr[10] = {}, amountStr[20] = {};
        MyStr::intToStr(bill.getId(), billIdStr);
        MyStr::intToStr(bill.getAppointmentId(), apptIdStr);
        MyStr::floatToStr(bill.getAmount(), amountStr, 2);

        char billInfoLine[300];
        MyStr::copy(billInfoLine, "Bill #");
        MyStr::concat(billInfoLine, billIdStr);
        MyStr::concat(billInfoLine, "  |  Appt #");
        MyStr::concat(billInfoLine, apptIdStr);
        MyStr::concat(billInfoLine, "  |  PKR ");
        MyStr::concat(billInfoLine, amountStr);
        MyStr::concat(billInfoLine, "  |  ");
        MyStr::concat(billInfoLine, bill.getDate());
        drawText(billInfoLine, 46.f, rowY + 13.f, 14, Pal::textPri());

        sf::Color statusColor = MyStr::equals(bill.getStatus(), "paid") ? Pal::success()
            : MyStr::equals(bill.getStatus(), "unpaid") ? Pal::danger()
            : Pal::textSec();
        drawBadge(bill.getStatus(), 830.f, rowY + 12.f, statusColor);

        if (MyStr::equals(bill.getStatus(), "unpaid")) { totalUnpaid += bill.getAmount(); }
        else if (MyStr::equals(bill.getStatus(), "paid")) { totalPaid += bill.getAmount(); }
        billRowYInt += 48;
        billRowCount++;
    }
    if (billRowCount == 0)
    {
        drawText("No bills.", 46.f, 140.f, 15, Pal::textSec());
    }

    // Summary card on the right
    drawCard(934.f, 76.f, 246.f, 200.f);
    drawText("Summary", 950.f, 92.f, 15, Pal::textPri(), true);
    drawSeparator(114.f, 944.f, 1170.f);

    char unpaidLabel[30] = "PKR ";
    char unpaidAmtStr[20] = {};
    MyStr::floatToStr(totalUnpaid, unpaidAmtStr, 2);
    MyStr::concat(unpaidLabel, unpaidAmtStr);
    drawText("Outstanding", 950.f, 126.f, 12, Pal::textSec());
    drawText(unpaidLabel, 950.f, 144.f, 18, Pal::danger(), true);

    char paidLabel[30] = "PKR ";
    char paidAmtStr[20] = {};
    MyStr::floatToStr(totalPaid, paidAmtStr, 2);
    MyStr::concat(paidLabel, paidAmtStr);
    drawText("Paid", 950.f, 184.f, 12, Pal::textSec());
    drawText(paidLabel, 950.f, 202.f, 18, Pal::success(), true);

    Button backButton;
    makeOutlineButton(backButton, 30.f, 706.f, 150.f, 38.f, "Back", Pal::textSec());
    if (drawButton(backButton))
    {
        currentScreen = Screen::PatientMenu;
        resetForm();
    }
}

 
//  PATIENT – PAY BILL
 

void GUI::renderPatientPay()
{
    drawTopBar("Pay Bill");
    drawCard(20.f, 76.f, 760.f, 640.f);
    drawText("Unpaid Bills", 36.f, 90.f, 16, Pal::textPri(), true);
    drawSeparator(114.f, 30.f, 770.f);

    if (formBoxCount == 0)
    {
        makeTextBox(formBoxes[0], 800.f, 160.f, 360.f, 44.f,
            "Bill ID", "Enter bill ID to pay");
        formBoxCount = 1;
    }

    // Show all unpaid bills so the patient can identify the correct ID
    Storage<Bill>* allBills = sys->getBills();
    float          listY = 124.f;
    int            unpaidCount = 0;
    for (int i = 0; i < allBills->size(); i++)
    {
        Bill& bill = (*allBills)[i];
        if (bill.getPatientId() != currentUserId
            || !MyStr::equals(bill.getStatus(), "unpaid"))
        {
            continue;
        }
        drawCard(30.f, listY, 730.f, 42.f, unpaidCount % 2 == 0 ? Pal::card() : Pal::panel());
        char billIdStr[10] = {}, amountStr[20] = {};
        MyStr::intToStr(bill.getId(), billIdStr);
        MyStr::floatToStr(bill.getAmount(), amountStr, 2);
        char billInfoLine[200];
        MyStr::copy(billInfoLine, "Bill #");
        MyStr::concat(billInfoLine, billIdStr);
        MyStr::concat(billInfoLine, "  |  PKR ");
        MyStr::concat(billInfoLine, amountStr);
        MyStr::concat(billInfoLine, "  |  ");
        MyStr::concat(billInfoLine, bill.getDate());
        drawText(billInfoLine, 46.f, listY + 12.f, 14, Pal::textPri());
        listY += 46.f;
        unpaidCount++;
    }
    if (unpaidCount == 0)
    {
        drawText("No unpaid bills.", 46.f, 150.f, 15, Pal::textSec());
    }

    // Right panel: payment form
    drawCard(790.f, 76.f, 390.f, 640.f);
    drawText("Pay Selected Bill", 806.f, 90.f, 16, Pal::textPri(), true);
    drawSeparator(114.f, 800.f, 1170.f);
    drawTextBox(formBoxes[0]);

    // Show current wallet balance for reference
    Patient* payingPatient = sys->getPatients()->findById(currentUserId);
    if (payingPatient)
    {
        char walletLabel[40] = "Wallet  PKR ";
        char walletAmtStr[20];
        MyStr::floatToStr(payingPatient->getBalance(), walletAmtStr, 2);
        MyStr::concat(walletLabel, walletAmtStr);
        drawText(walletLabel, 806.f, 226.f, 14, Pal::success());
    }

    Button payButton, backButton;
    makeButton(payButton, 800.f, 280.f, 360.f, 44.f, "Pay Now", Pal::success());
    makeOutlineButton(backButton, 800.f, 336.f, 170.f, 38.f, "Back", Pal::textSec());

    if (drawButton(backButton))
    {
        currentScreen = Screen::PatientMenu;
        resetForm();
        return;
    }

    if (drawButton(payButton))
    {
        try
        {
            if (!Validator::isValidId(formBoxes[0].content))
            {
                throw InvalidInputException("Invalid Bill ID.");
            }
            int   enteredBillId = MyStr::toInt(formBoxes[0].content);
            Bill* targetBill = sys->getBills()->findById(enteredBillId);
            if (!targetBill || targetBill->getPatientId() != currentUserId
                || !MyStr::equals(targetBill->getStatus(), "unpaid"))
            {
                throw InvalidInputException("Bill not found or already paid.");
            }
            Patient* payingPat = sys->getPatients()->findById(currentUserId);
            if (payingPat->getBalance() < targetBill->getAmount())
            {
                throw InsufficientFundsException("Insufficient wallet balance. Please top up.");
            }

            // Deduct the amount and mark the bill paid, then persist both records
            *payingPat -= targetBill->getAmount();
            targetBill->setStatus("paid");
            FileHandler::savePatients(sys->getPatients());
            FileHandler::saveBills(sys->getBills());

            char payMsg[100] = "Payment successful! Remaining: PKR ";
            char remainStr[20];
            MyStr::floatToStr(payingPat->getBalance(), remainStr, 2);
            MyStr::concat(payMsg, remainStr);
            setMessage(payMsg, true);
        }
        catch (HospitalException& ex)
        {
            setMessage(ex.what(), false);
        }
    }
}

 
//  PATIENT – TOP UP BALANCE
 

void GUI::renderPatientTopUp()
{
    drawTopBar("Top Up Balance");
    drawCard(400.f, 160.f, 400.f, 280.f);
    drawText("Add Funds to Wallet", 420.f, 178.f, 17, Pal::textPri(), true);
    drawSeparator(204.f, 410.f, 790.f);

    if (formBoxCount == 0)
    {
        makeTextBox(formBoxes[0], 420.f, 228.f, 360.f, 48.f,
            "Amount (PKR)", "Enter amount to add");
        formBoxCount = 1;
    }
    drawTextBox(formBoxes[0]);

    Patient* topUpPatient = sys->getPatients()->findById(currentUserId);
    if (topUpPatient)
    {
        char currentBalanceLabel[40] = "Current balance: PKR ";
        char currentBalanceStr[20];
        MyStr::floatToStr(topUpPatient->getBalance(), currentBalanceStr, 2);
        MyStr::concat(currentBalanceLabel, currentBalanceStr);
        drawText(currentBalanceLabel, 420.f, 296.f, 13, Pal::textSec());
    }

    Button addButton, backButton;
    makeButton(addButton, 420.f, 322.f, 360.f, 44.f, "Add Funds", Pal::success());
    makeOutlineButton(backButton, 420.f, 378.f, 170.f, 38.f, "Back", Pal::textSec());

    if (drawButton(backButton))
    {
        currentScreen = Screen::PatientMenu;
        resetForm();
        return;
    }

    if (drawButton(addButton))
    {
        try
        {
            if (!Validator::isPositiveFloat(formBoxes[0].content))
            {
                throw InvalidInputException("Amount must be a positive number.");
            }
            Patient* updatedPatient = sys->getPatients()->findById(currentUserId);
            *updatedPatient += MyStr::toFloat(formBoxes[0].content);
            FileHandler::savePatients(sys->getPatients());

            char newBalanceMsg[100] = "Balance updated! New balance: PKR ";
            char newBalanceStr[20];
            MyStr::floatToStr(updatedPatient->getBalance(), newBalanceStr, 2);
            MyStr::concat(newBalanceMsg, newBalanceStr);
            setMessage(newBalanceMsg, true);

            // Clear the amount field so the user can enter another top-up
            formBoxes[0].len = 0;
            formBoxes[0].content[0] = '\0';
        }
        catch (HospitalException& ex)
        {
            setMessage(ex.what(), false);
        }
    }
}

 
//  DOCTOR MENU
 

void GUI::renderDoctorMenu()
{
    Doctor* currentDoctor = sys->getDoctors()->findById(currentUserId);
    if (!currentDoctor)
    {
        currentScreen = Screen::Login;
        return;
    }

    drawTopBar("Doctor Dashboard");
    drawCard(20.f, 76.f, 340.f, 120.f);
    drawText("DOCTOR", 36.f, 86.f, 11, Pal::textSec());
    drawText(currentDoctor->getName(), 36.f, 102.f, 20, Pal::textPri(), true);
    drawText(currentDoctor->getSpecialization(), 36.f, 130.f, 13, Pal::accent());

    const char* actionLabels[] = {
        "Today's Schedule", "Mark Complete", "Mark No-Show",
        "Write Prescription", "Patient History", "Logout"
    };
    Screen actionTargets[] = {
        Screen::DoctorTodayAppts, Screen::DoctorComplete,
        Screen::DoctorNoShow,     Screen::DoctorPrescription,
        Screen::DoctorPatientHistory, Screen::Login
    };
    sf::Color actionColors[] = {
        Pal::accent(),   Pal::success(), Pal::warning(),
        Pal::accentDk(), Pal::accentDk(), Pal::danger()
    };

    for (int i = 0; i < 6; i++)
    {
        float cardX = 20.f + static_cast<float>(i % 2) * 380.f;
        float cardY = 210.f + static_cast<float>(i / 2) * 80.f;
        drawCard(cardX, cardY, 360.f, 64.f);
        sf::RectangleShape leftStripe(sf::Vector2f(4.f, 64.f));
        leftStripe.setPosition(sf::Vector2f(cardX, cardY));
        leftStripe.setFillColor(actionColors[i]);
        window.draw(leftStripe);
        drawText(actionLabels[i], cardX + 18.f, cardY + 22.f, 16, Pal::textPri());
        Button openButton;
        makeButton(openButton, cardX + 290.f, cardY + 16.f, 60.f, 30.f, "Open",
            withAlpha(actionColors[i], 180));
        if (drawButton(openButton))
        {
            resetForm();
            currentScreen = actionTargets[i];
            if (i == 5)
            {
                currentRole = 0;
                currentUserId = 0;
            }
        }
    }
}

 
//  DOCTOR – TODAY'S SCHEDULE
 

void GUI::renderDoctorToday()
{
    drawTopBar("Today's Schedule");
    drawCard(20.f, 76.f, 1160.f, 650.f);

    char todayDate[11];
    sys->getTodayDate(todayDate);

    // Collect and sort today's appointments for this doctor by time slot
    Storage<Appointment>* allAppointments = sys->getAppointments();
    int sortedIndices[100], apptCount = 0;
    for (int i = 0; i < allAppointments->size(); i++)
    {
        if ((*allAppointments)[i].getDoctorId() == currentUserId
            && MyStr::equals((*allAppointments)[i].getDate(), todayDate))
        {
            sortedIndices[apptCount++] = i;
        }
    }

    // Sort ascending by time slot (HH:MM parsed as integer)
    for (int i = 0; i < apptCount - 1; i++)
    {
        for (int j = 0; j < apptCount - i - 1; j++)
        {
            int timeA = MyStr::toInt((*allAppointments)[sortedIndices[j]].getTimeSlot());
            int timeB = MyStr::toInt((*allAppointments)[sortedIndices[j + 1]].getTimeSlot());
            if (timeA > timeB)
            {
                int tmp = sortedIndices[j];
                sortedIndices[j] = sortedIndices[j + 1];
                sortedIndices[j + 1] = tmp;
            }
        }
    }

    // Column header
    drawCard(30.f, 84.f, 1140.f, 28.f, Pal::hdr());
    drawText("Time", 46.f, 89.f, 12, Pal::textSec());
    drawText("Patient", 200.f, 89.f, 12, Pal::textSec());
    drawText("Status", 900.f, 89.f, 12, Pal::textSec());

    if (apptCount == 0)
    {
        drawText("No appointments today.", 46.f, 140.f, 15, Pal::textSec());
    }

    for (int i = 0; i < apptCount; i++)
    {
        Appointment& appt = (*allAppointments)[sortedIndices[i]];
        Patient* apptPat = sys->getPatients()->findById(appt.getPatientId());
        float        rowY = 116.f + static_cast<float>(i) * 46.f;
        drawCard(30.f, rowY, 1140.f, 40.f, i % 2 == 0 ? Pal::card() : Pal::panel());
        drawText(appt.getTimeSlot(), 46.f, rowY + 12.f, 14, Pal::accent());
        drawText(apptPat ? apptPat->getName() : "Unknown", 200.f, rowY + 12.f, 14, Pal::textPri());

        sf::Color statusColor = MyStr::equals(appt.getStatus(), "pending") ? Pal::warning()
            : MyStr::equals(appt.getStatus(), "completed") ? Pal::success()
            : Pal::danger();
        drawBadge(appt.getStatus(), 870.f, rowY + 10.f, statusColor);
    }

    Button backButton;
    makeOutlineButton(backButton, 30.f, 706.f, 150.f, 38.f, "Back", Pal::textSec());
    if (drawButton(backButton))
    {
        currentScreen = Screen::DoctorMenu;
        resetForm();
    }
}

 
//  DOCTOR – MARK COMPLETE
 

void GUI::renderDoctorComplete()
{
    drawTopBar("Mark Appointment Complete");
    drawCard(20.f, 76.f, 760.f, 640.f);
    drawText("Pending Today", 36.f, 90.f, 16, Pal::textPri(), true);
    drawSeparator(114.f, 30.f, 770.f);

    if (formBoxCount == 0)
    {
        makeTextBox(formBoxes[0], 800.f, 160.f, 360.f, 44.f,
            "Appointment ID", "Enter ID to mark complete");
        formBoxCount = 1;
    }

    char todayDate[11];
    sys->getTodayDate(todayDate);

    // Show only today's pending appointments for this doctor
    Storage<Appointment>* allAppointments = sys->getAppointments();
    float listY = 124.f;
    int   rowCount = 0;
    for (int i = 0; i < allAppointments->size(); i++)
    {
        Appointment& appt = (*allAppointments)[i];
        if (appt.getDoctorId() != currentUserId
            || !MyStr::equals(appt.getDate(), todayDate)
            || !MyStr::equals(appt.getStatus(), "pending"))
        {
            continue;
        }
        Patient* apptPat = sys->getPatients()->findById(appt.getPatientId());
        drawCard(30.f, listY, 730.f, 42.f, rowCount % 2 == 0 ? Pal::card() : Pal::panel());
        char rowBuf[200], apptIdStr[10];
        MyStr::intToStr(appt.getId(), apptIdStr);
        MyStr::copy(rowBuf, "#");
        MyStr::concat(rowBuf, apptIdStr);
        MyStr::concat(rowBuf, "  ");
        MyStr::concat(rowBuf, appt.getTimeSlot());
        MyStr::concat(rowBuf, "  |  ");
        MyStr::concat(rowBuf, apptPat ? apptPat->getName() : "?");
        drawText(rowBuf, 46.f, listY + 12.f, 14, Pal::textPri());
        listY += 46.f;
        rowCount++;
    }
    if (rowCount == 0)
    {
        drawText("No pending appointments today.", 46.f, 150.f, 15, Pal::textSec());
    }

    drawCard(790.f, 76.f, 390.f, 640.f);
    drawText("Mark Complete", 806.f, 90.f, 16, Pal::textPri(), true);
    drawSeparator(114.f, 800.f, 1170.f);
    drawTextBox(formBoxes[0]);

    Button markButton, backButton;
    makeButton(markButton, 800.f, 220.f, 360.f, 44.f, "Mark as Completed", Pal::success());
    makeOutlineButton(backButton, 800.f, 276.f, 170.f, 38.f, "Back", Pal::textSec());

    if (drawButton(backButton))
    {
        currentScreen = Screen::DoctorMenu;
        resetForm();
        return;
    }
    if (drawButton(markButton))
    {
        if (!Validator::isValidId(formBoxes[0].content))
        {
            setMessage("Invalid ID.", false);
            return;
        }
        int          enteredApptId = MyStr::toInt(formBoxes[0].content);
        Appointment* targetAppt = sys->getAppointments()->findById(enteredApptId);
        if (!targetAppt || targetAppt->getDoctorId() != currentUserId
            || !MyStr::equals(targetAppt->getStatus(), "pending")
            || !MyStr::equals(targetAppt->getDate(), todayDate))
        {
            setMessage("Invalid appointment.", false);
            return;
        }
        targetAppt->setStatus("completed");
        FileHandler::saveAppointments(sys->getAppointments());
        setMessage("Appointment marked as completed.", true);
    }
}

 
//  DOCTOR – MARK NO-SHOW
 

void GUI::renderDoctorNoShow()
{
    drawTopBar("Mark No-Show");
    drawCard(20.f, 76.f, 760.f, 640.f);
    drawText("Pending Today", 36.f, 90.f, 16, Pal::textPri(), true);
    drawSeparator(114.f, 30.f, 770.f);

    if (formBoxCount == 0)
    {
        makeTextBox(formBoxes[0], 800.f, 160.f, 360.f, 44.f,
            "Appointment ID", "Enter ID to mark no-show");
        formBoxCount = 1;
    }

    char todayDate[11];
    sys->getTodayDate(todayDate);

    Storage<Appointment>* allAppointments = sys->getAppointments();
    float listY = 124.f;
    int   rowCount = 0;
    for (int i = 0; i < allAppointments->size(); i++)
    {
        Appointment& appt = (*allAppointments)[i];
        if (appt.getDoctorId() != currentUserId
            || !MyStr::equals(appt.getDate(), todayDate)
            || !MyStr::equals(appt.getStatus(), "pending"))
        {
            continue;
        }
        Patient* apptPat = sys->getPatients()->findById(appt.getPatientId());
        drawCard(30.f, listY, 730.f, 42.f, rowCount % 2 == 0 ? Pal::card() : Pal::panel());
        char rowBuf[200], apptIdStr[10];
        MyStr::intToStr(appt.getId(), apptIdStr);
        MyStr::copy(rowBuf, "#");
        MyStr::concat(rowBuf, apptIdStr);
        MyStr::concat(rowBuf, "  ");
        MyStr::concat(rowBuf, appt.getTimeSlot());
        MyStr::concat(rowBuf, "  |  ");
        MyStr::concat(rowBuf, apptPat ? apptPat->getName() : "?");
        drawText(rowBuf, 46.f, listY + 12.f, 14, Pal::textPri());
        listY += 46.f;
        rowCount++;
    }
    if (rowCount == 0)
    {
        drawText("No pending appointments today.", 46.f, 150.f, 15, Pal::textSec());
    }

    drawCard(790.f, 76.f, 390.f, 640.f);
    drawText("Mark No-Show", 806.f, 90.f, 16, Pal::textPri(), true);
    drawSeparator(114.f, 800.f, 1170.f);
    drawTextBox(formBoxes[0]);

    Button markButton, backButton;
    makeButton(markButton, 800.f, 220.f, 360.f, 44.f, "Mark as No-Show", Pal::warning());
    makeOutlineButton(backButton, 800.f, 276.f, 170.f, 38.f, "Back", Pal::textSec());

    if (drawButton(backButton))
    {
        currentScreen = Screen::DoctorMenu;
        resetForm();
        return;
    }
    if (drawButton(markButton))
    {
        if (!Validator::isValidId(formBoxes[0].content))
        {
            setMessage("Invalid ID.", false);
            return;
        }
        int          enteredApptId = MyStr::toInt(formBoxes[0].content);
        Appointment* targetAppt = sys->getAppointments()->findById(enteredApptId);
        if (!targetAppt || targetAppt->getDoctorId() != currentUserId
            || !MyStr::equals(targetAppt->getStatus(), "pending")
            || !MyStr::equals(targetAppt->getDate(), todayDate))
        {
            setMessage("Invalid appointment.", false);
            return;
        }

        // Mark the appointment and cancel its associated bill (no charge for no-shows)
        targetAppt->setStatus("no-show");
        Storage<Bill>* allBills = sys->getBills();
        for (int i = 0; i < allBills->size(); i++)
        {
            if ((*allBills)[i].getAppointmentId() == enteredApptId)
            {
                (*allBills)[i].setStatus("cancelled");
            }
        }
        FileHandler::saveAppointments(sys->getAppointments());
        FileHandler::saveBills(sys->getBills());
        setMessage("Marked as no-show.", true);
    }
}

 
//  DOCTOR – WRITE PRESCRIPTION
 

void GUI::renderDoctorPrescription()
{
    drawTopBar("Write Prescription");
    drawCard(20.f, 76.f, 1160.f, 650.f);
    drawText("New Prescription", 36.f, 90.f, 16, Pal::textPri(), true);
    drawSeparator(114.f, 30.f, 1170.f);

    if (formBoxCount == 0)
    {
        makeTextBox(formBoxes[0], 36.f, 148.f, 400.f, 44.f,
            "Appointment ID", "Enter completed appointment ID");
        makeTextBox(formBoxes[1], 36.f, 234.f, 1100.f, 44.f,
            "Medicines", "e.g. Paracetamol 500mg; Ibuprofen 400mg");
        makeTextBox(formBoxes[2], 36.f, 320.f, 1100.f, 44.f,
            "Notes", "Clinical notes, instructions, follow-up");
        formBoxCount = 3;
    }
    for (int i = 0; i < formBoxCount; i++)
    {
        drawTextBox(formBoxes[i]);
    }

    Button saveButton, backButton;
    makeButton(saveButton, 36.f, 390.f, 300.f, 44.f, "Save Prescription", Pal::success());
    makeOutlineButton(backButton, 356.f, 390.f, 160.f, 44.f, "Back", Pal::textSec());

    if (drawButton(backButton))
    {
        currentScreen = Screen::DoctorMenu;
        resetForm();
        return;
    }

    if (drawButton(saveButton))
    {
        if (!Validator::isValidId(formBoxes[0].content))
        {
            setMessage("Invalid Appointment ID.", false);
            return;
        }
        int          enteredApptId = MyStr::toInt(formBoxes[0].content);
        Appointment* targetAppt = sys->getAppointments()->findById(enteredApptId);

        // Prescription can only be written for a completed appointment assigned to this doctor
        if (!targetAppt || targetAppt->getDoctorId() != currentUserId
            || !MyStr::equals(targetAppt->getStatus(), "completed"))
        {
            setMessage("Appointment must be completed and assigned to you.", false);
            return;
        }

        // Prevent duplicate prescriptions for the same appointment
        Storage<Prescription>* allPrescriptions = sys->getPrescriptions();
        for (int i = 0; i < allPrescriptions->size(); i++)
        {
            if ((*allPrescriptions)[i].getAppointmentId() == enteredApptId)
            {
                setMessage("Prescription already exists for this appointment.", false);
                return;
            }
        }

        int          newPrescriptionId = sys->nextPrescriptionId();
        Prescription newPrescription(newPrescriptionId, enteredApptId,
            targetAppt->getPatientId(), currentUserId,
            targetAppt->getDate(),
            formBoxes[1].content, formBoxes[2].content);
        allPrescriptions->add(newPrescription);
        FileHandler::appendPrescription(&newPrescription);
        setMessage("Prescription saved successfully.", true);
    }
}

 
//  DOCTOR – PATIENT HISTORY
 

void GUI::renderDoctorPatientHistory()
{
    drawTopBar("Patient History");
    drawCard(20.f, 76.f, 500.f, 650.f);
    drawText("Search Patient", 36.f, 90.f, 16, Pal::textPri(), true);
    drawSeparator(114.f, 30.f, 510.f);

    if (formBoxCount == 0)
    {
        makeTextBox(formBoxes[0], 36.f, 148.f, 440.f, 44.f,
            "Patient ID", "Enter patient numeric ID");
        formBoxCount = 1;
    }
    drawTextBox(formBoxes[0]);

    Button viewButton, backButton;
    makeButton(viewButton, 36.f, 214.f, 210.f, 44.f, "View History", Pal::accent());
    makeOutlineButton(backButton, 258.f, 214.f, 160.f, 44.f, "Back", Pal::textSec());

    // searchedPatientId persists between frames so results stay visible
    static int searchedPatientId = 0;

    if (drawButton(backButton))
    {
        currentScreen = Screen::DoctorMenu;
        resetForm();
        searchedPatientId = 0;
        return;
    }

    if (drawButton(viewButton))
    {
        if (!Validator::isValidId(formBoxes[0].content))
        {
            setMessage("Invalid Patient ID.", false);
            searchedPatientId = 0;
            return;
        }
        int      enteredPatientId = MyStr::toInt(formBoxes[0].content);
        Patient* foundPatient = sys->getPatients()->findById(enteredPatientId);
        if (!foundPatient)
        {
            setMessage("Patient not found.", false);
            searchedPatientId = 0;
            return;
        }

        // Only allow access if this doctor has at least one completed appointment with the patient
        bool hasCompletedAppt = false;
        Storage<Appointment>* allAppointments = sys->getAppointments();
        for (int i = 0; i < allAppointments->size(); i++)
        {
            if ((*allAppointments)[i].getPatientId() == enteredPatientId
                && (*allAppointments)[i].getDoctorId() == currentUserId
                && MyStr::equals((*allAppointments)[i].getStatus(), "completed"))
            {
                hasCompletedAppt = true;
                break;
            }
        }
        if (!hasCompletedAppt)
        {
            setMessage("Access denied: no completed appointments with this patient.", false);
            searchedPatientId = 0;
            return;
        }
        searchedPatientId = enteredPatientId;
        setMessage("Records loaded.", true);
    }

    // Patient summary shown below the search form once a valid search has been made
    if (searchedPatientId != 0)
    {
        Patient* displayedPatient = sys->getPatients()->findById(searchedPatientId);
        if (displayedPatient)
        {
            drawText(displayedPatient->getName(), 36.f, 276.f, 16, Pal::accent(), true);
            char ageLine[40];
            char ageStr[10];
            MyStr::intToStr(displayedPatient->getAge(), ageStr);
            MyStr::copy(ageLine, "Age: ");
            MyStr::concat(ageLine, ageStr);
            char genderChar[2] = { displayedPatient->getGender(), '\0' };
            MyStr::concat(ageLine, "  |  Gender: ");
            MyStr::concat(ageLine, genderChar);
            drawText(ageLine, 36.f, 300.f, 13, Pal::textSec());
        }
    }

    // Right panel: prescription history
    drawCard(530.f, 76.f, 650.f, 650.f);
    drawText("Prescriptions", 546.f, 90.f, 16, Pal::textPri(), true);
    drawSeparator(114.f, 540.f, 1170.f);

    if (searchedPatientId == 0)
    {
        drawText("Search for a patient to view history.", 546.f, 150.f, 14, Pal::textSec());
        return;
    }

    Storage<Prescription>* allPrescriptions = sys->getPrescriptions();
    int prescriptionIndices[100], prescriptionCount = 0;
    for (int i = 0; i < allPrescriptions->size(); i++)
    {
        if ((*allPrescriptions)[i].getPatientId() == searchedPatientId
            && (*allPrescriptions)[i].getDoctorId() == currentUserId)
        {
            prescriptionIndices[prescriptionCount++] = i;
        }
    }

    if (prescriptionCount == 0)
    {
        drawText("No records.", 546.f, 150.f, 14, Pal::textSec());
        return;
    }

    float rowY = 124.f;
    for (int i = 0; i < prescriptionCount && rowY < 680.f; i++)
    {
        Prescription& rx = (*allPrescriptions)[prescriptionIndices[i]];
        drawCard(540.f, rowY, 630.f, 70.f, i % 2 == 0 ? Pal::card() : Pal::panel());
        drawBadge(rx.getDate(), 556.f, rowY + 6.f, Pal::accent());
        char medicinesLine[600] = "Meds: ";
        MyStr::concat(medicinesLine, rx.getMedicines());
        drawText(medicinesLine, 556.f, rowY + 30.f, 13, Pal::textSec());
        char notesLine[400] = "Notes: ";
        MyStr::concat(notesLine, rx.getNotes());
        drawText(notesLine, 556.f, rowY + 48.f, 12, withAlpha(Pal::textSec(), 160));
        rowY += 76.f;
    }
}

 
//  ADMIN MENU
 

void GUI::renderAdminMenu()
{
    drawTopBar("Admin Control Panel");
    drawCard(20.f, 76.f, 340.f, 80.f);
    drawText("ADMINISTRATOR", 36.f, 90.f, 11, Pal::textSec());
    drawText("Full System Access", 36.f, 108.f, 16, Pal::textPri(), true);

    const char* actionLabels[] = {
        "Add Doctor",        "Remove Doctor",    "All Patients",     "All Doctors",
        "All Appointments",  "Unpaid Bills",     "Discharge Patient",
        "Security Log",      "Daily Report",     "Logout"
    };
    Screen actionTargets[] = {
        Screen::AdminAddDoctor,    Screen::AdminRemoveDoctor,
        Screen::AdminViewPatients, Screen::AdminViewDoctors,
        Screen::AdminViewAppts,    Screen::AdminUnpaidBills,
        Screen::AdminDischarge,    Screen::AdminSecurityLog,
        Screen::AdminDailyReport,  Screen::Login
    };
    sf::Color actionColors[] = {
        Pal::success(),  Pal::danger(),   Pal::accent(),  Pal::accent(),
        Pal::accentDk(), Pal::warning(),  Pal::warning(),
        Pal::textSec(),  Pal::accentDk(), Pal::danger()
    };

    for (int i = 0; i < 10; i++)
    {
        float cardX = 20.f + static_cast<float>(i % 2) * 380.f;
        float cardY = 176.f + static_cast<float>(i / 2) * 80.f;
        drawCard(cardX, cardY, 360.f, 64.f);
        sf::RectangleShape leftStripe(sf::Vector2f(4.f, 64.f));
        leftStripe.setPosition(sf::Vector2f(cardX, cardY));
        leftStripe.setFillColor(actionColors[i]);
        window.draw(leftStripe);
        drawText(actionLabels[i], cardX + 18.f, cardY + 22.f, 15, Pal::textPri());
        Button openButton;
        makeButton(openButton, cardX + 290.f, cardY + 16.f, 60.f, 30.f, "Open",
            withAlpha(actionColors[i], 180));
        if (drawButton(openButton))
        {
            resetForm();
            currentScreen = actionTargets[i];
            if (i == 9)
            {
                currentRole = 0;
                currentUserId = 0;
            }
        }
    }
}

 
//  ADMIN – ADD DOCTOR
 

void GUI::renderAdminAddDoctor()
{
    drawTopBar("Add New Doctor");
    drawCard(300.f, 76.f, 600.f, 640.f);
    drawText("Doctor Registration", 320.f, 94.f, 17, Pal::textPri(), true);
    drawSeparator(120.f, 310.f, 890.f);

    if (formBoxCount == 0)
    {
        makeTextBox(formBoxes[0], 320.f, 148.f, 560.f, 44.f, "Full Name", "Dr. First Last");
        makeTextBox(formBoxes[1], 320.f, 234.f, 560.f, 44.f, "Specialization", "e.g. Cardiology");
        makeTextBox(formBoxes[2], 320.f, 320.f, 560.f, 44.f, "Contact", "11-digit number");
        makeTextBox(formBoxes[3], 320.f, 406.f, 560.f, 44.f, "Password", "Minimum 6 characters", true);
        makeTextBox(formBoxes[4], 320.f, 492.f, 560.f, 44.f, "Consultation Fee", "Amount in PKR");
        formBoxCount = 5;
    }
    for (int i = 0; i < formBoxCount; i++)
    {
        drawTextBox(formBoxes[i]);
    }

    Button registerButton, backButton;
    makeButton(registerButton, 320.f, 562.f, 560.f, 44.f, "Register Doctor", Pal::success());
    makeOutlineButton(backButton, 320.f, 618.f, 170.f, 38.f, "Back", Pal::textSec());

    if (drawButton(backButton))
    {
        currentScreen = Screen::AdminMenu;
        resetForm();
        return;
    }

    if (drawButton(registerButton))
    {
        // Validate all fields before creating the doctor
        if (formBoxes[0].len == 0 || formBoxes[0].len > 50)
        {
            setMessage("Invalid name.", false);
            return;
        }
        if (formBoxes[1].len == 0 || formBoxes[1].len > 50)
        {
            setMessage("Invalid specialization.", false);
            return;
        }
        if (!Validator::isValidContact(formBoxes[2].content))
        {
            setMessage("Contact must be exactly 11 digits.", false);
            return;
        }
        if (!Validator::isValidPassword(formBoxes[3].content))
        {
            setMessage("Password must be at least 6 characters.", false);
            return;
        }
        if (!Validator::isPositiveFloat(formBoxes[4].content))
        {
            setMessage("Fee must be a positive number.", false);
            return;
        }

        int    newDoctorId = sys->nextDoctorId();
        Doctor newDoctor(newDoctorId, formBoxes[0].content, formBoxes[1].content,
            formBoxes[2].content, formBoxes[3].content,
            MyStr::toFloat(formBoxes[4].content));
        sys->getDoctors()->add(newDoctor);
        FileHandler::appendDoctor(&newDoctor);

        char successMsg[100] = "Doctor registered successfully! ID: ";
        char newIdStr[10] = {};
        MyStr::intToStr(newDoctorId, newIdStr);
        MyStr::concat(successMsg, newIdStr);
        setMessage(successMsg, true);

        // Clear all fields so admin can register another doctor without clicking Back
        for (int i = 0; i < 5; i++)
        {
            formBoxes[i].len = 0;
            formBoxes[i].content[0] = '\0';
        }
    }
}

 
//  ADMIN – REMOVE DOCTOR
 

void GUI::renderAdminRemoveDoctor()
{
    drawTopBar("Remove Doctor");
    drawCard(20.f, 76.f, 760.f, 650.f);
    drawText("All Doctors", 36.f, 90.f, 16, Pal::textPri(), true);
    drawSeparator(114.f, 30.f, 770.f);

    if (formBoxCount == 0)
    {
        makeTextBox(formBoxes[0], 800.f, 160.f, 360.f, 44.f,
            "Doctor ID", "Enter doctor ID to remove");
        formBoxCount = 1;
    }

    // List all doctors so the admin can find the correct ID
    Storage<Doctor>* allDoctors = sys->getDoctors();
    float listY = 124.f;
    for (int i = 0; i < allDoctors->size() && listY < 680.f; i++)
    {
        Doctor& doc = (*allDoctors)[i];
        drawCard(30.f, listY, 730.f, 42.f, i % 2 == 0 ? Pal::card() : Pal::panel());
        char doctorIdStr[10], feeStr[20];
        MyStr::intToStr(doc.getId(), doctorIdStr);
        MyStr::floatToStr(doc.getFee(), feeStr, 2);
        char doctorInfoLine[300];
        MyStr::copy(doctorInfoLine, "ID:");
        MyStr::concat(doctorInfoLine, doctorIdStr);
        MyStr::concat(doctorInfoLine, "  ");
        MyStr::concat(doctorInfoLine, doc.getName());
        MyStr::concat(doctorInfoLine, "  |  ");
        MyStr::concat(doctorInfoLine, doc.getSpecialization());
        MyStr::concat(doctorInfoLine, "  |  PKR ");
        MyStr::concat(doctorInfoLine, feeStr);
        drawText(doctorInfoLine, 46.f, listY + 12.f, 14, Pal::textPri());
        listY += 46.f;
    }

    drawCard(790.f, 76.f, 390.f, 650.f);
    drawText("Remove Doctor", 806.f, 90.f, 16, Pal::textPri(), true);
    drawSeparator(114.f, 800.f, 1170.f);
    drawTextBox(formBoxes[0]);

    Button removeButton, backButton;
    makeButton(removeButton, 800.f, 224.f, 360.f, 44.f, "Remove Doctor", Pal::danger());
    makeOutlineButton(backButton, 800.f, 280.f, 170.f, 38.f, "Back", Pal::textSec());

    if (drawButton(backButton))
    {
        currentScreen = Screen::AdminMenu;
        resetForm();
        return;
    }

    if (drawButton(removeButton))
    {
        if (!Validator::isValidId(formBoxes[0].content))
        {
            setMessage("Invalid ID.", false);
            return;
        }
        int enteredDoctorId = MyStr::toInt(formBoxes[0].content);

        // Block removal if the doctor still has pending appointments
        Storage<Appointment>* allAppointments = sys->getAppointments();
        for (int i = 0; i < allAppointments->size(); i++)
        {
            if ((*allAppointments)[i].getDoctorId() == enteredDoctorId
                && MyStr::equals((*allAppointments)[i].getStatus(), "pending"))
            {
                setMessage("Cannot remove: doctor has pending appointments.", false);
                return;
            }
        }

        if (sys->getDoctors()->removeById(enteredDoctorId))
        {
            FileHandler::saveDoctors(sys->getDoctors());
            setMessage("Doctor removed.", true);
        }
        else
        {
            setMessage("Doctor not found.", false);
        }
    }
}

 
//  ADMIN – VIEW PATIENTS
 

void GUI::renderAdminViewPatients()
{
    drawTopBar("All Patients");
    drawCard(20.f, 76.f, 1160.f, 650.f);

    // Column header
    drawCard(30.f, 84.f, 1140.f, 28.f, Pal::hdr());
    drawText("ID", 46.f, 89.f, 12, Pal::textSec());
    drawText("Name", 110.f, 89.f, 12, Pal::textSec());
    drawText("Age", 340.f, 89.f, 12, Pal::textSec());
    drawText("Gender", 400.f, 89.f, 12, Pal::textSec());
    drawText("Contact", 470.f, 89.f, 12, Pal::textSec());
    drawText("Balance", 640.f, 89.f, 12, Pal::textSec());
    drawText("Unpaid", 780.f, 89.f, 12, Pal::textSec());

    Storage<Patient>* allPatients = sys->getPatients();
    Storage<Bill>* allBills = sys->getBills();

    float topY = 116.f;
    float rowHeight = 36.f;
    float visibleH = 580.f;
    float maxScroll = std::max(0.f, static_cast<float>(allPatients->size()) * rowHeight - visibleH);
    if (listScrollY > maxScroll)
    {
        listScrollY = maxScroll;
    }

    // Scrollable clipped view for the patient rows
    sf::View listView(sf::Vector2f(570.f, visibleH / 2.f + listScrollY),
        sf::Vector2f(1140.f, visibleH));
    listView.setViewport(sf::FloatRect(
        sf::Vector2f(30.f / 1200.f, topY / 780.f),
        sf::Vector2f(1140.f / 1200.f, visibleH / 780.f)));
    window.setView(listView);

    for (int i = 0; i < allPatients->size(); i++)
    {
        Patient& pat = (*allPatients)[i];
        int      unpaidCount = 0;
        for (int j = 0; j < allBills->size(); j++)
        {
            if ((*allBills)[j].getPatientId() == pat.getId()
                && MyStr::equals((*allBills)[j].getStatus(), "unpaid"))
            {
                unpaidCount++;
            }
        }
        float rowY = static_cast<float>(i) * rowHeight;
        if (i % 2 == 0)
        {
            sf::RectangleShape altRow(sf::Vector2f(1140.f, rowHeight));
            altRow.setPosition(sf::Vector2f(0.f, rowY));
            altRow.setFillColor(withAlpha(Pal::card(), 80));
            window.draw(altRow);
        }
        char idStr[10] = {}, ageStr[10] = {}, balanceStr[20] = {}, unpaidStr[10] = {};
        char genderStr[2] = { pat.getGender(), '\0' };
        MyStr::intToStr(pat.getId(), idStr);
        MyStr::intToStr(pat.getAge(), ageStr);
        MyStr::floatToStr(pat.getBalance(), balanceStr, 2);
        MyStr::intToStr(unpaidCount, unpaidStr);
        drawText(idStr, 16.f, rowY + 8.f, 13, Pal::textSec());
        drawText(pat.getName(), 80.f, rowY + 8.f, 13, Pal::textPri());
        drawText(ageStr, 310.f, rowY + 8.f, 13, Pal::textSec());
        drawText(genderStr, 370.f, rowY + 8.f, 13, Pal::textSec());
        drawText(pat.getContact(), 440.f, rowY + 8.f, 13, Pal::textSec());
        drawText(balanceStr, 610.f, rowY + 8.f, 13, Pal::success());
        drawText(unpaidStr, 750.f, rowY + 8.f, 13,
            unpaidCount > 0 ? Pal::danger() : Pal::textSec());
    }
    window.setView(window.getDefaultView());

    Button backButton;
    makeOutlineButton(backButton, 30.f, 706.f, 150.f, 38.f, "Back", Pal::textSec());
    if (drawButton(backButton))
    {
        currentScreen = Screen::AdminMenu;
        resetForm();
    }
}

 
//  ADMIN – VIEW DOCTORS
 

void GUI::renderAdminViewDoctors()
{
    drawTopBar("All Doctors");
    drawCard(20.f, 76.f, 1160.f, 650.f);

    // Column header
    drawCard(30.f, 84.f, 1140.f, 28.f, Pal::hdr());
    drawText("ID", 46.f, 89.f, 12, Pal::textSec());
    drawText("Name", 110.f, 89.f, 12, Pal::textSec());
    drawText("Specialization", 340.f, 89.f, 12, Pal::textSec());
    drawText("Contact", 580.f, 89.f, 12, Pal::textSec());
    drawText("Fee", 750.f, 89.f, 12, Pal::textSec());

    Storage<Doctor>* allDoctors = sys->getDoctors();

    float topY = 116.f;
    float rowHeight = 38.f;
    float visibleH = 580.f;
    float maxScroll = std::max(0.f, static_cast<float>(allDoctors->size()) * rowHeight - visibleH);
    if (listScrollY > maxScroll)
    {
        listScrollY = maxScroll;
    }

    sf::View listView(sf::Vector2f(570.f, visibleH / 2.f + listScrollY),
        sf::Vector2f(1140.f, visibleH));
    listView.setViewport(sf::FloatRect(
        sf::Vector2f(30.f / 1200.f, topY / 780.f),
        sf::Vector2f(1140.f / 1200.f, visibleH / 780.f)));
    window.setView(listView);

    for (int i = 0; i < allDoctors->size(); i++)
    {
        Doctor& doc = (*allDoctors)[i];
        float   rowY = static_cast<float>(i) * rowHeight;
        if (i % 2 == 0)
        {
            sf::RectangleShape altRow(sf::Vector2f(1140.f, rowHeight));
            altRow.setPosition(sf::Vector2f(0.f, rowY));
            altRow.setFillColor(withAlpha(Pal::card(), 80));
            window.draw(altRow);
        }
        char idStr[10] = {}, feeStr[20] = {};
        MyStr::intToStr(doc.getId(), idStr);
        MyStr::floatToStr(doc.getFee(), feeStr, 2);
        drawText(idStr, 16.f, rowY + 10.f, 13, Pal::textSec());
        drawText(doc.getName(), 80.f, rowY + 10.f, 14, Pal::textPri());
        drawText(doc.getSpecialization(), 310.f, rowY + 10.f, 13, Pal::accent());
        drawText(doc.getContact(), 550.f, rowY + 10.f, 13, Pal::textSec());
        char feeLabel[30] = "PKR ";
        MyStr::concat(feeLabel, feeStr);
        drawText(feeLabel, 720.f, rowY + 10.f, 13, Pal::success());
    }
    window.setView(window.getDefaultView());

    Button backButton;
    makeOutlineButton(backButton, 30.f, 706.f, 150.f, 38.f, "Back", Pal::textSec());
    if (drawButton(backButton))
    {
        currentScreen = Screen::AdminMenu;
        resetForm();
    }
}

 
//  ADMIN – VIEW APPOINTMENTS
 

void GUI::renderAdminViewAppts()
{
    drawTopBar("All Appointments");
    drawCard(20.f, 76.f, 1160.f, 650.f);

    // Column header
    drawCard(30.f, 84.f, 1140.f, 28.f, Pal::hdr());
    drawText("ID", 46.f, 89.f, 12, Pal::textSec());
    drawText("Patient", 110.f, 89.f, 12, Pal::textSec());
    drawText("Doctor", 300.f, 89.f, 12, Pal::textSec());
    drawText("Date", 480.f, 89.f, 12, Pal::textSec());
    drawText("Time", 640.f, 89.f, 12, Pal::textSec());
    drawText("Status", 760.f, 89.f, 12, Pal::textSec());

    Storage<Appointment>* allAppointments = sys->getAppointments();
    int apptCount = allAppointments->size();
    int sortedIndices[200];
    for (int i = 0; i < apptCount; i++)
    {
        sortedIndices[i] = i;
    }

    // Sort descending by date (most recent first)
    for (int i = 0; i < apptCount - 1; i++)
    {
        for (int j = 0; j < apptCount - i - 1; j++)
        {
            const char* dateA = (*allAppointments)[sortedIndices[j]].getDate();
            const char* dateB = (*allAppointments)[sortedIndices[j + 1]].getDate();
            long keyA = static_cast<long>((dateA[6] - '0') * 1000 + (dateA[7] - '0') * 100 + (dateA[8] - '0') * 10 + (dateA[9] - '0')) * 10000L
                + ((dateA[3] - '0') * 10 + (dateA[4] - '0')) * 100 + (dateA[0] - '0') * 10 + (dateA[1] - '0');
            long keyB = static_cast<long>((dateB[6] - '0') * 1000 + (dateB[7] - '0') * 100 + (dateB[8] - '0') * 10 + (dateB[9] - '0')) * 10000L
                + ((dateB[3] - '0') * 10 + (dateB[4] - '0')) * 100 + (dateB[0] - '0') * 10 + (dateB[1] - '0');
            if (keyA < keyB)
            {
                int tmp = sortedIndices[j];
                sortedIndices[j] = sortedIndices[j + 1];
                sortedIndices[j + 1] = tmp;
            }
        }
    }

    float topY = 116.f;
    float rowHeight = 36.f;
    float visibleH = 580.f;
    float maxScroll = std::max(0.f, static_cast<float>(apptCount) * rowHeight - visibleH);
    if (listScrollY > maxScroll)
    {
        listScrollY = maxScroll;
    }

    sf::View listView(sf::Vector2f(570.f, visibleH / 2.f + listScrollY),
        sf::Vector2f(1140.f, visibleH));
    listView.setViewport(sf::FloatRect(
        sf::Vector2f(30.f / 1200.f, topY / 780.f),
        sf::Vector2f(1140.f / 1200.f, visibleH / 780.f)));
    window.setView(listView);

    for (int i = 0; i < apptCount; i++)
    {
        Appointment& appt = (*allAppointments)[sortedIndices[i]];
        Patient* apptPat = sys->getPatients()->findById(appt.getPatientId());
        Doctor* apptDoc = sys->getDoctors()->findById(appt.getDoctorId());
        float        rowY = static_cast<float>(i) * rowHeight;
        if (i % 2 == 0)
        {
            sf::RectangleShape altRow(sf::Vector2f(1140.f, rowHeight));
            altRow.setPosition(sf::Vector2f(0.f, rowY));
            altRow.setFillColor(withAlpha(Pal::card(), 80));
            window.draw(altRow);
        }
        char apptIdStr[10] = {};
        MyStr::intToStr(appt.getId(), apptIdStr);
        drawText(apptIdStr, 16.f, rowY + 8.f, 13, Pal::textSec());
        drawText(apptPat ? apptPat->getName() : "?", 80.f, rowY + 8.f, 13, Pal::textPri());
        drawText(apptDoc ? apptDoc->getName() : "?", 270.f, rowY + 8.f, 13, Pal::textPri());
        drawText(appt.getDate(), 450.f, rowY + 8.f, 13, Pal::textSec());
        drawText(appt.getTimeSlot(), 610.f, rowY + 8.f, 13, Pal::textSec());

        sf::Color statusColor = MyStr::equals(appt.getStatus(), "completed") ? Pal::success()
            : MyStr::equals(appt.getStatus(), "pending") ? Pal::warning()
            : Pal::danger();
        drawText(appt.getStatus(), 730.f, rowY + 8.f, 13, statusColor);
    }
    window.setView(window.getDefaultView());

    Button backButton;
    makeOutlineButton(backButton, 30.f, 706.f, 150.f, 38.f, "Back", Pal::textSec());
    if (drawButton(backButton))
    {
        currentScreen = Screen::AdminMenu;
        resetForm();
    }
}

 
//  ADMIN – UNPAID BILLS
 

void GUI::renderAdminUnpaidBills()
{
    drawTopBar("Unpaid Bills");
    drawCard(20.f, 76.f, 1160.f, 650.f);

    // Column header
    drawCard(30.f, 84.f, 1140.f, 28.f, Pal::hdr());
    drawText("Bill", 46.f, 89.f, 12, Pal::textSec());
    drawText("Patient", 120.f, 89.f, 12, Pal::textSec());
    drawText("Amount", 400.f, 89.f, 12, Pal::textSec());
    drawText("Date", 570.f, 89.f, 12, Pal::textSec());
    drawText("Status", 760.f, 89.f, 12, Pal::textSec());

    Storage<Bill>* allBills = sys->getBills();
    float          listY = 116.f;
    int            unpaidCount = 0;

    for (int i = 0; i < allBills->size() && listY < 660.f; i++)
    {
        Bill& bill = (*allBills)[i];
        if (!MyStr::equals(bill.getStatus(), "unpaid"))
        {
            continue;
        }
        Patient* billPatient = sys->getPatients()->findById(bill.getPatientId());
        drawCard(30.f, listY, 1140.f, 38.f, unpaidCount % 2 == 0 ? Pal::card() : Pal::panel());

        char billIdStr[10], amountStr[20];
        MyStr::intToStr(bill.getId(), billIdStr);
        MyStr::floatToStr(bill.getAmount(), amountStr, 2);
        drawText(billIdStr, 46.f, listY + 10.f, 13, Pal::textSec());
        drawText(billPatient ? billPatient->getName() : "?", 90.f, listY + 10.f, 13, Pal::textPri());

        char amountLabel[30] = "PKR ";
        MyStr::concat(amountLabel, amountStr);
        drawText(amountLabel, 370.f, listY + 10.f, 13, Pal::danger());
        drawText(bill.getDate(), 540.f, listY + 10.f, 13, Pal::textSec());

        // Mark overdue if unpaid for more than 7 days
        bool isOverdue = sys->daysBetween(bill.getDate()) > 7;
        drawBadge(isOverdue ? "OVERDUE" : "UNPAID", 730.f, listY + 9.f,
            isOverdue ? Pal::danger() : Pal::warning());
        listY += 42.f;
        unpaidCount++;
    }
    if (unpaidCount == 0)
    {
        drawText("No unpaid bills.", 46.f, 140.f, 15, Pal::textSec());
    }

    Button backButton;
    makeOutlineButton(backButton, 30.f, 706.f, 150.f, 38.f, "Back", Pal::textSec());
    if (drawButton(backButton))
    {
        currentScreen = Screen::AdminMenu;
        resetForm();
    }
}

 
//  ADMIN – DISCHARGE PATIENT
 

void GUI::renderAdminDischarge()
{
    drawTopBar("Discharge Patient");
    drawCard(350.f, 100.f, 500.f, 440.f);
    drawText("Discharge Patient", 370.f, 118.f, 17, Pal::textPri(), true);
    drawSeparator(144.f, 360.f, 840.f);
    drawText("Requirements: No unpaid bills, no pending appointments.",
        370.f, 154.f, 12, Pal::textSec());

    if (formBoxCount == 0)
    {
        makeTextBox(formBoxes[0], 370.f, 190.f, 460.f, 44.f,
            "Patient ID", "Enter patient numeric ID");
        formBoxCount = 1;
    }
    drawTextBox(formBoxes[0]);

    Button dischargeButton, backButton;
    makeButton(dischargeButton, 370.f, 260.f, 460.f, 44.f, "Discharge Patient", Pal::danger());
    makeOutlineButton(backButton, 370.f, 316.f, 200.f, 38.f, "Back", Pal::textSec());

    if (drawButton(backButton))
    {
        currentScreen = Screen::AdminMenu;
        resetForm();
        return;
    }

    if (drawButton(dischargeButton))
    {
        if (!Validator::isValidId(formBoxes[0].content))
        {
            setMessage("Invalid ID.", false);
            return;
        }
        int      enteredPatientId = MyStr::toInt(formBoxes[0].content);
        Patient* dischargePatient = sys->getPatients()->findById(enteredPatientId);
        if (!dischargePatient)
        {
            setMessage("Patient not found.", false);
            return;
        }

        // Block discharge if any bill is still unpaid
        Storage<Bill>* allBills = sys->getBills();
        for (int i = 0; i < allBills->size(); i++)
        {
            if ((*allBills)[i].getPatientId() == enteredPatientId
                && MyStr::equals((*allBills)[i].getStatus(), "unpaid"))
            {
                setMessage("Cannot discharge: patient has unpaid bills.", false);
                return;
            }
        }

        // Block discharge if any appointment is still pending
        Storage<Appointment>* allAppointments = sys->getAppointments();
        for (int i = 0; i < allAppointments->size(); i++)
        {
            if ((*allAppointments)[i].getPatientId() == enteredPatientId
                && MyStr::equals((*allAppointments)[i].getStatus(), "pending"))
            {
                setMessage("Cannot discharge: patient has pending appointments.", false);
                return;
            }
        }

        // Archive the patient record before removing from active store
        FileHandler::appendDischargedPatient(dischargePatient);
        sys->getPatients()->removeById(enteredPatientId);

        // Remove all bills and appointments associated with this patient
        for (int i = 0; i < allBills->size();)
        {
            if ((*allBills)[i].getPatientId() == enteredPatientId)
            {
                allBills->removeById((*allBills)[i].getId());
            }
            else
            {
                i++;
            }
        }
        for (int i = 0; i < allAppointments->size();)
        {
            if ((*allAppointments)[i].getPatientId() == enteredPatientId)
            {
                allAppointments->removeById((*allAppointments)[i].getId());
            }
            else
            {
                i++;
            }
        }

        // Remove prescriptions linked to this patient
        Storage<Prescription>* allPrescriptions = sys->getPrescriptions();
        for (int i = 0; i < allPrescriptions->size();)
        {
            if ((*allPrescriptions)[i].getPatientId() == enteredPatientId)
            {
                allPrescriptions->removeById((*allPrescriptions)[i].getId());
            }
            else
            {
                i++;
            }
        }

        // Persist all modified stores
        FileHandler::savePatients(sys->getPatients());
        FileHandler::saveBills(allBills);
        FileHandler::saveAppointments(allAppointments);
        FileHandler::savePrescriptions(allPrescriptions);
        setMessage("Patient discharged and archived successfully.", true);

        formBoxes[0].len = 0;
        formBoxes[0].content[0] = '\0';
    }
}

 
//  ADMIN – SECURITY LOG
 

void GUI::renderAdminSecurityLog()
{
    drawTopBar("Security Log");
    drawCard(20.f, 76.f, 1160.f, 650.f);

    // Column header
    drawCard(30.f, 84.f, 1140.f, 28.f, Pal::hdr());
    drawText("Timestamp", 46.f, 89.f, 12, Pal::textSec());
    drawText("Role", 280.f, 89.f, 12, Pal::textSec());
    drawText("ID Entered", 420.f, 89.f, 12, Pal::textSec());
    drawText("Result", 620.f, 89.f, 12, Pal::textSec());

    char logLines[100][256];
    int  lineCount = 0;
    FileHandler::readSecurityLog(logLines, &lineCount, 100);
    if (lineCount == 0)
    {
        drawText("No events logged.", 46.f, 140.f, 15, Pal::textSec());
    }

    float rowY = 116.f;
    for (int i = 0; i < lineCount && rowY < 660.f; i++)
    {
        drawCard(30.f, rowY, 1140.f, 32.f, i % 2 == 0 ? Pal::card() : Pal::panel());

        // Each log line is: timestamp,role,enteredId,result
        char logTokens[4][100] = {};
        int  tokenCount = MyStr::split(logLines[i], ',', logTokens, 4);
        if (tokenCount >= 4)
        {
            drawText(logTokens[0], 46.f, rowY + 7.f, 13, Pal::textSec());
            drawText(logTokens[1], 250.f, rowY + 7.f, 13, Pal::textPri());
            drawText(logTokens[2], 390.f, rowY + 7.f, 13, Pal::textSec());
            sf::Color resultColor = MyStr::equals(logTokens[3], "SUCCESS")
                ? Pal::success() : Pal::danger();
            drawBadge(logTokens[3], 590.f, rowY + 6.f, resultColor);
        }
        else
        {
            drawText(logLines[i], 46.f, rowY + 7.f, 13, Pal::textSec());
        }
        rowY += 36.f;
    }

    Button backButton;
    makeOutlineButton(backButton, 30.f, 706.f, 150.f, 38.f, "Back", Pal::textSec());
    if (drawButton(backButton))
    {
        currentScreen = Screen::AdminMenu;
        resetForm();
    }
}

 
//  ADMIN – DAILY REPORT
 

void GUI::renderAdminDailyReport()
{
    drawTopBar("Daily Report");

    char todayDate[11];
    sys->getTodayDate(todayDate);

    // Count today's appointments by status
    Storage<Appointment>* allAppointments = sys->getAppointments();
    int totalAppts = 0, pendingCount = 0, completedCount = 0, noShowCount = 0, cancelledCount = 0;
    for (int i = 0; i < allAppointments->size(); i++)
    {
        Appointment& appt = (*allAppointments)[i];
        if (!MyStr::equals(appt.getDate(), todayDate))
        {
            continue;
        }
        totalAppts++;
        if (MyStr::equals(appt.getStatus(), "pending")) { pendingCount++; }
        else if (MyStr::equals(appt.getStatus(), "completed")) { completedCount++; }
        else if (MyStr::equals(appt.getStatus(), "no-show")) { noShowCount++; }
        else if (MyStr::equals(appt.getStatus(), "cancelled")) { cancelledCount++; }
    }

    // Sum today's revenue from paid bills
    float          todayRevenue = 0.f;
    Storage<Bill>* allBills = sys->getBills();
    for (int i = 0; i < allBills->size(); i++)
    {
        if (MyStr::equals((*allBills)[i].getStatus(), "paid")
            && MyStr::equals((*allBills)[i].getDate(), todayDate))
        {
            todayRevenue += (*allBills)[i].getAmount();
        }
    }

    // Stat cards row: Total, Completed, Pending, No-Show, Cancelled
    struct StatCard { const char* label; int value; sf::Color color; };
    StatCard statCards[] = {
        { "Total",     totalAppts,     Pal::accent()  },
        { "Completed", completedCount, Pal::success() },
        { "Pending",   pendingCount,   Pal::warning() },
        { "No-Show",   noShowCount,    Pal::danger()  },
        { "Cancelled", cancelledCount, Pal::textSec() }
    };
    for (int i = 0; i < 5; i++)
    {
        float cardX = 20.f + static_cast<float>(i) * 228.f;
        drawCard(cardX, 80.f, 218.f, 90.f);
        sf::RectangleShape topStripe(sf::Vector2f(218.f, 4.f));
        topStripe.setPosition(sf::Vector2f(cardX, 80.f));
        topStripe.setFillColor(statCards[i].color);
        window.draw(topStripe);
        drawText(statCards[i].label, cardX + 12.f, 94.f, 12, Pal::textSec());
        char valueStr[10] = {};
        MyStr::intToStr(statCards[i].value, valueStr);
        drawText(valueStr, cardX + 12.f, 114.f, 28, statCards[i].color, true);
    }

    // Revenue summary card
    drawCard(20.f, 180.f, 340.f, 70.f);
    char revenueLabelStr[30] = "PKR ";
    char revenueAmtStr[20] = {};
    MyStr::floatToStr(todayRevenue, revenueAmtStr, 2);
    MyStr::concat(revenueLabelStr, revenueAmtStr);
    drawText("Revenue Today", 36.f, 192.f, 12, Pal::textSec());
    drawText(revenueLabelStr, 36.f, 210.f, 20, Pal::success(), true);

    // Outstanding bills panel (bottom-left)
    drawCard(20.f, 264.f, 560.f, 420.f);
    drawText("Outstanding Bills", 36.f, 278.f, 14, Pal::textPri(), true);
    drawSeparator(300.f, 30.f, 570.f);

    float          outstandingRowY = 308.f;
    int            outstandingCount = 0;
    Storage<Patient>* allPatients = sys->getPatients();
    for (int i = 0; i < allPatients->size() && outstandingRowY < 640.f; i++)
    {
        float totalOwed = 0.f;
        for (int j = 0; j < allBills->size(); j++)
        {
            if ((*allBills)[j].getPatientId() == (*allPatients)[i].getId()
                && MyStr::equals((*allBills)[j].getStatus(), "unpaid"))
            {
                totalOwed += (*allBills)[j].getAmount();
            }
        }
        if (totalOwed > 0.f)
        {
            drawCard(30.f, outstandingRowY, 540.f, 34.f,
                outstandingCount % 2 == 0 ? Pal::card() : Pal::panel());
            drawText((*allPatients)[i].getName(), 46.f, outstandingRowY + 8.f, 13, Pal::textPri());
            char owedLabel[30] = "PKR ";
            char owedAmtStr[20] = {};
            MyStr::floatToStr(totalOwed, owedAmtStr, 2);
            MyStr::concat(owedLabel, owedAmtStr);
            drawText(owedLabel, 440.f, outstandingRowY + 8.f, 13, Pal::danger());
            outstandingRowY += 38.f;
            outstandingCount++;
        }
    }
    if (outstandingCount == 0)
    {
        drawText("None", 46.f, 318.f, 13, Pal::textSec());
    }

    // Doctor summary panel (bottom-right): completed / pending / no-show counts per doctor
    drawCard(594.f, 264.f, 586.f, 420.f);
    drawText("Doctor Summary", 610.f, 278.f, 14, Pal::textPri(), true);
    drawSeparator(300.f, 604.f, 1170.f);

    Storage<Doctor>* allDoctors = sys->getDoctors();
    float            doctorRowY = 308.f;
    for (int i = 0; i < allDoctors->size() && doctorRowY < 640.f; i++)
    {
        int docCompletedCount = 0, docPendingCount = 0, docNoShowCount = 0;
        for (int j = 0; j < allAppointments->size(); j++)
        {
            Appointment& appt = (*allAppointments)[j];
            if (appt.getDoctorId() != (*allDoctors)[i].getId()
                || !MyStr::equals(appt.getDate(), todayDate))
            {
                continue;
            }
            if (MyStr::equals(appt.getStatus(), "completed")) { docCompletedCount++; }
            else if (MyStr::equals(appt.getStatus(), "pending")) { docPendingCount++; }
            else if (MyStr::equals(appt.getStatus(), "no-show")) { docNoShowCount++; }
        }
        drawCard(604.f, doctorRowY, 566.f, 34.f, i % 2 == 0 ? Pal::card() : Pal::panel());
        drawText((*allDoctors)[i].getName(), 620.f, doctorRowY + 8.f, 13, Pal::textPri());

        char summaryStr[60] = "C:";
        char numBuf[10] = {};
        MyStr::intToStr(docCompletedCount, numBuf);
        MyStr::concat(summaryStr, numBuf);
        MyStr::concat(summaryStr, "  P:");
        MyStr::intToStr(docPendingCount, numBuf); 
        MyStr::concat(summaryStr, numBuf);
        MyStr::concat(summaryStr, "  NS:");
        MyStr::intToStr(docNoShowCount, numBuf); 
        MyStr::concat(summaryStr, numBuf);
        drawText(summaryStr, 900.f, doctorRowY + 8.f, 12, Pal::textSec());
        doctorRowY += 38.f;
    }
    if (allDoctors->size() == 0)
    {
        drawText("No doctors.", 620.f, 318.f, 13, Pal::textSec());
    }

    Button backButton;
    makeOutlineButton(backButton, 30.f, 706.f, 150.f, 38.f, "Back", Pal::textSec());
    if (drawButton(backButton))
    {
        currentScreen = Screen::AdminMenu;
        resetForm();
    }
}