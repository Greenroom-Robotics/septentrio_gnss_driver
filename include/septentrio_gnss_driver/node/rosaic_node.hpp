// *****************************************************************************
//
// © Copyright 2020, Septentrio NV/SA.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//    1. Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//    2. Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//    3. Neither the name of the copyright holder nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// *****************************************************************************

// *****************************************************************************
//
// Boost Software License - Version 1.0 - August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:

// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
// *****************************************************************************

#pragma once

/**
 * @file rosaic_node.hpp
 * @date 21/08/20
 * @brief The heart of the ROSaic driver: The ROS node that represents it
 */

// tf2 includes
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
// ROSaic includes
#include <septentrio_gnss_driver/communication/communication_core.hpp>

/**
 * @namespace rosaic_node
 * This namespace is for the ROSaic node, handling all aspects regarding
 * ROS parameters, ROS message publishing etc.
 */
namespace rosaic_node {
    /**
     * @class ROSaicNode
     * @brief This class represents the ROsaic node, to be extended..
     */
    class ROSaicNode : public ROSaicNodeBase
    {
    public:
        //! The constructor initializes and runs the ROSaic node, if everything works
        //! fine. It loads the user-defined ROS parameters, subscribes to Rx
        //! messages, and publishes requested ROS messages...
        ROSaicNode(const rclcpp::NodeOptions& options);

        ~ROSaicNode();

    private:
        void setup();
        /**
         * @brief Gets the node parameters from the ROS Parameter Server, parts of
         * which are specified in a YAML file
         *
         * The other ROSaic parameters are specified via the command line.
         */
        [[nodiscard]] bool getROSParams();
        /**
         * @brief Checks if the period has a valid value
         * @param[in] period period [ms]
         * @param[in] isIns wether recevier is an INS
         * @return wether the period is valid
         */
        [[nodiscard]] bool validPeriod(uint32_t period, bool isIns) const;
        /**
         * @brief Gets transforms from tf2
         * @param[in] targetFrame traget frame id
         * @param[in] sourceFrame source frame id
         * @param[out] T_s_t transfrom from source to target
         */
        void getTransform(const std::string& targetFrame,
                          const std::string& sourceFrame,
                          TransformStampedMsg& T_s_t) const;
        /**
         * @brief Gets Euler angles from quaternion message
         * @param[in] qm quaternion message
         * @param[out] roll roll angle
         * @param[out] pitch pitch angle
         * @param[out] yaw yaw angle
         */
        void getRPY(const QuaternionMsg& qm, double& roll, double& pitch,
                    double& yaw) const;

        void sendVelocity(const std::string& velNmea);

        void diagnosticsCallback();

        //! Handles communication with the Rx
        io::CommunicationCore IO_;
        //! tf2 buffer and listener
        tf2_ros::Buffer tfBuffer_;
        std::unique_ptr<tf2_ros::TransformListener> tfListener_;

        std::thread setupThread_;

        rclcpp::TimerBase::SharedPtr diagnosticsTimer_;
        rclcpp::Publisher<DiagnosticArrayMsg>::SharedPtr diagnostics_publisher_;
        bool connectedToINS_ = false;
    };
} // namespace rosaic_node