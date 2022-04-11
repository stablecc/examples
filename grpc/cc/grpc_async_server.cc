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
#include "grpc_async_server.h"
#include <string>
#include <sstream>
#include <future>
#include <system_error>
#include <util/logger.h>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "grpc/proto/scc.grpc.pb.h"

using std::endl;
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using scc::Command;
using scc::HealthRequest;
using scc::HealthReply;
using scc::util::Logger;

/*
	Implements a multithreaded server using the grpc asynchronous server methods.
*/

class RequestProcessor
{
	Command::AsyncService* m_service;

	ServerCompletionQueue* m_cq;
	ServerContext m_ctx;

	enum CallStatus { INIT, PROCESS, FINISH };
	CallStatus m_status;
	
	HealthRequest m_request;
	HealthReply m_reply;
	
	ServerAsyncResponseWriter<HealthReply> m_responder;

public:
	RequestProcessor(Command::AsyncService* service, ServerCompletionQueue* cq)
		: m_service(service), m_cq(cq), m_status(INIT), m_responder(&m_ctx)
	{
		process();
	}

	void process()
	{
		if (m_status == INIT)
		{
			m_status = PROCESS;

			m_service->RequestHealth(&m_ctx, &m_request, &m_responder, m_cq, m_cq, this);
		}
		else if (m_status == PROCESS)
		{
			new RequestProcessor(m_service, m_cq);		// start processing another request

			// finish this one

			m_status = FINISH;

			if (!m_request.message().empty())
			{
				m_reply.set_message(m_request.message());
			}

			m_responder.Finish(m_reply, Status::OK, this);
		}
		else
		{
			delete this;
		}
	}
};

std::unique_ptr<CommandServer>  GrpcAsyncServer::get(const std::string& host, unsigned port, int max_threads, bool verbose)
{
	return std::unique_ptr<CommandServer>(new GrpcAsyncServer(host, port, max_threads, verbose));
}

struct GrpcAsyncServerInt
{
	Command::AsyncService service;
	std::unique_ptr<ServerCompletionQueue> cq;
	std::unique_ptr<Server> server;
	int max_threads;
	std::vector<std::future<void>> procs;
	bool verbose;
};

GrpcAsyncServer::GrpcAsyncServer(const std::string& host, unsigned port, int max_threads, bool verbose)
{
	m_ctx.reset(new GrpcAsyncServerInt);

	m_ctx->max_threads = max_threads;
	m_ctx->verbose = verbose;

	Logger log;
	if (m_ctx->verbose)
	{
		log.add_cout();
	}

	std::stringstream s;
	s << host << ":" << port;

	ServerBuilder builder;
	if (max_threads < 1)
	{
		throw std::runtime_error("max threads value must be > 0");
	}

	builder.AddListeningPort(s.str(), grpc::InsecureServerCredentials());

	builder.RegisterService(&m_ctx->service);

	m_ctx->cq = builder.AddCompletionQueue();

	m_ctx->server = builder.BuildAndStart();

	log << "serving at " << s.str() << " with max " << max_threads << " threads" << endl;
}

GrpcAsyncServer::~GrpcAsyncServer()
{
}

std::string GrpcAsyncServer::server_name() const
{
	return "grpc asyncronous Health server (using grpc::ServerBuilder with Command::AsyncService)";
}

void GrpcAsyncServer::serve()
{
	Logger log;
	if (m_ctx->verbose)
	{
		log.add_cout();
	}

	auto procRun = [&]()
	{
		new RequestProcessor(&m_ctx->service, m_ctx->cq.get());		// request processors add themselves to the queue
																	// and are self-deleting
		RequestProcessor* rp;
		bool ok;
		while (m_ctx->cq->Next((void**)&rp, &ok))					// returns true until cq is drained and shut down
		{
			if (ok)
			{
				rp->process();
			}
		}
	};

	log << "server starting " << m_ctx->max_threads << " processors" << endl;

	for (int i = 0; i < m_ctx->max_threads; i++)
	{
		m_ctx->procs.push_back(std::async(procRun));
	}

	log << "server wait for shutdown" << endl;

	m_ctx->server->Wait();
}

void GrpcAsyncServer::shut()
{
	Logger log;
	if (m_ctx->verbose)
	{
		log.add_cout();
	}

	log << "shutting server" << endl;
	m_ctx->server->Shutdown();
	
	log << "shutting queue" << endl;
	m_ctx->cq->Shutdown();

	log << "waiting for processors" << endl;
	for (auto& p : m_ctx->procs)
	{
		p.wait();
	}

	log << "server done" << endl;
}
