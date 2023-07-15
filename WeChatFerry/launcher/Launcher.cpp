#include "Launcher.h"

using namespace launcher;

[STAThreadAttribute]

int main() {
    Launcher launcher;
    launcher.ShowDialog();
    return 0;
}
