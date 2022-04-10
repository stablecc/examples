
#include "grpc_client.h"
#include "grpc_async_server.h"
#include "grpc_sync_server.h"
#include <gtest/gtest.h>
#include <util/logger.h>
#include <memory>
#include <future>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

using std::endl;
using std::string;
using std::vector;
using std::future;
using std::stringstream;
using std::async;
using scc::util::Logger;

struct GrpcTest : public testing::Test
{
	std::unique_ptr<CommandServer> async_serv;
	std::unique_ptr<CommandServer> sync_serv;
	std::future<void> fut_async;
	std::future<void> fut_sync;

	GrpcTest()
	:	async_serv(GrpcAsyncServer::get("0.0.0.0", 15430, 10)),
		sync_serv(GrpcSyncServer::get("0.0.0.0", 15431, 10))
	{
		fut_async = std::async([&]()
		{
			async_serv->serve();
		});
		
		fut_sync = std::async([&]()
		{
			sync_serv->serve();
		});
	}

	virtual ~GrpcTest()
	{
		async_serv->shut();
		fut_async.wait();
		
		sync_serv->shut();
		fut_sync.wait();
	}
};

TEST_F(GrpcTest, do_nothing)
{
}

TEST_F(GrpcTest, async_single)
{
	Logger log;
	log.add_cout();

	GrpcClient client("127.0.0.1", 15430);
	string message("async_single");
	string r = client.health(message);
	log << "HealthRequest( " << message << " ) -> HealthReply( " << r << " )" << endl;
	ASSERT_EQ(message, r);
}

TEST_F(GrpcTest, async_multi)
{
	Logger log;
	log.add_cout();

	auto reqf = [](string m) -> string
	{
		GrpcClient client("127.0.0.1", 15430);
		string r = client.health(m);

		Logger log;
		log.add_cout();

		log << "HealthRequest( " << m << " ) -> HealthReply( " << r << " )" << endl;

		return r;
	};

	vector<future<string>> clients;

	vector<string> ver;

	for (int i = 0; i < 100; i++)
	{
		stringstream s;
		s << "message from async_single " << std::setw(3) << std::setfill('0') << i;

		ver.push_back(s.str());
		clients.push_back(async(reqf, s.str()));
	}

	vector<string> v;
	for (auto& f : clients)
	{
		auto got = f.get();
		v.push_back(got);
	}

	std::sort(v.begin(), v.end());

	ASSERT_EQ(v, ver);
}

TEST_F(GrpcTest, sync_single)
{
	Logger log;
	log.add_cout();

	GrpcClient client("127.0.0.1", 15431);
	string message("sync_single");
	string r = client.health(message);
	log << "HealthRequest( " << message << " ) -> HealthReply( " << r << " )" << endl;
	ASSERT_EQ(message, r);
}

TEST_F(GrpcTest, sync_multi)
{
	Logger log;
	log.add_cout();

	auto reqf = [](string m) -> string
	{
		GrpcClient client("127.0.0.1", 15431);
		string r = client.health(m);

		Logger log;
		log.add_cout();

		log << "HealthRequest( " << m << " ) -> HealthReply( " << r << " )" << endl;

		return r;
	};

	vector<future<string>> clients;

	vector<string> ver;

	for (int i = 0; i < 100; i++)
	{
		stringstream s;
		s << "message from sync_single " << std::setw(3) << std::setfill('0') << i;

		ver.push_back(s.str());
		clients.push_back(async(reqf, s.str()));
	}

	vector<string> v;
	for (auto& f : clients)
	{
		auto got = f.get();
		v.push_back(got);
	}

	std::sort(v.begin(), v.end());

	ASSERT_EQ(v, ver);
}
