#ifndef CONFIGREADER_H
#define CONFIGREADER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <nlohmann/json.hpp>  // You need to install the nlohmann/json library
#include "Camera.h"

class ConfigReader {
public:
    ConfigReader(const std::string& config_file);
    std::vector<Camera> start();
    void stop();

private:
    std::string config_file;
    std::vector<std::string> refresh_snap_times;
    std::vector<Camera> active_cameras;
    std::mutex config_mutex;
    std::atomic<bool> running;

    void read_config();
    void initialize_refresh_times();
    void reinitialize_refresh_times();
    void update_cameras(const nlohmann::json& new_config);
};

#endif // CONFIGREADER_H
