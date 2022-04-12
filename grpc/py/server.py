#!/usr/bin/env python3
# BSD 3-Clause License
# 
# Copyright (c) 2022, Stable Cloud Computing, Inc.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""Server for grpc health command"""

from concurrent import futures
import logging
import grpc
import sys
import signal

import scc_pb2 as pb
import scc_pb2_grpc as pb_grpc

server = None

class Command(pb_grpc.CommandServicer):
	def Health(self, request, context):
		if request.message:
			return pb.HealthReply(message=request.message)
		else:
			return pb.HealthReply()

if __name__ == '__main__':
	print("args: [host] [port]")
	logging.basicConfig()
	host = "0.0.0.0"
	port = "1933"
	if len(sys.argv) > 1:
		host = sys.argv[1]
	if len(sys.argv) > 2:
		port = sys.argv[2]

	server = grpc.server(futures.ThreadPoolExecutor(max_workers=5))
	pb_grpc.add_CommandServicer_to_server(Command(), server)
	addr = host+":"+port
	server.add_insecure_port(addr)
	print("serving on", addr)
	server.start()

	def handler(num, fr):
		print("shutting down server")
		# stop the server to cause the main thread to complete and exit
		server.stop(1.0)

	signal.signal(signal.SIGINT, handler)
	signal.signal(signal.SIGTERM, handler)

	server.wait_for_termination()
