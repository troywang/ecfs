/*
 * managerbase.h
 *
 *  Created on: Aug 21, 2014
 *      Author: wanghuan
 */

#ifndef MANAGERBASE_H_
#define MANAGERBASE_H_

#include <boost/noncopyable.hpp>

class DatanodeServer;
class ManagerBase : public boost::noncopyable
{
public:
	ManagerBase (DatanodeServer &server) : mServer(server) {
	}

	virtual ~ManagerBase () {
	}

	virtual void init () {
	}

	virtual void uninit () {
	}

	DatanodeServer& get_server () {
		return mServer;
	}

protected:
	DatanodeServer &mServer;
};


#endif /* MANAGERBASE_H_ */
