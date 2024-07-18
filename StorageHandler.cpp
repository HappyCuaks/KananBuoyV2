#include "StorageHandler.h"

StorageHandler::StorageHandler(int port) : port(port), running(false) {}

void StorageHandler::start() {
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if (!fs::exists(STORAGE_DIR)) {
        fs::create_directories(STORAGE_DIR);
    }

    running = true;
    std::cout << "StorageHandler is running on port " << port << std::endl;

    while (running) {
        int new_socket;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            if (!running) break; // Exit if stopped
            perror("accept");
            exit(EXIT_FAILURE);
        }
        std::thread(&StorageHandler::handle_client, this, new_socket).detach();
    }
}

void StorageHandler::stop() {
    running = false;
    close(server_fd);
    std::cout << "StorageHandler has stopped." << std::endl;
}

void StorageHandler::handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    int valread = read(client_socket, buffer, BUFFER_SIZE);
    if (valread <= 0) {
        close(client_socket);
        return;
    }

    std::string sender_id(buffer, valread);
    std::string timestamp = get_current_timestamp();

    fs::path dir_path = fs::path(STORAGE_DIR) / sender_id;
    if (!fs::exists(dir_path)) {
        fs::create_directories(dir_path);
    }

    std::ofstream outfile((dir_path / (sender_id + "_" + timestamp + ".jpg")).string(), std::ios::binary);

    while ((valread = read(client_socket, buffer, BUFFER_SIZE)) > 0) {
        outfile.write(buffer, valread);
    }

    outfile.close();
    close(client_socket);

    std::cout << "Image from " << sender_id << " saved as " << sender_id + "_" + timestamp + ".jpg" << std::endl;
}

std::string StorageHandler::get_current_timestamp() {
    std::time_t now = std::time(nullptr);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", std::localtime(&now));
    return std::string(buf);
}

// int main() {
//     StorageHandler handler(PORT);
//     std::thread storage_thread(&StorageHandler::start, &handler);

//     // Keep the main thread running for demonstration purposes
//     std::this_thread::sleep_for(std::chrono::hours(24));

//     handler.stop();
//     if (storage_thread.joinable()) {
//         storage_thread.join();
//     }

//     return 0;
// }