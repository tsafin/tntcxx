#include <alya.h>
/*
 * Copyright 2010-2020, Tarantool AUTHORS, please see AUTHORS file.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "Utils/Helpers.hpp"
#include "Utils/TupleReader.hpp"
#include "Utils/System.hpp"

#include "../src/Client/Connector.hpp"

const char *localhost = "127.0.0.1";
int WAIT_TIMEOUT = 1000; //milliseconds
using Net_t = DefaultNetProvider<Buf_t >;

constexpr size_t NUM_REQ = 1024;
constexpr size_t NUM_TEST = 10;

int main()
{
	if (cleanDir() != 0)
		return -1;
	if (launchTarantool() != 0)
		return -1;
	sleep(1);

	Connector<Buf_t> client;
	Connection<Buf_t, Net_t> conn(client);
	int rc = client.connect(conn, localhost, 3301);
	if (rc != 0)
		exit(0);

	for (size_t k = 0; k < NUM_TEST; k++) {
		auto s = conn.space[512];
		rid_t ids[NUM_REQ];
		alya::CTimer t;
		t.Start();
		for (size_t i = 0; i < NUM_REQ; i++) {
			ids[i] = s.select(std::make_tuple(1));
		}
		t.Stop();
		COUTF("Send", t.Mrps(NUM_REQ));

		t.Start();
		client.waitAll(conn, ids, NUM_REQ, WAIT_TIMEOUT);
		for (size_t i = 0; i < NUM_REQ; i++) {
			if (!conn.futureIsReady(ids[i]))
				abort();
			auto resp = conn.getResponse(ids[i]);
			if (!resp)
				abort();
			if (resp->header.code != 0)
				abort();
		}
		t.Stop();
		COUTF("Recv", t.Mrps(NUM_REQ));
	}

	client.close(conn);

	pid_t parent_pid = getpid();
	if (fork() == 0) {
		int rc = daemon(1, 1);
		(void)rc;
		int status = 0;
		waitpid(parent_pid, &status, 0);
		sleep(1);
		cleanDir();
		exit(0);
	}

	return 0;
}
