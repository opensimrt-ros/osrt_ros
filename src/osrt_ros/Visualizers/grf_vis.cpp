#include "osrt_ros/Visualizers/grf_vis.h"
#include "experimental/GRFMPrediction.h"
#include "opensimrt_msgs/Dual.h"
#include <SimTKcommon/internal/BigMatrix.h>

using opensimrt_msgs::DualConstPtr;

void Visualizers::GrfVis::after_vis()
{
	rightGRFDecorator = new OpenSimRT::ForceDecorator(SimTK::Green, 0.001, 3);
	visualizer->addDecorationGenerator(rightGRFDecorator);
	leftGRFDecorator = new OpenSimRT::ForceDecorator(SimTK::Green, 0.001, 3);
	visualizer->addDecorationGenerator(leftGRFDecorator);


}

void Visualizers::GrfVis::callback(const DualConstPtr &msg) {
	//get q from message
	SimTK::Vector q(msg->q.data.size());
	for (int i=0;i<msg->q.data.size(); i++)
	{
		q[i] = msg->q.data[i];
	}
	//get grfs from message
	
	try {
		visualizer->update(q);
		OpenSimRT::GRFMPrediction::Output grfmOutput;
		// after update q
		ROS_DEBUG_STREAM("updated visuals ok");
		rightGRFDecorator->update(grfmOutput.right.point, grfmOutput.right.force);
		leftGRFDecorator->update(grfmOutput.left.point, grfmOutput.left.force);
		ROS_DEBUG_STREAM("visualizer ran ok.");
	} catch (std::exception &e) {
		ROS_ERROR_STREAM("Error in visualizer. cannot show data!!!!!" << std::endl
				<< e.what());
	}
}
