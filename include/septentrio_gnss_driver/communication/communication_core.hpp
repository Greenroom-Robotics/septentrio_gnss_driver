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

#ifndef COMMUNICATION_CORE_HPP // This block is called a conditional group. The
                               // controlled text will get included in the
                               // preprocessor output iff the macroname is not
                               // defined.
#define COMMUNICATION_CORE_HPP // Include guards help to avoid the double inclusion
                               // of header files, by defining a token = macro.

// Boost includes
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/exception/diagnostic_information.hpp> // dealing with bad file descriptor error
#include <boost/function.hpp>
// C++ library includes
#include <fstream>
#include <memory>
#include <sstream>
#include <unistd.h> // for usleep()
// ROSaic includes
#include <septentrio_gnss_driver/communication/async_manager.hpp>
#include <septentrio_gnss_driver/communication/callback_handlers.hpp>

/**
 * @file communication_core.hpp
 * @date 22/08/20
 * @brief Highest-Level view on communication services
 */

/**
 * @namespace io_comm_rx
 * This namespace is for the communication interface, handling all aspects related to
 * serial and TCP/IP communication..
 */
namespace io_comm_rx {

    //! Possible baudrates for the Rx
    const static uint32_t BAUDRATES[] = {
        1200,    2400,    4800,    9600,    19200,   38400,   57600,
        115200,  230400,  460800,  500000,  576000,  921600,  1000000,
        1152000, 1500000, 2000000, 2500000, 3000000, 3500000, 4000000};

    /**
     * @class Comm_IO
     * @brief Handles communication with and configuration of the mosaic (and beyond)
     * receiver(s)
     */
    class Comm_IO
    {
    public:
        /**
         * @brief Constructor of the class Comm_IO
         * @param[in] node Pointer to node
         */
        Comm_IO(ROSaicNodeBase* node, Settings* settings);
        /**
         * @brief Default destructor of the class Comm_IO
         */
        ~Comm_IO()
        {
            send("logout \x0D");
            stopping_ = true;
            connectionThread_->join();
        }

        /**
         * @brief Initializes the I/O handling
         */
        void initializeIO();

        /**
         * @brief Configures Rx: Which SBF/NMEA messages it should output and later
         * correction settings
         * @param[in] settings The device's settings
         * */
        void configureRx();

        /**
         * @brief Defines which Rx messages to read and which ROS messages to publish
         * @param[in] settings The device's settings
         * */
        void defineMessages();

    private:
        /**
         * @brief Sets up the stage for SBF file reading
         * @param[in] file_name The name of (or path to) the SBF file, e.g. "xyz.sbf"
         */
        void prepareSBFFileReading(std::string file_name);

        /**
         * @brief Sets up the stage for PCAP file reading
         * @param[in] file_name The path to PCAP file, e.g. "/tmp/capture.sbf"
         */
        void preparePCAPFileReading(std::string file_name);

        /**
         * @brief Attempts to (re)connect every reconnect_delay_s_ seconds
         */
        void reconnect();

        /**
         * @brief Calls the reconnect() method
         */
        void connect();

        /**
         * @brief Initializes the serial port
         * @param[in] port The device's port address
         * @param[in] baudrate The chosen baud rate of the port
         * @param[in] flowcontrol Default is "None", set variable (not yet checked)
         * to "RTS|CTS" to activate hardware flow control (only for serial ports
         * COM1, COM2 and COM3 (for mosaic))
         * @return True if connection could be established, false otherwise
         */
        bool initializeSerial(std::string port, uint32_t baudrate = 115200,
                              std::string flowcontrol = "None");

        /**
         * @brief Initializes the TCP I/O
         * @param[in] host The TCP host
         * @param[in] port The TCP port
         * @return True if connection could be established, false otherwise
         */
        bool initializeTCP(std::string host, std::string port);

        /**
         * @brief Initializes SBF file reading and reads SBF file by repeatedly
         * calling read_callback_()
         * @param[in] file_name The name of (or path to) the SBF file, e.g. "xyz.sbf"
         */
        void initializeSBFFileReading(std::string file_name);

        /**
         * @brief Initializes PCAP file reading and reads PCAP file by repeatedly
         * calling read_callback_()
         * @param[in] file_name The name of (or path to) the PCAP file, e.g.
         * "/tmp/capture.pcap"
         */
        void initializePCAPFileReading(std::string file_name);

        /**
         * @brief Set the I/O manager
         * @param[in] manager An I/O handler
         */
        void setManager(const boost::shared_ptr<Manager>& manager);

        /**
         * @brief Reset the Serial I/O port, e.g. after a Rx reset
         * @param[in] port The device's port address
         */
        void resetSerial(std::string port);

        /**
         * @brief Hands over to the send() method of manager_
         * @param cmd The command to hand over
         */
        void send(std::string cmd);

        //! Pointer to Node
        ROSaicNodeBase* node_;
        //! Callback handlers for the inwards streaming messages
        CallbackHandlers handlers_;
        //! Settings
        Settings* settings_;
        //! Whether connecting to Rx was successful
        bool connected_ = false;
        //! Since the configureRx() method should only be called once the connection
        //! was established, we need the threads to communicate this to each other.
        //! Associated mutex..
        boost::mutex connection_mutex_;
        //! Since the configureRx() method should only be called once the connection
        //! was established, we need the threads to communicate this to each other.
        //! Associated condition variable..
        boost::condition_variable connection_condition_;
        //! Host name of TCP server
        std::string tcp_host_;
        //! TCP port number
        std::string tcp_port_;
        //! Whether yet-to-be-established connection to Rx will be serial or TCP
        bool serial_;
        //! Saves the port description
        std::string serial_port_;
        //! Processes I/O stream data
        //! This declaration is deliberately stream-independent (Serial or TCP).
        boost::shared_ptr<Manager> manager_;
        //! Baudrate at the moment, unless InitializeSerial or ResetSerial fail
        uint32_t baudrate_;

        //! Connection or reading thread
        std::unique_ptr<boost::thread> connectionThread_;
        //! Indicator for threads to exit
        std::atomic<bool> stopping_;

        friend class CallbackHandlers;
        friend class RxMessage;

        //! Host currently connected to
        std::string host_;
        //! Port over which TCP/IP connection is currently established
        std::string port_;
        //! Sleep time in microseconds (there is no Unix command for milliseconds)
        //! after setting the baudrate to certain value (important between
        //! increments)
        const static unsigned int SET_BAUDRATE_SLEEP_ = 500000;
    };
} // namespace io_comm_rx

#endif // for COMMUNICATION_CORE_HPP
