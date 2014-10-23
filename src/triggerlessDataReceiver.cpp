//============================================================================
// Name        : simpleEclipseCPP11Project.cpp
// Author      : Jonas Kunze (kunze.jonas@gmail.com)
//============================================================================

#include <socket/ZMQHandler.h>

#include "options/MyOptions.h"
#include "StrawHandler.h"

using namespace na62;

int main(int argc, char* argv[]) {
	/*
	 * Static Class initializations
	 */
	MyOptions::Load(argc, argv);

	ZMQHandler::Initialize(1);

	StrawHandler handler;

	handler.run();
	return 0;
}
