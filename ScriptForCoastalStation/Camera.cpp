#include "Camera.h"

Camera::Camera(const std::string& id, const std::string& ip, const std::string& user, const std::string& password,
               int width, int height, const std::vector<std::string>& snap_times, int pictures_per_day)
    : id(id), ip(ip), user(user), password(password), width(width), height(height),
      snap_times(snap_times), pictures_per_day(pictures_per_day), running(false) {
    // Constructor only stores the parameters
}

// Getters
std::string Camera::get_id() const {
    return id;
}

std::string Camera::get_ip() const {
    return ip;
}

std::string Camera::get_user() const {
    return user;
}

std::string Camera::get_password() const {
    return password;
}

std::pair<int, int> Camera::get_resolution() const {
    return std::make_pair(width, height);
}

std::vector<std::string> Camera::get_snap_times() const {
    std::lock_guard<std::mutex> lock(snap_times_mutex);
    return snap_times;
}

// Setters
void Camera::set_id(const std::string& new_id) {
    std::lock_guard<std::mutex> lock(snap_times_mutex);
    id = new_id;
}

void Camera::set_ip(const std::string& new_ip) {
    std::lock_guard<std::mutex> lock(snap_times_mutex);
    ip = new_ip;
}

void Camera::set_user(const std::string& new_user) {
    std::lock_guard<std::mutex> lock(snap_times_mutex);
    user = new_user;
}

void Camera::set_password(const std::string& new_password) {
    std::lock_guard<std::mutex> lock(snap_times_mutex);
    password = new_password;
}

void Camera::set_resolution(int new_width, int new_height) {
    std::lock_guard<std::mutex> lock(snap_times_mutex);
    width = new_width;
    height = new_height;
}

void Camera::set_snap_times(const std::vector<std::string>& new_snap_times) {
    {
        std::lock_guard<std::mutex> lock(snap_times_mutex);
        snap_times = new_snap_times;
    }
    reinitialize_snap_times();
}

void Camera::connect_to_storage() {
    struct sockaddr_in address;
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        throw std::runtime_error("Socket creation error");
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) <= 0) {
        throw std::runtime_error("Invalid address/ Address not supported");
    }

    if (connect(client_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        throw std::runtime_error("Connection Failed");
    }
}

void Camera::initialize_snap_times() {
    running = true;
    for (const auto& time_str : snap_times) {
        std::thread([this, time_str]() {
            while (running) {
                auto now = std::chrono::system_clock::now();
                std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
                std::tm now_tm = *std::localtime(&now_time_t);

                char current_time[6];
                std::strftime(current_time, sizeof(current_time), "%H:%M", &now_tm);

                {
                    std::lock_guard<std::mutex> lock(snap_times_mutex);
                    if (is_time_to_snap(std::string(current_time))) {
                        take_snapshot();
                        std::this_thread::sleep_for(std::chrono::minutes(1)); // To avoid multiple snapshots at the same minute
                    }
                }

                std::this_thread::sleep_for(std::chrono::seconds(30)); // Check every 30 seconds
            }
        }).detach();
    }
}

void Camera::reinitialize_snap_times() {
    // Stop current threads by setting running to false and wait for threads to exit
    running = false;
    std::this_thread::sleep_for(std::chrono::seconds(31)); // Ensure existing threads exit

    // Initialize with new snap times
    initialize_snap_times();
}

bool Camera::is_time_to_snap(const std::string& current_time) {
    for (const auto& time_str : snap_times) {
        if (current_time == time_str) {
            return true;
        }
    }
    return false;
}

void Camera::take_snapshot() {
    std::string url = "http://" + ip + "/cgi-bin/api.cgi?cmd=Snap&channel=0&rs=Takito&user=" + user + "&password=" + password +
                      "&width=" + std::to_string(width) + "&height=" + std::to_string(height);

    CURL* curl;
    CURLcode res;
    std::vector<char> buffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            // Send id
            send(client_socket, id.c_str(), id.size(), 0);

            // Send image
            send(client_socket, buffer.data(), buffer.size(), 0);

            std::cout << "Snapshot taken and sent to storage" << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

size_t Camera::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    std::vector<char>* buffer = (std::vector<char>*)userp;
    buffer->insert(buffer->end(), (char*)contents, (char*)contents + total_size);
    return total_size;
}

void Camera::start() {
    connect_to_storage();
    initialize_snap_times();
}

void Camera::stop() {
    running = false;
    std::this_thread::sleep_for(std::chrono::seconds(31)); // Ensure existing threads exit
    close(client_socket);
}

int main() {
    try {
        Camera camera("camera1", "192.168.1.55", "admin", "jumanji3", 3840, 2160, {"08:00", "12:00", "16:00"}, 3);
        camera.start();

        // Keep the main thread running
        std::this_thread::sleep_for(std::chrono::hours(24));
        camera.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


// int main() {
//     try {
//         Camera camera("camera1", "192.168.1.55", "admin", "jumanji3", 3840, 2160, {"08:00", "12:00", "16:00"}, 3);
//         camera.start();

//         // Keep the main thread running
//         std::this_thread::sleep_for(std::chrono::hours(24));
//         camera.stop();
//     } catch (const std::exception& e) {
//         std::cerr << "Error: " << e.what() << std::endl;
//         return EXIT_FAILURE;
//     }

//     return EXIT_SUCCESS;
// }