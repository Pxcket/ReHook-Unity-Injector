#include <iostream>
#include "../utils/Console.h"

void show_logo() {
    set_console_purple();

    std::cout
        << skCrypt(" ___________________________________________\n")
        << skCrypt("|  _______________________________________  |\n")
        << skCrypt("| / .-----------------------------------. \\ |\n")
        << skCrypt("| | | /\\ :                        90 min| | |\n")
        << skCrypt("| | |/--\\:....................... NR [ ]| | |\n")
        << skCrypt("| | `-----------------------------------' | |\n")
        << skCrypt("| |      //-\\\\   |         |   //-\\\\      | |\n")
        << skCrypt("| |     ||( )||  |_________|  ||( )||     | |                   Written with love by Pxcket <3\n")
        << skCrypt("| |      \\\\-//   :....:....:   \\\\-//      | |                   big help from Z0mb1e :)\n")
        << skCrypt("| |       _ _ ._  _ _ .__|_ _.._  _       | |\n")
        << skCrypt("| |      (_(_)| |(_(/_|  |_(_||_)(/_      | |\n")
        << skCrypt("| |                           |           | |\n")
        << skCrypt("| `______ ____________________ ____ ______' |\n")
        << skCrypt("|        /    []             []    \\        |\n")
        << skCrypt("|       /  ()                   ()  \\       |\n")
        << skCrypt("!______/_____________________________\______!\n")
        << skCrypt("\n")
        << skCrypt("\n")
        << skCrypt("\n");

    reset_console();
}