#include "client_ws.hpp"
#include "json_spirit.h"
#include "boost/program_options.hpp"

#define STDWSPORT 8055

using namespace std;
using namespace json_spirit;
namespace po = boost::program_options;

typedef SimpleWeb::SocketClient<SimpleWeb::WS> WsClient;


int main(int argc, char** argv) {

	int port = STDWSPORT;
	string Hostname = "localhost";
	string url = "";

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("Hostname,H", po::value<string>(), "set websocket hostname")
    ("port,p", po::value<int>(&port)->implicit_value(STDWSPORT), "set websocket port")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);    

	if (vm.count("help")) {
		cout << desc << "\n";
		
		return 3;
	}

	if (vm.count("port")) {
		port = vm["port"].as<int>();
	}

	if (vm.count("Hostname")) {
		Hostname = vm["Hostname"].as<string>();
	}

	url = Hostname + ":" + std::to_string(port);

	WsClient client(url);

	client.on_message=[&client](shared_ptr<WsClient::Message> message) {
		auto message_str=message->string();
//		cout << "Client: Message received: \"" << message_str << "\"" << endl;

		json_spirit::Value val;
		auto success = json_spirit::read(message_str, val);
		
		if (success) {
	  	auto jsonObject = val.get_obj();
			for (auto entry : jsonObject) {
	  		if (entry.name_ == "Status" && entry.value_.type() == json_spirit::Value_type::obj_type) {
					auto jsonObject2 = entry.value_.get_obj();

					for (auto entry2 : jsonObject2) {
						if (entry2.name_ == "connected" && entry2.value_.type() == json_spirit::Value_type::bool_type) {
							auto connected = entry2.value_.get_bool();
							if (connected) {
								client.send_close(1000);
								cout << "CONNECTION OK" << endl;
								return 0;
							} else {
								client.send_close(1000);
								cout << "CONNECTION CRITICAL" << endl;
								return 2;
							}
							break;
	 	        }
					}
					break;
				}
			}
		}
  	client.send_close(1000);
		cout << "CONNECTION UNKNOWN" << endl;
		return 3;
  };

  client.on_open=[&client]() {
//  	cout << "Client: Opened connection" << endl;

    string message="\"GetStatus\"";
//    cout << "Client: Sending message: \"" << message << "\"" << endl;

    auto send_stream=make_shared<WsClient::SendStream>();
    *send_stream << message;
    client.send(send_stream);
	};

// 	client.on_close=[](int status, const string& /*reason*/) {
//	cout << "Client: Closed connection with status code " << status << endl;
//  };


	//See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
	client.on_error=[&](const boost::system::error_code& ec) {
	cout << "CONNECTION UNKNOWN | ";
  cout << "Client: Error: " << ec << ", error message: " << ec.message() << " URL: " << url <<endl;
	return 3;
  };

  client.start();

  return 3;
}
