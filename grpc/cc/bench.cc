#include <benchmark/benchmark.h>
#include "grpc_client.h"
#include "grpc_async_server.h"
#include "grpc_sync_server.h"
#include <future>
#include <vector>
#include <memory>
#include <string>

struct AsyncFixture : public benchmark::Fixture
{
	std::future<void> fut;
	std::unique_ptr<CommandServer> serv;
	std::unique_ptr<GrpcClient> cli;
	std::string msg;

	void SetUp(const benchmark::State& state)
	{
		serv = GrpcAsyncServer::get("0.0.0.0", 15430, state.range(0));
		fut = std::async([&]()
		{
			serv->serve();
		});
		cli.reset(new GrpcClient("127.0.0.1", 15430));
		msg.resize(state.range(1), 'X');
	}
	void TearDown(const benchmark::State& state)
	{
		serv->shut();
		fut.wait();
	}
};

BENCHMARK_DEFINE_F(AsyncFixture, Request)(benchmark::State& state)
{
	for (auto _ : state)
	{
		cli->health(msg);
	}
	state.SetBytesProcessed(state.iterations()*state.range(1));
	state.SetItemsProcessed(state.iterations());
}

BENCHMARK_REGISTER_F(AsyncFixture, Request)->Args({2, 1024})->Args({5, 1024})->Args({10, 1024})->Args({2, 10*1024})->Args({5, 10*1024})->Args({10, 10*1024});

struct SyncFixture : public benchmark::Fixture
{
	std::future<void> fut;
	std::unique_ptr<CommandServer> serv;
	std::unique_ptr<GrpcClient> cli;
	std::string msg;

	void SetUp(const benchmark::State& state)
	{
		serv = GrpcSyncServer::get("0.0.0.0", 15430, state.range(0));
		fut = std::async([&]()
		{
			serv->serve();
		});
		cli.reset(new GrpcClient("127.0.0.1", 15430));
	}
	void TearDown(const benchmark::State& state)
	{
		serv->shut();
		fut.wait();
	}
};

BENCHMARK_DEFINE_F(SyncFixture, Request)(benchmark::State& state)
{
	for (auto _ : state)
	{
		cli->health(msg);
	}
	state.SetBytesProcessed(state.iterations()*state.range(1));
	state.SetItemsProcessed(state.iterations());
}

BENCHMARK_REGISTER_F(SyncFixture, Request)->Args({2, 1024})->Args({5, 1024})->Args({10, 1024})->Args({2, 10*1024})->Args({5, 10*1024})->Args({10, 10*1024});
