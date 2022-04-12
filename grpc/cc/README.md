# c++ grpc example

Example of c++ grpc.

```
bazel run :test_all # all unit tests
bazel run -c opt :bench # all bench tests with optimized build
bazel run :syncserv -- -v # synchronous server
bazel run :asyncserv -- -v # asynchronous server
bazel run :syncclient -- "test message" 1000 # send message 1000 times
```
