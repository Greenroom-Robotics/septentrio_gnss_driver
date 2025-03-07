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

// C++ includes
#include <condition_variable>

// ROSaic includes
#ifdef ROS2
#include <septentrio_gnss_driver/abstraction/typedefs.hpp>
#endif
#ifdef ROS1
#include <septentrio_gnss_driver/abstraction/typedefs_ros1.hpp>
#endif
#include <septentrio_gnss_driver/communication/message_handler.hpp>
#include <septentrio_gnss_driver/communication/telegram.hpp>

/**
 * @file telegram_handler.hpp
 * @brief Handles messages when reading NMEA/SBF/response/error/connection descriptor
 * messages
 */

namespace io {
    class Semaphore
    {
    public:
        Semaphore() : block_(true) {}

        void notify()
        {
            std::unique_lock<std::mutex> lock(mtx_);
            block_ = false;
            cv_.notify_one();
        }

        void wait()
        {
            std::unique_lock<std::mutex> lock(mtx_);
            while (block_)
            {
                cv_.wait(lock);
            }
            block_ = true;
        }

    private:
        std::mutex mtx_;
        std::condition_variable cv_;
        bool block_;
    };

    /**
     * @class TelegramHandler
     * @brief Represents ensemble of (to be constructed) ROS messages, to be handled
     * at once by this class
     */
    class TelegramHandler
    {

    public:
        TelegramHandler(ROSaicNodeBase* node) : node_(node), messageHandler_(node) {}

        ~TelegramHandler()
        {
            cdSemaphore_.notify();
            responseSemaphore_.notify();
        }

        void clearSemaphores()
        {
            cdSemaphore_.notify();
            responseSemaphore_.notify();
        }

        /**
         * @brief Called every time a telegram is received
         */
        void handleTelegram(const std::shared_ptr<Telegram>& telegram);

        //! Returns the connection descriptor
        void resetWaitforMainCd() { mainConnectionDescriptor_ = std::string(); }

        //! Returns the connection descriptor
        [[nodiscard]] std::string getMainCd()
        {
            cdSemaphore_.wait();
            return mainConnectionDescriptor_;
        }

        //! Waits for response
        void waitForResponse() { responseSemaphore_.wait(); }

        //! Waits for capabilities
        void waitForCapabilities() { capabilitiesSemaphore_.wait(); }

        // Add a public getter method for messageHandler_
        MessageHandler& getMessageHandler() {return messageHandler_;}

    private:
        void handleSbf(const std::shared_ptr<Telegram>& telegram);
        void handleNmea(const std::shared_ptr<Telegram>& telegram);
        void handleResponse(const std::shared_ptr<Telegram>& telegram);
        void handleError(const std::shared_ptr<Telegram>& telegram);
        void handleCd(const std::shared_ptr<Telegram>& telegram);
        //! Pointer to Node
        ROSaicNodeBase* node_;

        //! MessageHandler parser
        MessageHandler messageHandler_;

        Semaphore cdSemaphore_;
        Semaphore responseSemaphore_;
        Semaphore capabilitiesSemaphore_;
        std::string mainConnectionDescriptor_ = std::string();
    };

} // namespace io