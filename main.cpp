#include <iostream>
#include <thread>
#include <conio.h>  // For _kbhit() and _getch() in Windows. For Linux, use termios.h and other POSIX libraries.
#include "StorageHandler.h"
#include "ConfigReader.h"

int main() {
    StorageHandler storageHandler(PORT);
    ConfigReader configReader("config.json");

    std::thread storage_thread(&StorageHandler::start, &storageHandler);
    std::vector<Camera> cameras = configReader.start();

    bool exit_program = false;
    while (!exit_program) {
        if (_kbhit()) {
            char ch = _getch();
            if (ch == 27) {  // ESC key
                std::cout << "Terminate session? (Y/N): ";
                char confirm = _getch();
                if (confirm == 'Y' || confirm == 'y') {
                    configReader.stop();
                    storageHandler.stop();
                    if (storage_thread.joinable()) {
                        storage_thread.join();
                    }
                    exit_program = true;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
