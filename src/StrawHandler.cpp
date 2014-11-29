/*
 * StrawHandler.cpp
 *
 *  Created on: Sep 11, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
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
#include <map>

#include "options/MyOptions.h"
#include "EventInfo.h"

namespace na62 {
struct UDP_HDR;
} /* namespace na62 */

namespace na62 {

StrawHandler::StrawHandler() :
		numberOfFramesReceived_(0) {
	std::stringstream address;
	address << "tcp://*:" << MyOptions::GetInt(OPTION_STRAW_PORT);
	pullSocket_ = ZMQHandler::GenerateSocket("StrawReceiver", ZMQ_PULL);
	pullSocket_->bind(address.str().c_str());

	dimListener_.startServer();
	usleep(100000);

	monitorThread_ =
			new std::thread(
					[this]() {
						while(true) {
							sleep(1);
							LOG_INFO << "Number of frames received: " << numberOfFramesReceived_<<ENDL;
						}
					});

}

StrawHandler::~StrawHandler() {
	ZMQHandler::DestroySocket(pullSocket_);
	pullSocket_ = nullptr;
}

std::string StrawHandler::generateFileName(EventInfo info) {
	struct tm tstruct;
	char buf[32];
	tstruct = *localtime((time_t*) &(info.sob));
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%Y-%m-%d_%X", &tstruct);

	char fileName[64];
	sprintf(fileName, "%u-%s-%06u-%04u.dat", info.sob, buf, info.runNumber,
			info.burstID);

	std::string storageDir = MyOptions::GetString(OPTION_STORAGE_DIR);
	std::stringstream filePath;
	filePath << Options::GetString(OPTION_FILE_PREFIX) << "_" << fileName;
	//return DataDumper::generateFreeFilePath(filePath.str(), storageDir);
	return storageDir + "/" + filePath.str();
}

void StrawHandler::run() {

	std::map<uint, std::pair<std::shared_ptr<std::ofstream>, EventInfo>> fileAndEventInfoByBurstID;

	while (ZMQHandler::IsRunning()) {
		zmq::message_t msg;
		pullSocket_->recv(&msg);

		/*
		 * First part of the message is burstID
		 */
		uint burstID = (uint) *((uint*) msg.data());

		if (!msg.more()) {
			LOG_ERROR
					<< "Received single message. Expecting multimessage with burstID as first message and data as second message!"
					<< ENDL;
			continue;
		}

		numberOfFramesReceived_++;

		std::shared_ptr<std::ofstream> myfile;

		if (fileAndEventInfoByBurstID.find(burstID)
				!= fileAndEventInfoByBurstID.end()) {
			EventInfo info = { burstID, dimListener_.getRunNumber(),
								dimListener_.getSobTimeStamp() };

			std::string fileName = generateFileName(info);

			LOG_INFO << "Received data from new burst " << burstID
					<< ". Now writing to file " << fileName << ENDL;

			myfile = std::make_shared<std::ofstream>();
			myfile->open(fileName.data(),
					std::ios::out | std::ios::app | std::ios::binary);


			fileAndEventInfoByBurstID[burstID] = std::make_pair(myfile, info);

			/*
			 * Close old file handles
			 */
			if (fileAndEventInfoByBurstID.find(burstID - 10)
					!= fileAndEventInfoByBurstID.end()) {
				std::shared_ptr<std::ofstream> file =
						fileAndEventInfoByBurstID[burstID - 10].first;
				if (file->is_open()) {
					file->close();
				}
				fileAndEventInfoByBurstID.erase(burstID - 10);
			}

		} else {
			myfile = fileAndEventInfoByBurstID[burstID].first;
		}

		/*
		 * Receive second part of multimessage (data)
		 */
		pullSocket_->recv(&msg);

		const char* rawData = (const char*) msg.data();
		const u_int dataLength = msg.size();

		if (!myfile->good()) {
			LOG_ERROR << "Unable to write to file " << generateFileName(fileAndEventInfoByBurstID[burstID].second) << ENDL;
			// carry on to free the memory. myfile.write will not throw!
		} else {
			myfile->write(rawData, dataLength);
		}
	}
}

} /* namespace na62 */
