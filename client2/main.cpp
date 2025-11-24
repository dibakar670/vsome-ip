#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
 
#define SERVICE_ID  0x1234
#define INSTANCE_ID 0x5678
#define METHOD_ID   0x42
 
class SomeipClient {
public:
    SomeipClient() {
        app_ = vsomeip::runtime::get()->create_application("client");
    }
 
    void on_availability(vsomeip::service_t service, vsomeip::instance_t instance, bool is_available) {
        if (is_available) {
            std::cout << "[Client] Service is available. Sending request..." << std::endl;
 
            auto request = vsomeip::runtime::get()->create_request();
            request->set_service(SERVICE_ID);
            request->set_instance(INSTANCE_ID);
            request->set_method(METHOD_ID);
 
            std::string message = "Hello from client_2!";
            std::vector<vsomeip::byte_t> payload_data(message.begin(), message.end());
 
            auto payload = vsomeip::runtime::get()->create_payload();
            payload->set_data(payload_data);
 
            request->set_payload(payload);
            app_->send(request);
        } else {
            std::cout << "[Client] Service is NOT available." << std::endl;
        }
    }
 
    void on_response(const std::shared_ptr<vsomeip::message> &response) {
    auto payload = response->get_payload();
    if (payload) {
        std::string response_str(reinterpret_cast<const char*>(payload->get_data()), payload->get_length());
        std::cout << "[Client] Received response: " << response_str << std::endl;
    }
}
 
 
    void start() {
        app_->init();
 
        app_->register_availability_handler(
            SERVICE_ID, INSTANCE_ID,
            std::bind(&SomeipClient::on_availability, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
        );
 
        app_->register_message_handler(
            SERVICE_ID, INSTANCE_ID, METHOD_ID,
            std::bind(&SomeipClient::on_response, this, std::placeholders::_1)
        );
 
        app_->request_service(SERVICE_ID, INSTANCE_ID);
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
 
 