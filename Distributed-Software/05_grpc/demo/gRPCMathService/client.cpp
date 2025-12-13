/*
 * $ protoc --cpp_out=. --cpp-grpc_out=.  mathtest.proto
 * $ c++ -std=c++11 -I/root/.local/include -I/usr/local/include -L/root/.local/lib  -L/usr/local/lib client.cpp mathtest.pb.cc mathtest.grpc.pb.cc `pkg-config --cflags --libs protobuf grpc++`  -o client
*/

#include <string>

#include <grpcpp/grpcpp.h>
#include "mathtest.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using mathtest::MathTest;
using mathtest::MathRequest;
using mathtest::MathReply;

class MathTestClient {
    public:
        MathTestClient(std::shared_ptr<Channel> channel) : stub_(MathTest::NewStub(channel)) {}

    int sendRequest(int a, int b) {
        MathRequest request;

        request.set_a(a);
        request.set_b(b);

        MathReply reply;

        ClientContext context;

        Status status = stub_->sendRequest(&context, request, &reply);

        if(status.ok()){
            return reply.result();
        } else {
            std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            return -1;
        }
    }

    private:
        std::unique_ptr<MathTest::Stub> stub_;
};

void Run(int x, int y) {
    std::string address("0.0.0.0:5000");
    MathTestClient client(
        grpc::CreateChannel(
            address, 
            grpc::InsecureChannelCredentials()
        )
    );

    int response;

    int a = x;
    int b = y;

    response = client.sendRequest(a, b);
    std::cout << "Answer received: " << a << " * " << b << " = " << response << std::endl;
}

int main(int argc, char* argv[]){
    int x = 5;
    int y = 10;

    if(argc == 3) {
        x = atoi(argv[1]);
        y = atoi(argv[2]);
    }
    Run(x, y);

    return 0;
}