/*
BSD 3-Clause License

Copyright (c) 2022, Stable Cloud Computing, Inc.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "grpc_client.h"
#include <string>
#include <sstream>
#include <future>
#include <system_error>
#include <util/event.h>
#include <util/logger.h>

#include <grpcpp/grpcpp.h>

#include "grpc/proto/scc.grpc.pb.h"

using std::endl;
using std::stringstream;
using std::runtime_error;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using scc::Command;
using scc::HealthRequest;
using scc::HealthReply;
using scc::util::Logger;

class CommandClient
{
	std::unique_ptr<Command::Stub> m_stub;
public:
	CommandClient(std::shared_ptr<Channel> channel)
	: m_stub(Command::NewStub(channel))
	{}

	// Assembles the client's payload, sends it and presents the response back
	// from the server.
	std::string Health(const std::string& message)
	{
		HealthRequest request;

		if (message.size())
		{
			request.set_message(message);
		}

		HealthReply reply;
		ClientContext context;

		Status status = m_stub->Health(&context, request, &reply);

		if (!status.ok())
		{
			stringstream s;
			s << status.error_code() << ": " << status.error_message();
			throw runtime_error(s.str());
		}
		return reply.message();
	}
};

GrpcClient::GrpcClient(const std::string& host, unsigned port)
{
	std::stringstream s;
	s << host << ":" << port;

	m_cmd.reset(new CommandClient(grpc::CreateChannel(s.str(), grpc::InsecureChannelCredentials())));
}

std::string GrpcClient::health(const std::string& message)
{
	return m_cmd->Health(message);
}

#if defined(GRPC_CLIENT_MAIN)
int main(int argc, char **argv)
{
	using std::string;
	using std::cout;
	using std::cerr;

	string host("127.0.0.1"), message;
	unsigned port=1933;

	if (argc > 1)
	{
		message = argv[1];
	}
	if (argc > 2)
	{
		host = argv[2];
	}
	if (argc > 3)
	{
		port = atoi(argv[3]);
	}

	cout << "creating client to " << host << ":" << port << endl;

	GrpcClient c(host, port);

	try
	{
		cout << "sending HealthRequest( " << message << " )" << endl;
		auto r = c.health(message);
		cout << "got HealthReply( " << r << " )" << endl;
	}
	catch (std::exception& ex)
	{
		cerr << "failed with error " << ex.what() << endl;
	}

	return 0;
}
#endif
