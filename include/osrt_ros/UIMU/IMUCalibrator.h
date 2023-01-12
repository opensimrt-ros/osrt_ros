/**
 * -----------------------------------------------------------------------------
 * Copyright 2019-2021 OpenSimRT developers.
 *
 * This file is part of OpenSimRT.
 *
 * OpenSimRT is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * OpenSimRT is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * OpenSimRT. If not, see <https://www.gnu.org/licenses/>.
 * -----------------------------------------------------------------------------
 *
 * @file IMUCalibrator.h
 *
 * @brief Calibration of the IMU data required before using the IK module.
 *
 * @author Filip Konstantinos <filip.k@ece.upatras.gr>
 */
#pragma once

#include "InputDriver.h"
#include "InverseKinematics.h"
//#include "U?->NGIMUInputDriver.h"
#include "Utils.h"
#include <Simulation/Model/Model.h>
#include <string>
#include <type_traits>
#include <ros/ros.h>
namespace OpenSimRT {

	/**
	 * Class used to calibrate IMUData for solving the IK module with orientation
	 * data. Initially, it calibrates the IK tasks based on the data from the
	 * initial pose of the subject that corresponds to the osim model pose. The
	 * subject should stand in pose for a number of seconds or a specific number of
	 * samples acquired from stream. After that the calibrator is used to transform
	 * the received data in the dynamic simulation based on the initial
	 * measurements. The transformations applied are the transformation of the
	 * ground-reference of the sensors to the OpenSim's ground reference frame, and
	 * the a heading transformation of the subject so that the heading axis of the
	 * subject coinsides with anterior axis in OpenSim (X-axis).
	 */
	class  IMUCalibrator {
		public:
			std::vector<ros::Publisher> pub;

			bool use_new_average_method = false;
			/**
			 * Construct a calibrator object. The constructor uses the Type-Erasure
			 * pattern/idiom that is based on automatic type deduction to remove the
			 * dependency on the IMUData type of the input driver. The only requirement
			 * is that the IMUData type of the driver MUST have a member function
			 * `getQuaternion()` to receive quaternion estimations for each IMU sensor.
			 */
			template <typename T>
				IMUCalibrator(const OpenSim::Model& otherModel,
						const InputDriver<T>* const driver,
						const std::vector<std::string>& observationOrder)
				// instantiate the DriverErasure object by forwarding the input
				// driver in its contructor.
				: impl(new DriverErasure<T>(
							std::forward<const InputDriver<T>* const>(driver), use_new_average_method)),
				model(*otherModel.clone()) {
					setup(observationOrder);
				}

			/**
			 * Set the rotation sequence of the axes (in degrees) that form the
			 * transformation between the sensor's reference frame and the OpenSim's
			 * ground reference frame.
			 */
			SimTK::Rotation setGroundOrientationSeq(const double& xDegrees,
					const double& yDegrees,
					const double& zDegrees);
			/**
			 * Compute the transformation for the heading correction based on the
			 * measurements acquired during the static phase.
			 */
			SimTK::Rotation computeHeadingRotation(const std::string& baseImuName,
					const std::string& imuDirectionAxis);
			/**
			 * Calibrate the IK IMUTasks prior the construction of the IK module.
			 */
			void calibrateIMUTasks(std::vector<InverseKinematics::IMUTask>& imuTasks);

			/**
			 * Time duration (in seconds) of the static phase during calibration.
			 */
			void recordTime(const double& timeout);

			/**
			 * Time duration (in number number of acquired samples) of the static
			 * phase during calibration.
			 */
			void recordNumOfSamples(const size_t& numSamples);

			/**
			 * Calibrate NGIMU data acquired from stream and create the IK input.
			 */
			void setMethod(bool method);
			void publishCalibrationData();
			template <typename T>
				SimTK::Array_<SimTK::Rotation> transform(const std::vector<T>& imuData) {
					SimTK::Array_<SimTK::Rotation> imuObservations;
					for (const auto& data : imuData) {
						const auto& q = data.getQuaternion();
						const auto R = R_heading * R_GoGi * ~SimTK::Rotation(q);
						imuObservations.push_back(R);
					}
					return imuObservations;
				}

		private:
			/**
			 * Type erasure on imu InputDriver types. Base class. Provides an interface
			 * for the functionality of derived classes.
			 */
			class DriverErasureBase {
				public:
					virtual ~DriverErasureBase() = default;
					bool use_new_average_method = false;
					virtual void recordTime(const double& timeout) = 0;
					virtual void recordNumOfSamples(const size_t& numSamples) = 0;
					virtual std::vector<std::vector<SimTK::Quaternion>> getTableData() = 0;
					virtual std::vector<SimTK::Quaternion> computeAvgStaticPose() = 0;
					void setMethod(bool method)
					{
						use_new_average_method = method;
						ROS_INFO_STREAM("method set to " << use_new_average_method);
					}
			};

