#include "client_ws.hpp"
#include "json_spirit.h"

using namespace std;
using namespace json_spirit;

typedef SimpleWeb::SocketClient<SimpleWeb::WS> WsClient;

int main() {
    
    //Example 4: Client communication with server
    //Possible output:
    //Server: Opened connection 140184920260656
    //Client: Opened connection
    //Client: Sending message: "Hello"
    //Server: Message received: "Hello" from 140184920260656
    //Server: Sending message "Hello" to 140184920260656
    //Client: Message received: "Hello"
    //Client: Sending close connection
    //Server: Closed connection 140184920260656 with status code 1000
    //Client: Closed connection with status code 1000
    WsClient client("c9000.db0sda.ampr.org:8055");
    client.on_message=[&client](shared_ptr<WsClient::Message> message) {
        auto message_str=message->string();

        cout << "Client: Message received: \"" << message_str << "\"" << endl;
	json_spirit::Value val;
	auto success = json_spirit::read(message_str, val);
	if (success) {
	    auto jsonObject = val.get_obj();
	for (auto entry : jsonObject) {
	      if (entry.name_ == "Status" && entry.value_.type() == json_spirit::Value_type::obj_type) {
		auto jsonObject2 = entry.value_.get_obj();
		cout << "Status ist da" << endl;

		for (auto entry2 : jsonObject2) {
   		if (entry2.name_ == "connected")  { cout << "vboolllll" << endl; }
		 if (entry2.name_ == "connected" && entry2.value_.type() == json_spirit::Value_type::bool_type) {
                   auto connected = entry2.value_.get_bool();
		   cout << "conn" << endl;
		   break;
 	         }
                }


	        break;
	      }
	}
}

        cout << "Client: Sending close connection" << endl;
        client.send_close(1000);
    };

    client.on_open=[&client]() {
        cout << "Client: Opened connection" << endl;

        string message="\"GetStatus\"";
        cout << "Client: Sending message: \"" << message << "\"" << endl;

        auto send_stream=make_shared<WsClient::SendStream>();
        *send_stream << message;
        client.send(send_stream);
    };

    client.on_close=[](int status, const string& /*reason*/) {
        cout << "Client: Closed connection with status code " << status << endl;
    };

    //See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
    client.on_error=[](const boost::system::error_code& ec) {
        cout << "Client: Error: " << ec << ", error message: " << ec.message() << endl;
    };

    client.start();

    return 0;
}
