#include "ConfigReader.h"

ConfigReader::ConfigReader(const std::string& config_file) : config_file(config_file), running(false) {
    read_config();
}

void ConfigReader::read_config() {
    std::ifstream file(config_file);
    nlohmann::json config_json;
    file >> config_json;

    {
        std::lock_guard<std::mutex> lock(config_mutex);
        refresh_snap_times = config_json["refresh_times"].get<std::vector<std::string>>();
        active_cameras.clear();
        for (const auto& cam_config : config_json["cameras"]) {
            Camera cam(
                cam_config["id"],
                cam_config["ip"],
                cam_config["user"],
                cam_config["password"],
                cam_config["resolution"]["width"],
                cam_config["resolution"]["height"],
                cam_config["snap_times"].get<std::vector<std::string>>(),
                cam_config["pictures_per_day"]
            );
            active_cameras.push_back(cam);
        }
    }
}

void ConfigReader::initialize_refresh_times() {
    running = true;
    for (const auto& time_str : refresh_snap_times) {
        std::thread([this, time_str]() {
            while (running) {
                auto now = std::chrono::system_clock::now();
                std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
                std::tm now_tm = *std::localtime(&now_time_t);

                char current_time[6];
                std::strftime(current_time, sizeof(current_time), "%H:%M", &now_tm);

                if (time_str == current_time) {
                    std::lock_guard<std::mutex> lock(config_mutex);
                    read_config();
                    std::this_thread::sleep_for(std::chrono::minutes(1)); // Avoid multiple refreshes in the same minute
                }

                std::this_thread::sleep_for(std::chrono::seconds(30)); // Check every 30 seconds
            }
        }).detach();
    }
}

void ConfigReader::reinitialize_refresh_times() {
    running = false;
    std::this_thread::sleep_for(std::chrono::seconds(31)); // Ensure existing threads exit

    initialize_refresh_times();
}

void ConfigReader::update_cameras(const nlohmann::json& new_config) {
    std::lock_guard<std::mutex> lock(config_mutex);
    // Find and update existing cameras or remove them if not in the new config
    for (auto it = active_cameras.begin(); it != active_cameras.end(); ) {
        bool found = false;
        for (const auto& cam_config : new_config["cameras"]) {
            if (cam_config["id"] == it->get_id()) {
                found = true;
                it->set_ip(cam_config["ip"]);
                it->set_user(cam_config["user"]);
                it->set_password(cam_config["password"]);
                it->set_resolution(cam_config["resolution"]["width"], cam_config["resolution"]["height"]);
                it->set_snap_times(cam_config["snap_times"].get<std::vector<std::string>>());
                break;
            }
        }
        if (!found) {
            it->stop();
            it = active_cameras.erase(it);
        } else {
            ++it;
        }
    }
    // Add new cameras
    for (const auto& cam_config : new_config["cameras"]) {
        bool found = false;
        for (const auto& cam : active_cameras) {
            if (cam_config["id"] == cam.get_id()) {
                found = true;
                break;
            }
        }
        if (!found) {
            Camera cam(
                cam_config["id"],
                cam_config["ip"],
                cam_config["user"],
                cam_config["password"],
                cam_config["resolution"]["width"],
                cam_config["resolution"]["height"],
                cam_config["snap_times"].get<std::vector<std::string>>(),
                cam_config["pictures_per_day"]
            );
            active_cameras.push_back(cam);
            cam.start(); // Start the newly added camera
        }
    }
}

std::vector<Camera> ConfigReader::start() {
    initialize_refresh_times();
    for (auto& cam : active_cameras) {
        cam.start();
    }
    return active_cameras;
}

void ConfigReader::stop() {
    running = false;
    std::this_thread::sleep_for(std::chrono::seconds(31)); // Ensure existing threads exit
    for (auto& cam : active_cameras) {
        cam.stop();
    }
}

// int main() {
//     try {
//         ConfigReader configReader("config.json");
//         std::vector<Camera> cameras = configReader.start();

//         // The main thread can be used to perform other tasks or just keep running
//         std::this_thread::sleep_for(std::chrono::hours(24));
        
//         // Stopping all cameras and config reader
//         configReader.stop();
//     } catch (const std::exception& e) {
//         std::cerr << "Error: " << e.what() << std::endl;
//         return EXIT_FAILURE;
//     }

//     return EXIT_SUCCESS;
// }
