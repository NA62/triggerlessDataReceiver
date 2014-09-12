/*
 * MyOptions.h
 *
 *  Created on: Apr 11, 2014
 \*      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef MYOPTIONS_H_
#define MYOPTIONS_H_

#include <options/Options.h>
#include <string>
#include <boost/thread.hpp>

#define OPTION_STRAW_PORT (char*)"strawPort"

#define OPTION_STORAGE_DIR (char*)"storageDir"

namespace na62 {
class MyOptions: public Options {
public:
	MyOptions();
	virtual ~MyOptions();

	static void Load(int argc, char* argv[]) {
		desc.add_options()

		(OPTION_CONFIG_FILE,
				po::value<std::string>()->default_value(
						"/etc/na62RawDataReceiver.cfg"),
				"Config file for the options shown here")

		(OPTION_STRAW_PORT, po::value<int>()->default_value(58917),
				"Port used to receive STRAW data.")

		(OPTION_STORAGE_DIR, po::value<std::string>()->required(),
				"Path to the directory where all burst files should be written to.")

				;

		Options::Initialize(argc, argv, desc);
	}
};

} /* namespace na62 */

#endif /* MYOPTIONS_H_ */
