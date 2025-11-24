#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <string>

#define SERVICE_ID    0x1234
#define INSTANCE_ID   0x5678
#define EVENT_ID_CPU  0x43
#define EVENT_GROUP_CPU 0x01
#define EVENT_ID_MEM    0x44 //added for mem
#define EVENT_GROUP_MEM 0x02

class SomeipClient {
public:
    SomeipClient() {
        std::cout << "[Client] Initializing SOME/IP application..." << std::endl;
        app = vsomeip::runtime::get()->create_application("SomeipClient");
        std::cout << "[Client] Application created." << std::endl;
    }

    void on_availability(vsomeip::service_t service, vsomeip::instance_t instance, bool is_available) {
        if (is_available) {
            std::cout << "[Client] Service available. Subscribing to CPU events..." << std::endl;
            app->subscribe(SERVICE_ID, INSTANCE_ID, EVENT_GROUP_CPU, 0, EVENT_ID_CPU);
            app->subscribe(SERVICE_ID, INSTANCE_ID, EVENT_GROUP_MEM, 0, EVENT_ID_MEM);
        } else {
            std::cout << "[Client] Service NOT available." << std::endl;
        }
    }

    void on_cpu_event(const std::shared_ptr<vsomeip::message> &event) {
        auto payload = event->get_payload();
        if (payload && payload->get_length() > 0) 
        {
            std::string cpu_str(payload->get_data(), payload->get_data() + payload->get_length());
            std::cout << "[Client] CPU event received: " << cpu_str << std::endl;
        } else 
        {
            std::cout << "[Client] CPU event received but payload is empty!" << std::endl;
        }
    }

    void on_mem_event(const std::shared_ptr<vsomeip::message> &event) {
    auto payload = event->get_payload();
    if (payload && payload->get_length() > 0) {
        std::string mem_str(payload->get_data(), payload->get_data() + payload->get_length());
        std::cout << "[Client] Memory event received: " << mem_str << std::endl;
    } else {
        std::cout << "[Client] Memory event received but payload is empty!" << std::endl;
    }
}



    void start() {
        std::cout << "[Client] Initializing application..." << std::endl;
        app->init();
        std::cout << "[Client] Application initialized." << std::endl;

        // Register availability handler
        std::cout << "[Client] Registering availability handler..." << std::endl;
        app->register_availability_handler(SERVICE_ID, INSTANCE_ID,
                                           std::bind(&SomeipClient::on_availability, this,
                                                     std::placeholders::_1,
                                                     std::placeholders::_2,
                                                     std::placeholders::_3));

        // Register CPU event handler
        std::cout << "[Client] Registering CPU event handler..." << std::endl;
        app->register_message_handler(SERVICE_ID, INSTANCE_ID, EVENT_ID_CPU,
                                      std::bind(&SomeipClient::on_cpu_event, this, std::placeholders::_1));

        app->register_message_handler(SERVICE_ID, INSTANCE_ID, EVENT_ID_MEM,
                                      std::bind(&SomeipClient::on_mem_event, this, std::placeholders::_1));

        std::cout << "[Client] Requesting service..." << std::endl;
        app->request_service(SERVICE_ID, INSTANCE_ID);

        std::cout << "[Client] Starting SOME/IP application loop..." << std::endl;
        app->start();
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
