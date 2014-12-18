/*
 * common.h
 *
 *  Created on: Apr 25, 2014
 *      Author: wanghuan
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <set>
#include <map>
#include <list>
#include <deque>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <stdexcept>

#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <ifaddrs.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <boost/thread.hpp>
#include <boost/utility.hpp>
#include <boost/function.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/interprocess/detail/atomic.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include "thirdparty/glog/logging.h"

#include "ecfs/src/common/codec.h"
#include "ecfs/src/common/utils.h"
#include "ecfs/src/common/md5lib.h"
#include "ecfs/src/common/errcode.h"
#include "ecfs/src/common/configs.h"
#include "ecfs/src/common/rw_lock.h"
#include "ecfs/src/common/exception.h"
#include "ecfs/src/common/thread_pool.h"
#include "ecfs/src/common/thrift_connection_pool.h"

#define MAX_FILE_NAME_LEN 31

#define LOG_INFO_SMART LOG_IF(INFO, (FLAGS_minloglevel <= google::INFO))
#define LOG_WARNING_SMART LOG_IF(WARNING, (FLAGS_minloglevel <= google::WARNING))

#define CHECK_RET(ret, msg) do { if (!RESULT_IS_SUCCESS(ret)) \
		ErasureUtils::throw_exception(ret, msg); \
		} while(0)

#define CHECK_TRUE(exp, msg) do { if (!(exp)) \
	    ErasureUtils::throw_exception(RESULT_ERR_GENERAL, msg); \
		} while(0)


enum ServerStatus {
	Server_Starting = 0,
    Server_Running = 1,
    Server_Suspended = 2,
    Server_Dead = 3,
};

/*
 * 0       7|8     11|12      31
 * +++++++++++++++++++++++++++++
 * | nodeid | diskid |  seqid  |
 * +++++++++++++++++++++++++++++
 */
#define NODE_ID_BITS 8
#define DISK_ID_BITS 4
#define SEQ_ID_BITS 20
#define CHUNK_ID_BITS 32

#define NODE_ID_OFFSET (CHUNK_ID_BITS - NODE_ID_BITS)
#define DISK_ID_OFFSET (NODE_ID_OFFSET - DISK_ID_BITS)

#define NODE_ID(chunkid) (int16_t)(((uint32_t)chunkid >> NODE_ID_OFFSET) & ((1 << NODE_ID_BITS) - 1))
#define DISK_ID(chunkid) (int8_t)(((uint32_t)chunkid >> DISK_ID_OFFSET) & ((1 << DISK_ID_BITS) - 1))
#define SEQ_ID(chunkid) (int32_t)(chunkid & ((1 << SEQ_ID_BITS) - 1))

#define MAKE_CHUNK_ID(nodeid, diskid, seqid) (nodeid << NODE_ID_OFFSET | diskid << DISK_ID_OFFSET | seqid)

#endif /* COMMON_H_ */
