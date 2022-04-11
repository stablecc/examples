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
#ifndef _GRPC_ASYNC_SERVER_H
#define _GRPC_ASYNC_SERVER_H

#include "server.h"
#include <memory>

struct GrpcAsyncServerInt;

class GrpcAsyncServer : public CommandServer
{
	std::unique_ptr<GrpcAsyncServerInt> m_ctx;
	GrpcAsyncServer(const std::string&, unsigned, int, int, bool);
public:
	GrpcAsyncServer& operator=(GrpcAsyncServer const &) = delete;
	virtual ~GrpcAsyncServer();

	/** Gets and starts the singleton command server.
		\param host Hostname or address
		\param port Port
		\param queues Number of completion queues.
		\param threads Polling threads per queue used to service requests.
		\param verbose Verbose server logging (connection only)
	*/
	static std::unique_ptr<CommandServer> get(const std::string&, unsigned, int, int, bool = false);

	void serve();
	void shut();
	std::string server_name() const;
};

#endif
