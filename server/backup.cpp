#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <thread>

#define SERVICE_ID  0x1234
#define INSTANCE_ID 0x5678
#define METHOD_ID   0x42
#define EVENT_ID_CPU 0x43


float get_cpu_usage(const std::string &path = "/host_proc/stat") {
    static unsigned long long last[8] = {0};
    std::ifstream file(path);
    std::string cpu;
    unsigned long long vals[8] = {0};
    file >> cpu >> vals[0] >> vals[1] >> vals[2] >> vals[3] >> vals[4] >> vals[5] >> vals[6] >> vals[7];
    file.close();

    unsigned long long prev_idle = last[3] + last[4];
    unsigned long long idle = vals[3] + vals[4];

    unsigned long long prev_total = 0, total = 0;
    for (int i = 0; i < 8; i++) { prev_total += last[i]; total += vals[i]; last[i] = vals[i]; }

    unsigned long long delta_total = total - prev_total;
    unsigned long long delta_idle = idle - prev_idle;

    if (delta_total == 0) return 0.0f;
    return 100.0f * (delta_total - delta_idle) / delta_total;
}

class SomeipProvider {
public:
    SomeipProvider() {
        app_ = vsomeip::runtime::get()->create_application("SomeipProvider");
    }

    void on_request(const std::shared_ptr<vsomeip::message> &_request) {
        uint16_t method = _request->get_method();          
        auto payload = _request->get_payload();          
        std::string data;

        if (method == METHOD_ID) {
            data = "Hello from server!";
            std::cout << "[Server] METHOD request received\n";

            if (payload) {
                std::string client_message(
                    reinterpret_cast<const char*>(payload->get_data()),
                    payload->get_length()
                );
                std::cout << "[Server] Client payload: " << client_message << std::endl;
            }
        }
        else if (method == EVENT_ID_CPU) {
            float cpu = get_cpu_usage();
            data = std::to_string(cpu);
            std::cout << "[Server] CPU request received. CPU = " << cpu << "%\n";

            if (payload) {
                std::string client_message(
                    reinterpret_cast<const char*>(payload->get_data()),
                    payload->get_length()
                );
                std::cout << "[Server] Client payload: " << client_message << std::endl;
            }
        }
        else {
            data = "Unknown method";
            std::cout << "[Server] Unknown request method\n";
        }

        auto response = vsomeip::runtime::get()->create_response(_request);
        auto resp_payload = vsomeip::runtime::get()->create_payload();
        resp_payload->set_data(
            reinterpret_cast<const vsomeip::byte_t*>(data.data()), data.size()
        );
        response->set_payload(resp_payload);
        app_->send(response);

        std::cout << "[Server] Sent response." << std::endl;
    }

    void start_event_notifier() {
    std::thread([this]() {
        while(true) {
            float cpu = get_cpu_usage();
            std::string data = "CPU Usage: " + std::to_string(cpu) + "%";

            auto payload = vsomeip::runtime::get()->create_payload();
            payload->set_data(std::vector<vsomeip::byte_t>(data.begin(), data.end()));

            app_->notify(SERVICE_ID, INSTANCE_ID, EVENT_ID_CPU, payload);
            std::cout << "[Server] Sent CPU event: " << data << std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }).detach();
}

    void start() {
        app_->init();
        app_->register_message_handler(
            SERVICE_ID, INSTANCE_ID, METHOD_ID,
            std::bind(&SomeipProvider::on_request, this, std::placeholders::_1)
        );

        app_->offer_event(SERVICE_ID, INSTANCE_ID, EVENT_ID_CPU, {EVENT_GROUP_CPU});
        app_->offer_service(SERVICE_ID, INSTANCE_ID);
        app_->start();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        start_event_notifier();
    }

private:
    std::shared_ptr<vsomeip::application> app_;
};

int main() {
    SomeipProvider server;
    server.start();
    return 0;
}
