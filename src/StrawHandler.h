/*
 * StrawHandler.h
 *
 *  Created on: Sep 11, 2014
 *      Author: root
 */

#ifndef STRAWHANDLER_H_
#define STRAWHANDLER_H_

#include <dim/DimListener.h>
#include <sys/types.h>
#include <string>

namespace zmq {
class socket_t;
} /* namespace zmq */

namespace na62 {

class StrawHandler {
public:
	StrawHandler();
	virtual ~StrawHandler();

	/**
	 * Starts listening to the ZMQ socket and pushing incoming data to a file
	 */
	void run();

private:
	zmq::socket_t* pullSocket_;
	std::string generateFileName(uint burstID);
	dim::DimListener dimListener;
};

} /* namespace na62 */

#endif /* STRAWHANDLER_H_ */
