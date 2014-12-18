/*
 * thrift_service_base.h
 *
 *  Created on: May 30, 2014
 *      Author: wanghuan
 */

#ifndef THRIFT_SERVICE_BASE_H_
#define THRIFT_SERVICE_BASE_H_

#include "ecfs/src/common/common.h"
#include "thirdparty/thrift/protocol/TBinaryProtocol.h"
#include "thirdparty/thrift/server/TNonblockingServer.h"
#include "thirdparty/thrift/server/TThreadPoolServer.h"
#include "thirdparty/thrift/transport/TServerSocket.h"
#include "thirdparty/thrift/transport/TBufferTransports.h"
#include "thirdparty/thrift/concurrency/ThreadManager.h"
#include "thirdparty/thrift/concurrency/PosixThreadFactory.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

class ThriftServiceThreadWorker {
public:
	ThriftServiceThreadWorker(boost::shared_ptr<TProcessor> & processor, int threadNumber, uint16_t port)
        : mProcessor(processor), mThreadNumber(threadNumber), mPort(port) { }

    virtual ~ThriftServiceThreadWorker() { };

public:
    // Worker function
    void operator()()
    {
        boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
        boost::shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(mThreadNumber);
        boost::shared_ptr<PosixThreadFactory> threadFactory = boost::shared_ptr<PosixThreadFactory>(new PosixThreadFactory());
        threadManager->threadFactory(threadFactory);
        threadManager->start();
        TNonblockingServer server(mProcessor, protocolFactory, mPort, threadManager);
        server.serve();
    }

private:
    boost::shared_ptr<TProcessor> mProcessor;
    int mThreadNumber;
    uint16_t mPort;
};


#endif /* THRIFT_SERVICE_BASE_H_ */
