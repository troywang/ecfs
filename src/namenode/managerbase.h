/*
 * managerbase.h
 *
 *  Created on: Sep 10, 2014
 *      Author: wanghuan
 */

#ifndef MANAGERBASE_H_
#define MANAGERBASE_H_

#include <boost/noncopyable.hpp>

class NamenodeServer;
class ManagerBase : public boost::noncopyable
{
public:
	ManagerBase (NamenodeServer &server) : mServer(server) {
	}

	virtual ~ManagerBase () {
	}

	virtual void init () {
	}

	virtual void uninit () {
	}

	NamenodeServer& get_server () {
		return mServer;
	}

protected:
	NamenodeServer &mServer;
};




#endif /* MANAGERBASE_H_ */
