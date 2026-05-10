# Hospital Management System

## Overview

The Hospital Management System is a desktop application developed in C++ using SFML. The project is designed to manage hospital operations efficiently through a graphical user interface. It provides features for handling patient records, doctor information, appointments, billing, and hospital reports.

The system combines frontend graphical components with backend data management logic to create a complete management solution.

---

# Features

## Patient Management

* Add new patients
* Update patient information
* Delete patient records
* Search patient details
* View all registered patients

## Doctor Management

* Add doctor records
* Update doctor information
* View doctor list
* Assign departments

## Appointment Management

* Book appointments
* Update appointment schedules
* Cancel appointments
* View appointment history

## Billing System

* Generate patient bills
* Store billing records
* Display payment information

## User Interface

* Interactive graphical interface using SFML
* Sidebar navigation menu
* Buttons and input fields
* Multiple management screens
* Dashboard system

---

# Technologies Used

* C++
* SFML 3
* Object-Oriented Programming (OOP)
* File Handling

---

# Project Structure

```text
HospitalManagementSystem/
│
├── main.cpp
├── Button.cpp
├── Button.h
├── Patient.cpp
├── Patient.h
├── Doctor.cpp
├── Doctor.h
├── Appointment.cpp
├── Appointment.h
├── Billing.cpp
├── Billing.h
├── FileManager.cpp
├── FileManager.h
├── assets/
│   ├── fonts/
│   ├── images/
│   └── icons/
└── README.md
```

---

# System Requirements

* Windows 10/11
* C++ Compiler
* Visual Studio 2022
* SFML 3 Library

---

# Installation

## Step 1: Install SFML

Download and install SFML from:

[https://www.sfml-dev.org/](https://www.sfml-dev.org/)

Make sure the SFML version matches your compiler version.

---

# Compile Instructions

## Using Visual Studio 2022

1. Open the project solution file.
2. Configure SFML include and library directories.
3. Link the required SFML libraries.
4. Select Debug or Release mode.
5. Build the project using:

```text
Build -> Build Solution
```

## Required SFML Libraries

```text
sfml-graphics
sfml-window
sfml-system
```

If using Debug mode, link:

```text
sfml-graphics-d.lib
sfml-window-d.lib
sfml-system-d.lib
```

If using Release mode, link:

```text
sfml-graphics.lib
sfml-window.lib
sfml-system.lib
```

---

# Run Instructions

1. Build the project successfully.
2. Run the generated executable file.
3. The Hospital Management System window will open.
4. Use the graphical menu to navigate through the system.

You can also run the project directly from Visual Studio using:

```text
Ctrl + F5
```

---

# How to Run

1. Build the project.
2. Run the executable file.
3. Use the graphical menu to access different modules.

---

# Backend Functionality

The backend logic handles:

* Data storage
* Record management
* File operations
* Searching algorithms
* Appointment processing
* Billing calculations

The frontend is developed using SFML while the backend is implemented using standard C++ programming concepts.

---

# Concepts Used

* Classes and Objects
* Inheritance
* Encapsulation
* File Handling
* Event Handling
* GUI Programming
* Modular Programming

---

# Future Improvements

* Database integration
* Login authentication system
* Online appointment booking
* Advanced reporting system
* Animated user interface
* Cloud storage support

---

# Screens Included

* Login Screen
* Dashboard
* Patient Management
* Doctor Management
* Appointment Management
* Billing System
* Reports Section

---

# Developed By

Muhammad Zain Ul Abedin

---

# License

This project is developed for educational purposes.
