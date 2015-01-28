/*
 * EventInfo.h
 *
 *  Created on: Nov 29, 2014
 *      Author: root
 */

#ifndef STRUCTS_EVENTINFO_H_
#define STRUCTS_EVENTINFO_H_

#include <boost/timer/timer.hpp>

struct EventInfo {
	uint32_t burstID;
	uint runNumber;
	uint sob;
	boost::timer::cpu_timer lastEventReceivedTimer;
};

#endif /* STRUCTS_EVENTINFO_H_ */
