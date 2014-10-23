/*
 * StrawHandler.cpp
 *
 *  Created on: Sep 11, 2014
 *      Author: root
 */

#include "StrawHandler.h"

#include <socket/ZMQHandler.h>
#include <sys/types.h>
#include <zmq.h>
#include <zmq.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <structs/Network.h>
#include <dim/DimListener.h>
#include <utils/DataDumper.h>
#include <fstream>
#include <thread>

#include "options/MyOptions.h"

namespace na62 {
struct UDP_HDR;
} /* namespace na62 */

namespace na62 {

StrawHandler::StrawHandler() :
		numberOfFramesReceived_(0) {
	std::stringstream address;
	address << "tcp://*:" << MyOptions::GetInt(OPTION_STRAW_PORT);
	pullSocket_ = ZMQHandler::GenerateSocket(ZMQ_PULL);
	pullSocket_->bind(address.str().c_str());

	dimListener_.startServer();
	usleep(100000);

	monitorThread_ =
			new std::thread(
					[this]() {
						while(true) {
							sleep(1);
							std::cout << "Number of frames received: " << numberOfFramesReceived_<<std::endl;
						}
					});

}

StrawHandler::~StrawHandler() {
	ZMQHandler::DestroySocket(pullSocket_);
}

std::string StrawHandler::generateFileName(uint burstID) {
	std::string storageDir = MyOptions::GetString(OPTION_STORAGE_DIR);
	std::stringstream fileName;
	fileName << Options::GetString(OPTION_FILE_PREFIX) << "_"
			<< dimListener_.getRunNumber() << "_burst_" << burstID;
	return DataDumper::generateFreeFilePath(fileName.str(), storageDir);
}

void StrawHandler::run() {
	uint lastBurstID = 0xFFFFFFFF;
	std::string fileName = "";

	std::ofstream myfile;

	while (ZMQHandler::IsRunning()) {
		zmq::message_t msg;
		pullSocket_->recv(&msg);

		/*
		 * First part of the message is burstID
		 */
		uint burstID = (uint) *((uint*) msg.data());

		if (!msg.more()) {
			std::cerr
					<< "Received single message. Expecting multimessage with burstID as first message and data as second message!"
					<< std::endl;
			continue;
		}

		numberOfFramesReceived_++;

		if (burstID != lastBurstID) {
			fileName = generateFileName(burstID);
			lastBurstID = burstID;

			std::cout << "Received data from new burst " << burstID
					<< ". Now writing to file " << fileName << std::endl;

			if (myfile.is_open()) {
				myfile.close();
			}

			myfile.open(fileName.data(),
					std::ios::out | std::ios::app | std::ios::binary);
		}

		/*
		 * Receive second part of multimessage (data)
		 */
		pullSocket_->recv(&msg);

		const char* rawData = (const char*) msg.data();
		const u_int dataLength = msg.size();

		if (!myfile.good()) {
			std::cerr << "Unable to write to file " << fileName << std::endl;
			// carry on to free the memory. myfile.write will not throw!
		} else {
			myfile.write(rawData, dataLength);
		}
	}
	if (myfile.is_open()) {
		myfile.close();
	}
}

} /* namespace na62 */
