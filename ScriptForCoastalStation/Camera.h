#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <chrono>
#include <thread>
#include <curl/curl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <mutex>
#include <atomic>

#define BUFFER_SIZE 1024

class Camera {
public:
    Camera(const std::string& id, const std::string& ip, const std::string& user, const std::string& password,
           int width, int height, const std::vector<std::string>& snap_times, int pictures_per_day);
    void start();
    void stop();

    // Getters
    std::string get_id() const;
    std::string get_ip() const;
    std::string get_user() const;
    std::string get_password() const;
    std::pair<int, int> get_resolution() const;
    std::vector<std::string> get_snap_times() const;

    // Setters
    void set_id(const std::string& new_id);
    void set_ip(const std::string& new_ip);
    void set_user(const std::string& new_user);
    void set_password(const std::string& new_password);
    void set_resolution(int new_width, int new_height);
    void set_snap_times(const std::vector<std::string>& new_snap_times);

private:
    std::string id;
    std::string ip;
    std::string user;
    std::string password;
    int width;
    int height;
    std::vector<std::string> snap_times;
    int pictures_per_day;
    int client_socket;
    std::mutex snap_times_mutex;
    std::atomic<bool> running;

    void connect_to_storage();
    void initialize_snap_times();
    void reinitialize_snap_times();
    void take_snapshot();
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    bool is_time_to_snap(const std::string& current_time);
};

#endif // CAMERA_H
