#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <chrono>

#define SERVICE_ID    0x1234
#define INSTANCE_ID   0x5678
#define EVENT_ID_CPU  0x43
#define EVENT_GROUP_CPU 0x01
#define EVENT_ID_MEM    0x44 //added for mem
#define EVENT_GROUP_MEM 0x02

class SomeipProvider {
public:
    SomeipProvider() {
        std::cout << "[Server] Initializing SOME/IP application..." << std::endl;
        app = vsomeip::runtime::get()->create_application("SomeipProvider");
        std::cout << "[Server] Application created." << std::endl;
    }

    // CPU usage function
    float get_cpu_usage(const std::string &path = "/proc/stat") {
        static unsigned long long last[8] = {0};
        std::ifstream file(path);
        std::string cpu;
        unsigned long long vals[8] = {0};
        file >> cpu >> vals[0] >> vals[1] >> vals[2] >> vals[3]
             >> vals[4] >> vals[5] >> vals[6] >> vals[7];
        file.close();

        unsigned long long prev_idle = last[3] + last[4];
        unsigned long long idle = vals[3] + vals[4];

        unsigned long long prev_total = 0, total = 0;
        for (int i = 0; i < 8; i++) {
            prev_total += last[i];
            total += vals[i];
            last[i] = vals[i];
        }

        unsigned long long delta_total = total - prev_total;
        unsigned long long delta_idle = idle - prev_idle;

        if (delta_total == 0) return 0.0f;
        return 100.0f * (delta_total - delta_idle) / delta_total;
    }


    std::string get_mem_usage(const std::string &path = "/host_proc/meminfo") {
    std::ifstream file(path);
    if (!file.is_open())
        return "Error: Could not read /host_proc/meminfo";

    std::string key;
    unsigned long long memTotal = 0, memAvailable = 0;

    while (file >> key) {
        if (key == "MemTotal:") {
            file >> memTotal;
        } else if (key == "MemAvailable:") {
            file >> memAvailable;
        }
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    file.close();

    unsigned long long memUsed = memTotal - memAvailable;

    return "Mem Used: " + std::to_string(memUsed) +
           " kB / Total: " + std::to_string(memTotal) + " kB";
}


    // Send CPU updates every 2 seconds
    void start_event_notifier() {
        std::thread([this]() {
            //std::this_thread::sleep_for(std::chrono::seconds(2));

            while (true) {
                   // CPU event
                float cpu = get_cpu_usage();
                std::string cpu_data = "CPU Usage: " + std::to_string(cpu) + "%";
                auto cpu_payload = vsomeip::runtime::get()->create_payload();
                cpu_payload->set_data(std::vector<vsomeip::byte_t>(cpu_data.begin(), cpu_data.end()));
                app->notify(SERVICE_ID, INSTANCE_ID, EVENT_ID_CPU, cpu_payload);
                std::cout << "[Server] Sent CPU event: " << cpu_data << std::endl;

                // MEM event
                std ::string mem = get_mem_usage();
                std::string mem_data = "Memory Usage: " + mem;
                auto mem_payload = vsomeip::runtime::get()->create_payload();
                mem_payload->set_data(std::vector<vsomeip::byte_t>(mem_data.begin(), mem_data.end()));
                app->notify(SERVICE_ID, INSTANCE_ID, EVENT_ID_MEM, mem_payload);
                std::cout << "[Server] Sent MEM event: " << mem_data << std::endl;

                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        }).detach();
    }

    void start() {
        std::cout << "[Server] Initializing application..." << std::endl;
        app->init();
        std::cout << "[Server] Application initialized." << std::endl;

        std::cout << "[Server] Offering CPU event..." << std::endl;
        app->offer_event(SERVICE_ID, INSTANCE_ID, EVENT_ID_CPU, {EVENT_GROUP_CPU});
        app->offer_event(SERVICE_ID, INSTANCE_ID, EVENT_ID_MEM, {EVENT_GROUP_MEM});
        app->offer_service(SERVICE_ID, INSTANCE_ID);
        std::cout << "[Server] Service and event offered." << std::endl;

        std::cout << "[Server] Starting SOME/IP application loop..." << std::endl;

        start_event_notifier();

        app->start(); 
        std::cout << "[Server] Application started." << std::endl;

    }

private:
    std::shared_ptr<vsomeip::application> app;
};

int main() {
    std::cout << "[Server] Starting SOME/IP server..." << std::endl;
    SomeipProvider server;
    server.start();
    std::cout << "[Server] Server stopped." << std::endl;
    return 0;
}
