#include <Actuators/Schutte1993Muscle_Deprecated.h>

#include <ros/ros.h>

#include <exception>
#define BOOST_STACKTRACE_USE_ADDR2LINE
#include <boost/stacktrace.hpp>
#include "osrt_ros/UIMU/UIMUnode.h"

int main(int argc, char** argv) {
	try {
		ros::init(argc, argv, "talker");
		ros::NodeHandle n;
		UIMUnode o;
		// either like this:
		OpenSim::Object* muscleModel = new OpenSim::Schutte1993Muscle_Deprecated();
		o.registerType(muscleModel);
		// or alternatively, more simply:
		// Object::registerType(Schutte1993Muscle_Deprecated());
 		o.onInit();
		o.run();
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		return -1;
	}
	return 0;
}


