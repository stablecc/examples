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
#include "grpc_sync_server.h"
#include <string>
#include <sstream>
#include <system_error>
#include <util/logger.h>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "grpc/proto/scc.grpc.pb.h"

using std::endl;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using scc::Command;
using scc::HealthRequest;
using scc::HealthReply;
using scc::util::Logger;

/*
	Implements a multithreaded server using the grpc synchronous server methods.
*/

class CommandServiceImpl final : public Command::Service
{
	Status Health(ServerContext* context, const HealthRequest* request, HealthReply* reply) override
	{
		if (!request->message().empty())
		{
			reply->set_message(request->message());
		}
		return Status::OK;
	}
};

std::unique_ptr<CommandServer> GrpcSyncServer::get(const std::string& host, unsigned port, int max_threads)
{
	return std::unique_ptr<CommandServer>(new GrpcSyncServer(host, port, max_threads));
}

struct GrpcSyncServerInt
{
	CommandServiceImpl service;
	std::unique_ptr<Server> server;
};

GrpcSyncServer::GrpcSyncServer(const std::string& host, unsigned port, int max_threads)
{
	Logger log;
	log.add_cout();

	m_ctx.reset(new GrpcSyncServerInt);

	std::stringstream s;
	s << host << ":" << port;

	grpc::EnableDefaultHealthCheckService(true);
	grpc::reflection::InitProtoReflectionServerBuilderPlugin();
	
	ServerBuilder builder;
	if (max_threads < 1)
	{
		throw std::runtime_error("max threads value must be > 0");
	}

	builder.SetSyncServerOption(ServerBuilder::MAX_POLLERS, max_threads);
	builder.AddListeningPort(s.str(), grpc::InsecureServerCredentials());
	builder.RegisterService(&m_ctx->service);

	log << "serving at " << s.str() << " with max " << max_threads << " threads" << endl;

	m_ctx->server = builder.BuildAndStart();
}

GrpcSyncServer::~GrpcSyncServer()
{
}

std::string GrpcSyncServer::server_name() const
{
	return "grpc syncronous Health server (using grpc::ServerBuilder with Command::Service)";
}

void GrpcSyncServer::serve()
{
	Logger log;
	log.add_cout();

	log << "server wait for shutdown" << endl;

	m_ctx->server->Wait();

	log << "server shut" << endl;
}

void GrpcSyncServer::shut()
{
	Logger log;
	log.add_cout();
	
	log << "shutting server" << endl;
	m_ctx->server->Shutdown();

	log << "server done" << endl;
}
