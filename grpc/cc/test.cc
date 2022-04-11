
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

TEST(GrpcTest, async_single)
{
	auto serv = GrpcAsyncServer::get("0.0.0.0", 12345, 10);
	auto fut = std::async([&]()
	{
		serv->serve();
	});

	Logger log;
	log.add_cout();

	GrpcClient client("127.0.0.1", 12345);
	string message("async_single");
	string r = client.health(message);
	log << "HealthRequest( " << message << " ) -> HealthReply( " << r << " )" << endl;
	ASSERT_EQ(message, r);

	serv->shut();
	fut.wait();
}

TEST(GrpcTest, async_multi)
{
	auto serv = GrpcAsyncServer::get("0.0.0.0", 12345, 10);
	auto fut = std::async([&]()
	{
		serv->serve();
	});

	Logger log;
	log.add_cout();

	auto reqf = [](string m) -> string
	{
		GrpcClient client("127.0.0.1", 12345);
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

	serv->shut();
	fut.wait();
}

TEST(GrpcTest, sync_single)
{
	auto serv = GrpcSyncServer::get("0.0.0.0", 12345, 10);
	auto fut = std::async([&]()
	{
		serv->serve();
	});

	Logger log;
	log.add_cout();

	GrpcClient client("127.0.0.1", 12345);
	string message("sync_single");
	string r = client.health(message);
	log << "HealthRequest( " << message << " ) -> HealthReply( " << r << " )" << endl;
	ASSERT_EQ(message, r);

	serv->shut();
	fut.wait();
}

TEST(GrpcTest, sync_multi)
{
	auto serv = GrpcSyncServer::get("0.0.0.0", 12345, 10);
	auto fut = std::async([&]()
	{
		serv->serve();
	});

	Logger log;
	log.add_cout();

	auto reqf = [](string m) -> string
	{
		GrpcClient client("127.0.0.1", 12345);
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

	serv->shut();
	fut.wait();
}
