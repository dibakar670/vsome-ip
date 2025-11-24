#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <string>

#define SERVICE_ID    0x1234
#define INSTANCE_ID   0x5678
#define METHOD_ID     0x42
#define EVENT_ID_CPU  0x43
class SomeipClient {
public:
    SomeipClient() {
        app_ = vsomeip::runtime::get()->create_application("CPUClient");
    }

    // Called when service availability changes
    void on_availability(vsomeip::service_t service, vsomeip::instance_t instance, bool is_available) {
        if (is_available) {
            std::cout << "[Client] Service is available." << std::endl;

            // Send initial METHOD request
            auto request = vsomeip::runtime::get()->create_request();
            request->set_service(SERVICE_ID);
            request->set_instance(INSTANCE_ID);
            request->set_method(METHOD_ID);

            std::string message = "Hello from client!";
            std::vector<vsomeip::byte_t> payload_data(message.begin(), message.end());
            auto payload = vsomeip::runtime::get()->create_payload();
            payload->set_data(payload_data);
            request->set_payload(payload);

            app_->send(request);

            // Subscribe to CPU events
            app_->subscribe(SERVICE_ID, INSTANCE_ID, EVENT_ID_CPU, {});

        } else {
            std::cout << "[Client] Service is NOT available." << std::endl;
        }
    }

    // Called when METHOD response is received
    void on_response(const std::shared_ptr<vsomeip::message> &response) {
        auto payload = response->get_payload();
        if (payload) {
            std::string response_str(reinterpret_cast<const char*>(payload->get_data()), payload->get_length());
            std::cout << "[Client] Received METHOD response: " << response_str << std::endl;
        }
    }

    // Called when CPU event is received
    void on_cpu_event(const std::shared_ptr<vsomeip::message> &event) {
        auto payload = event->get_payload();
        if (payload) {
            std::string cpu_str(reinterpret_cast<const char*>(payload->get_data()), payload->get_length());
            std::cout << "[Client] CPU event received: " << cpu_str << std::endl;
        }
    }

    void start() {
        app_->init();

        // Register availability handler
        app_->register_availability_handler(
            SERVICE_ID, INSTANCE_ID,
            std::bind(&SomeipClient::on_availability, this,
                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
        );

        // Register METHOD response handler
        app_->register_message_handler(
            SERVICE_ID, INSTANCE_ID, METHOD_ID,
            std::bind(&SomeipClient::on_response, this, std::placeholders::_1)
        );

        // Register CPU event handler
        app_->register_message_handler(
            SERVICE_ID, INSTANCE_ID, EVENT_ID_CPU,
            std::bind(&SomeipClient::on_cpu_event, this, std::placeholders::_1)
        );

        // Request service
        app_->request_service(SERVICE_ID, INSTANCE_ID);

        // Start the SOME/IP application
        app_->start();
    }

private:
    std::shared_ptr<vsomeip::application> app_;
};

int main() {
    SomeipClient client;
    client.start();
    return 0;
}
