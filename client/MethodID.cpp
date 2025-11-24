#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <string>

#define SERVICE_ID    0x1234
#define INSTANCE_ID   0x5678
#define METHOD_ID     0x42
#define METHOD_ID_CPU 0x44


class SomeipClient {
public:
    SomeipClient() {
        std::cout << "[Client] Initializing SOME/IP application..." << std::endl;
        app = vsomeip::runtime::get()->create_application("SomeipClient");
        std::cout << "[Client] Application created." << std::endl;
    }

    void on_availability(vsomeip::service_t service, vsomeip::instance_t instance, bool is_available) {
        if (is_available) {
            std::cout << "[Client] Service is available. Sending requests..." << std::endl;
            send_method_request();
            send_cpu_request();

            //  functions to add:
            // send_memory_request();
        } else {
            std::cout << "[Client] Service NOT available." << std::endl;
        }
    }

    void send_request(uint16_t method_id) {
        auto request = vsomeip::runtime::get()->create_request();
        request->set_service(SERVICE_ID);
        request->set_instance(INSTANCE_ID);
        request->set_method(method_id);
        app->send(request);

        std::cout << "[Client] Sent request for METHOD ID: " << method_id << std::endl;
    }

    void send_method_request() { send_request(METHOD_ID); }
    void send_cpu_request() { send_request(METHOD_ID_CPU); }

    //functions:
    // void send_memory_request() { send_request(METHOD_ID_MEMORY); }

    void on_response(const std::shared_ptr<vsomeip::message> &response) {
        auto payload = response->get_payload();
        if (payload) {
            std::string data(payload->get_data(), payload->get_data() + payload->get_length());
            std::cout << "[Client] Response received: " << data << std::endl;
        } else {
            std::cout << "[Client] Received empty response." << std::endl;
        }
    }

    void start() {
        std::cout << "[Client] Initializing application..." << std::endl;
        app->init();
        std::cout << "[Client] Application initialized." << std::endl;

        std::cout << "[Client] Registering availability handler..." << std::endl;
        app->register_availability_handler(SERVICE_ID, INSTANCE_ID,
                                           std::bind(&SomeipClient::on_availability, this,
                                                     std::placeholders::_1,
                                                     std::placeholders::_2,
                                                     std::placeholders::_3));
        std::cout << "[Client] Availability handler registered." << std::endl;

        std::cout << "[Client] Registering METHOD response handlers..." << std::endl;
        app->register_message_handler(SERVICE_ID, INSTANCE_ID, METHOD_ID,
                                       std::bind(&SomeipClient::on_response, this, std::placeholders::_1));
        app->register_message_handler(SERVICE_ID, INSTANCE_ID, METHOD_ID_CPU,
                                       std::bind(&SomeipClient::on_response, this, std::placeholders::_1));
        std::cout << "[Client] METHOD response handlers registered." << std::endl;

        std::cout << "[Client] Requesting service..." << std::endl;
        app->request_service(SERVICE_ID, INSTANCE_ID);
        std::cout << "[Client] Starting SOME/IP application loop..." << std::endl;
        app->start();
        std::cout << "[Client] Application started." << std::endl;
    }

private:
    std::shared_ptr<vsomeip::application> app;
};

int main() {
    std::cout << "[Client] Starting SOME/IP client..." << std::endl;
    SomeipClient client;
    client.start();
    std::cout << "[Client] Client stopped." << std::endl;
    return 0;
}
