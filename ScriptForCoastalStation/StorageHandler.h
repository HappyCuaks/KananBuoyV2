#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <string>
#include <filesystem>
#include <thread>
#include <vector>
#include <arpa/inet.h>
#include <unistd.h>
#include <atomic>

#define PORT 65432
#define BUFFER_SIZE 1024
#define STORAGE_DIR "storage"

namespace fs = std::filesystem;

class StorageHandler {
public:
    StorageHandler(int port);
    void start();
    void stop();

private:
    int server_fd;
    int port;
    std::atomic<bool> running;

    void handle_client(int client_socket);
    std::string get_current_timestamp();
};

#endif // STORAGEHANDLER_H