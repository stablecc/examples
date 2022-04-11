#include <benchmark/benchmark.h>
#include "grpc_client.h"
#include "grpc_async_server.h"
#include "grpc_sync_server.h"
#include <future>

static void BM_Async(benchmark::State& state)
{
	auto serv = GrpcAsyncServer::get("0.0.0.0", 15430, 10);
	auto fut = std::async([&]()
	{
		serv->serve();
	});

	GrpcClient client("127.0.0.1", 15430);

	for (auto _ : state)
	{
		client.health("");
	}

	serv->shut();
	fut.wait();
}

BENCHMARK(BM_Async)->UseRealTime();

static void BM_Sync(benchmark::State& state)
{
	auto serv = GrpcSyncServer::get("0.0.0.0", 15430, 10);
	auto fut = std::async([&]()
	{
		serv->serve();
	});

	GrpcClient client("127.0.0.1", 15430);

	for (auto _ : state)
	{
		client.health("");
	}

	serv->shut();
	fut.wait();
}

BENCHMARK(BM_Sync)->UseRealTime();
