#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <fstream>

#define SERVICE_ID    0x1234
#define INSTANCE_ID   0x5678
#define METHOD_ID     0x42
#define METHOD_ID_CPU 0x44
// #define METHOD_ID_MEMORY 0x45 

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

    // Handle METHOD requests from clients
    void on_request(const std::shared_ptr<vsomeip::message> &request) {
        std::cout << "[Server] on_request called." << std::endl;

        uint16_t method = request->get_method();          
        auto payload = request->get_payload();          
        std::string response_data;

        std::cout << "[Server] Received request for method ID: " << method << std::endl;

        if (method == METHOD_ID) {
            response_data = "Hello from server!";
            std::cout << "[Server] METHOD request recognized." << std::endl;
        }
        else if (method == METHOD_ID_CPU) {
            float cpu = get_cpu_usage();
            response_data = "CPU Usage: " + std::to_string(cpu) + "%";
            std::cout << "[Server] CPU request received: " << cpu << "%\n";
        }
        // functions:
        // else if (method == METHOD_ID_MEMORY) { response_data = get_memory_usage(); }
        
        else {
            response_data = "Unknown method";
            std::cout << "[Server] Unknown method requested!" << std::endl;
        }

        // Log payload if available
        if (payload) {
            std::string client_message(
                reinterpret_cast<const char*>(payload->get_data()),
                payload->get_length()
            );
            std::cout << "[Server] Client payload: " << client_message << std::endl;
        } else {
            std::cout << "[Server] No payload received from client." << std::endl;
        }

        // Create and send the response
        std::cout << "[Server] Creating response message..." << std::endl;
        auto response = vsomeip::runtime::get()->create_response(request);

        std::cout << "[Server] Creating payload for response..." << std::endl;
        auto resp_payload = vsomeip::runtime::get()->create_payload();
        resp_payload->set_data(
            reinterpret_cast<const vsomeip::byte_t*>(response_data.data()),
            response_data.size()
        );
        response->set_payload(resp_payload);

        std::cout << "[Server] Sending response..." << std::endl;
        app->send(response);
        std::cout << "[Server] Response sent successfully: " << response_data << std::endl;
    }

    void start() {
        std::cout << "[Server] Initializing application..." << std::endl;
        app->init();
        std::cout << "[Server] Application initialized." << std::endl;

        std::cout << "[Server] Registering METHOD request handlers..." << std::endl;
        app->register_message_handler(
            SERVICE_ID, INSTANCE_ID, METHOD_ID,
            std::bind(&SomeipProvider::on_request, this, std::placeholders::_1)
        );
        app->register_message_handler(
            SERVICE_ID, INSTANCE_ID, METHOD_ID_CPU,
            std::bind(&SomeipProvider::on_request, this, std::placeholders::_1)
        );
        std::cout << "[Server] METHOD request handlers registered." << std::endl;

        std::cout << "[Server] Offering SOME/IP service..." << std::endl;
        app->offer_service(SERVICE_ID, INSTANCE_ID);
        std::cout << "[Server] Service offered successfully." << std::endl;

        std::cout << "[Server] Starting SOME/IP application loop..." << std::endl;
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
