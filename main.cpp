#include "HospitalSystem.h"
#include "GUI.h"
#include "Exceptions.h"
#include <iostream>

// Initialize the hospital system (loads data from files)
static HospitalSystem* initSystem() {
    HospitalSystem* sys = new HospitalSystem();
    sys->loadAll();
    return sys;
}

// Run the GUI session
static void runSession(HospitalSystem* sys) {
    GUI gui(sys);
    gui.run();
}

// Cleanup resources
static void cleanup(HospitalSystem* sys) {
    delete sys;
}

int main() {
    try {
        HospitalSystem* sys = initSystem();
        runSession(sys);
        cleanup(sys);
    }
    catch (HospitalException& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}