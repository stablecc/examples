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

"""Client for grpc health command"""

import logging
import grpc
import sys

import scc_pb2 as pb
import scc_pb2_grpc as pb_grpc

def send(host="127.0.0.1", port=1933, message=None, repeats=1):
	addr = host+":"+str(port)
	print("opening", addr)
	conn = grpc.insecure_channel(addr)
	cmd = pb_grpc.CommandStub(conn)
	reply = ""

	print("sending HealthRequest(", message, ") times", repeats)

	for _ in range(1, repeats):
		if message is None:
			reply = cmd.Health(pb.HealthRequest())
		else:
			reply = cmd.Health(pb.HealthRequest(message=message))
	
	return reply.message

if __name__ == '__main__':
	print("args: [message] [repeats] [host] [port]")
	logging.basicConfig()
	host = "127.0.0.1"
	port = 1933
	message = None
	repeats = 1
	if len(sys.argv) > 1:
		message = sys.argv[1]
	if len(sys.argv) > 2:
		repeats = int(sys.argv[2])
	if len(sys.argv) > 3:
		host = sys.argv[3]
	if len(sys.argv) > 4:
		port = int(sys.argv[4])
	r = send(host, port, message, repeats)
	print("last HealthReply(", r, ")")
