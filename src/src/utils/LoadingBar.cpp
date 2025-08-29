#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "Console.h"
#include "../core/Logo.h"

void loading_bar(int duration_seconds) {
    set_console_purple();
    std::cout << skCrypt("[*] Initializing mod loader...") << std::endl;
    set_console_magenta();

    const int total_steps = 20;
    const int step_duration = (duration_seconds * 1000) / total_steps;
    const int bar_width = 20;

    for (int i = 0; i <= total_steps; ++i) {
        float progress = i / (float)total_steps;
        int filled = static_cast<int>(progress * bar_width);
        int percent = static_cast<int>(progress * 100);

        std::string bar = "[";
        bar += std::string(filled, '#');
        bar += std::string(bar_width - filled, ' ');
        bar += "]";


        std::cout << "\r> " << bar << " " << percent << "%    " << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(step_duration));
    }


    std::cout << "\r" << std::string(80, ' ') << "\r"; 

    std::cout << "\r" << std::string(80, ' ') << "\r"; 
	system("cls");
    show_logo();
    set_console_success();
    std::cout << skCrypt("[+] Initialization complete!") << std::endl;
    reset_console();
}