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
#include <iostream>
#include <getopt.h>
#include <cstring>
#include <future>
#include <signal.h>
#include <util/logger.h>
#if defined(GRPC_SYNC_SERV)
#include "grpc_sync_server.h"
#elif defined(GRPC_ASYNC_SERV)
#include "grpc_async_server.h"
#endif

using std::cerr;
using std::endl;
using scc::util::Logger;

int main(int argc, char **argv)
{
	option lo[10];
	std::memset(&lo[0], 0, 10*sizeof(option));
	lo[0].name = "help";
	lo[0].val = '?';
	lo[1].name = "threads";
	lo[1].val = 't';
	lo[1].has_arg = 1;
	lo[2].name = "verbose";
	lo[2].val = 'v';
	lo[3].name = "queues";
	lo[3].val = 'q';
	lo[3].has_arg = 1;

	bool usage = false;
	int threads = 2, queues = 1;
	std::string host="0.0.0.0";
	unsigned port=1933;
	bool verbose=false;
	while (1)
	{
		int opt = getopt_long(argc, argv, "?ht:vq:", &lo[0], nullptr);
		if (opt == -1)
		{
			break;
		}
		switch (opt)
		{
		case '?':
		case 'h':
			usage = true;
			break;
		case 'v':
			verbose = true;
			break;
		case 't':
			if (!optarg)	usage = true;
			else			threads = atoi(optarg);
			break;
		case 'q':
			if (!optarg)	usage = true;
			else			queues = atoi(optarg);
			break;
		default:
			usage = true;
		}
	}

	if (usage)
	{
		using std::cerr;
		cerr << argv[0] << "[HOST] [PORT]" << endl;
		cerr << "  remote grpc Health server" << endl;
		cerr << endl;
		cerr << "      default HOST 0.0.0.0 PORT 5172" << endl;
		cerr << "      -q --queues number of completion queues used to serve requests (default 1)" << endl;
		cerr << "      -t --threads number of polling threads per queue (default 2)" << endl;
		cerr << "      -v --verbose verbose output (default off)" << endl;
		cerr << endl;
		exit(2);
	}

	if (optind < argc)
	{
		host = argv[optind++];
	}

	if (optind < argc)
	{
		port = atoi(argv[optind]);
	}

	Logger log;
	log.add_cout();
	log.id("main");

	#if defined(GRPC_SYNC_SERV)
	auto serv = GrpcSyncServer::get(host, port, queues, threads, verbose);
	#elif defined(GRPC_ASYNC_SERV)
	auto serv = GrpcAsyncServer::get(host, port, queues, threads, verbose);
	#else
	throw std::runtime_error("no server specified")
	#endif

	log << "using: " << serv->server_name() << endl;

	auto fut = std::async([&serv]()
	{
		serv->serve();
	});

	sigset_t sigs;
	sigfillset(&sigs);
	sigdelset(&sigs, SIGQUIT);			// allow ctrl-\ from keyboard to dump core
	sigdelset(&sigs, SIGABRT);			// allow abort() to dump core
	sigprocmask(SIG_SETMASK, &sigs, 0);

	log << "starting signal wait loop" << endl;

	bool done = false;
	while (!done)
	{
		int signum, r;
		if ((r = sigwait(&sigs, &signum)))
		{
			cerr << "sigwait(): " << strerror(r) << endl;
			break;
		}
		log << "signal: " << strsignal(signum) << " (" << signum << ")" << endl;
		switch (signum)
		{
		case SIGTERM:
			done = true;
		break;

		case SIGINT:
			done = true;
		break;
		}
	}

	log << "signal server exit" << endl;

	serv->shut();
	fut.wait();

	log << "server shut down" << endl;

	return 0;
}