			/**
			 * Type erasure on imu InputDriver types. Erasure class. Automatic type
			 * deduction of the driver's IMUData type <T>, allows any Input driver to be
			 * passed in the constructor.
			 */
			template <typename T> class DriverErasure : public DriverErasureBase {
				public:
					DriverErasure(const InputDriver<T>* const driver, bool method) : m_driver(driver) {
					use_new_average_method = method;
					}
					virtual std::vector<std::vector<SimTK::Quaternion>> getTableData() override
					{
					         std::vector<std::vector<SimTK::Quaternion>> table;
						int n = initIMUDataTable.size();    // num of recorded frames
						int m = initIMUDataTable[0].size(); // num of imu devices
							for (int j = 0; j< m ; ++j)
							{
								std::vector<SimTK::Quaternion> thisImuTable;
								for (int i = 0; i< n; ++i)
								{	
									thisImuTable.push_back(initIMUDataTable[i][j].getQuaternion());
								}
								table.push_back(thisImuTable);
							}	
						return table;
					}
					virtual void recordTime(const double& timeout) override {
						std::cout << "Recording Static Pose..." << std::endl;
						const auto start = std::chrono::steady_clock::now();
						while (std::chrono::duration_cast<std::chrono::seconds>(
									std::chrono::steady_clock::now() - start)
								.count() < timeout) {
							// get frame measurements. `getData()` is common to all input
							// drivers
							initIMUDataTable.push_back(m_driver->getData());
						}
					}

					virtual void recordNumOfSamples(const size_t& numSamples) override {
						std::cout << "Recording Static Pose..." << std::endl;
						size_t i = 0;
						while (i < numSamples) {
							// get frame measurements. `getData()` is common to all input
							// drivers
							std::cout << "am i stuck here?" << std::endl;
							initIMUDataTable.push_back(m_driver->getData());
							++i;
						}
					}

					/**
					 * Compute average of 3D rotations. Given a list of IMUData containing
					 * the quaternion measurements, computes the average quaternion error
					 * from the first sample and adds it back to the first sample to return
					 * the average quaternions.
					 *
					 * Source:
					 * https://math.stackexchange.com/questions/1984608/average-of-3d-rotations
					 */
					virtual std::vector<SimTK::Quaternion> computeAvgStaticPose() override {
						int n = initIMUDataTable.size();    // num of recorded frames
						int m = initIMUDataTable[0].size(); // num of imu devices
						auto avgQuaternionErrors =
							std::vector<SimTK::Quaternion>(m, SimTK::Quaternion());
						auto avgQuaternions(avgQuaternionErrors);
						
						if (use_new_average_method)
						{
							//calls new method for computing the avg pose
							ROS_INFO_STREAM("NEW: Using eigenvalues average of rotations");
							for (int j = 0; j< m ; ++j)
							{
								for (int i = 0; i< n; ++i)
								{
									SimTK::Quaternion q = initIMUDataTable[i][j].getQuaternion();
									ROS_INFO_STREAM(j << " <imu, sample > " << i );
									ROS_INFO_STREAM(q[0] << " " << q[1] << " " << q[2] << " " << q[3] );
								}
								
							}
							return avgQuaternions;
						}
							ROS_INFO_STREAM("OLD: Using norm average of rotations");

			

						// Quaternion product for each imu
						for (int j = 0; j < m; ++j)
							for (int i = 0; i < n; ++i) {
								// NOTE: requires `.getQuaternion()` function of the IMUData
								// type.
								const auto& q = initIMUDataTable[i][j].getQuaternion();
								avgQuaternionErrors[j] = avgQuaternionErrors[j] * (~q * q);
							}

						// convet to axis angle, devide with n, and convert back to
						// quaternions
						for (auto& q : avgQuaternionErrors) {
							q.setQuaternionFromAngleAxis(
									q.convertQuaternionToAngleAxis().scalarDivide(
										double(n)));
						}

						// add the error back to the first sample
						for (int j = 0; j < m; ++j) {
							// NOTE: requires `.getQuaternion()` function of the IMUData
							// type.
							const auto& q0 = initIMUDataTable[0][j].getQuaternion();
							const auto& qe = avgQuaternionErrors[j];
							avgQuaternions[j] = qe * q0;
						}

						return avgQuaternions;
					}
					
				private:
					SimTK::ReferencePtr<const InputDriver<T>> m_driver;
					std::vector<std::vector<T>> initIMUDataTable;
			};

			/**
			 * Supplamentary method used in the IMUCalibrator constructor.
			 */
			void setup(const std::vector<std::string>& observationOrder);

			OpenSim::Model model;
			SimTK::State state;
			std::unique_ptr<DriverErasureBase>
				impl; // pointer to DriverErasureBase class
			std::vector<SimTK::Quaternion> staticPoseQuaternions; // static pose data
			std::map<std::string, SimTK::Rotation> imuBodiesInGround; // R_GB per body
			std::vector<std::string> imuBodiesObservationOrder;       // imu order
			SimTK::Rotation R_GoGi;    // ground-to-ground transformation
			SimTK::Rotation R_heading; // heading correction
	};
} // namespace OpenSimRT
