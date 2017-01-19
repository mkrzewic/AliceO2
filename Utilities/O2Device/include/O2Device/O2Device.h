// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// @copyright
/// © Copyright 2014 Copyright Holders of the ALICE O2 collaboration.
/// See https://aliceinfo.cern.ch/AliceO2 for details on the Copyright holders.
/// This software is distributed under the terms of the
/// GNU General Public License version 3 (GPL Version 3).
///
/// License text in a separate file.
///
/// In applying this license, CERN does not waive the privileges and immunities
/// granted to it by virtue of its status as an Intergovernmental Organization
/// or submit itself to any jurisdiction.

/// @headerfile O2Device.h
///
/// @since 2014-12-10
/// @author M. Krzewicki <mkrzewic@cern.ch>

#ifndef O2DEVICE_H_
#define O2DEVICE_H_

#include <FairMQDevice.h>
#include <FairMQLogger.h>
#include "Headers/DataHeader.h"
#include <stdexcept>
#include "O2Device/SharedMemory.h"

namespace o2 {
namespace Base {

/// just a typedef to express the fact that it is not just a FairMQParts vector,
/// it has to follow the O2 convention of header-payload-header-payload
using O2Message = FairMQParts;

class O2Device : public FairMQDevice {
public:
  // way to select how many consumers we expect for the message in shared memory:
  // None: message is not in shared memory, went over some-mq directly.
  // One: expect exactly one consumer
  // Many: expect many consumers (e.g. PUB/SUB)
  enum class SharedMemoryConsumers {
    None,
    One,
    Many
  };

  using FairMQDevice::FairMQDevice;
  ~O2Device() override = default;

  /// Here is how to add an annotated data part (with header);
  /// @param[in,out] parts is a reference to the message;
  /// @param[] incomingStack header block must be MOVED in (rvalue ref)
  /// @param[] dataMessage the data message must be MOVED in (unique_ptr by value)
  bool AddMessage(O2Message& parts,
                  o2::Header::Stack&& incomingStack,
                  FairMQMessagePtr incomingDataMessage) {

    //we have to move the incoming data
    o2::Header::Stack headerStack{std::move(incomingStack)};
    FairMQMessagePtr dataMessage{std::move(incomingDataMessage)};

    FairMQMessagePtr headerMessage = NewMessage(headerStack.buffer.get(),
                                                headerStack.bufferSize,
                                                &o2::Header::Stack::freefn,
                                                headerStack.buffer.get());
    headerStack.buffer.release();
    parts.AddPart(std::move(headerMessage));
    parts.AddPart(std::move(dataMessage));
    return true;
  }

  void setSharedMemorySize(size_t siz)
  {
    mSharedMemorySize = siz;
  }

  /// Add a message part to a message intendended for a local channel;
  /// this forces tha creation of shared memory metadata.
  /// Payload is empty so the user has to make sure this is only used when
  /// shared mem makes sense.
  /// TODO: automate this, the user should not have to think about this
  bool AddMessage(O2Message& parts, o2::Header::Stack headerBlockOrig, o2::SharedMemory::BlockPtr& shmBlock,
                  SharedMemoryConsumers consumers = SharedMemoryConsumers::One)
  {
    using namespace o2;

    // if we target meny consumers, register with garbage collector
    if (consumers == SharedMemoryConsumers::Many) {
      mShmManager->registerBlock(shmBlock);
    }

    // only add the shmem information if we actually send via shared memory
    if (consumers != SharedMemoryConsumers::None) {
      auto handle = shmBlock->getHandle(*mShmManager);
      Header::Stack headerBlock{ std::move(headerBlockOrig), Header::BoostShmHeader{ handle, shmBlock->getID() } };

      parts.AddPart(
        NewMessage(headerBlock.buffer.get(), headerBlock.bufferSize, &Header::Stack::freefn, headerBlock.buffer.get()));
      headerBlock.buffer.release();
      parts.AddPart(NewMessage());
    } else {
      // here we just sent the data "normally" over the network and need to decrease the use count
      // in the callback
      parts.AddPart(NewMessage(headerBlockOrig.buffer.get(), headerBlockOrig.bufferSize, &Header::Stack::freefn,
                               headerBlockOrig.buffer.get()));
      headerBlockOrig.buffer.release();
      parts.AddPart(NewMessage(shmBlock->data(), shmBlock->size(),
                               [](void*, void* ptr) { intrusive_ptr_release(static_cast<SharedMemory::Block*>(ptr)); },
                               &*shmBlock));
    }
    return true;
  }

  void CloseSharedMemoryMessage(O2Message&& tmpmessage)
  {
    O2Message parts{ std::move(tmpmessage) };
    for (auto it = parts.fParts.begin(); it != parts.fParts.end(); ++it) {
      byte* headerBuffer = nullptr;
      size_t headerBufferSize = 0;
      if (*it != nullptr) {
        headerBuffer = reinterpret_cast<byte*>((*it)->GetData());
        headerBufferSize = (*it)->GetSize();
        // handle possible use of shared memory
        using namespace o2::Header;
        const BoostShmHeader* shmheader = get<BoostShmHeader>(headerBuffer, headerBufferSize);
        if (shmheader) {
          if (!mShmManager) {
            mShmManager.reset(new o2::SharedMemory::Manager(mSharedMemorySize));
          }
          o2::SharedMemory::BlockPtr shmBlock{ mShmManager->getBlock(shmheader->handle, shmheader->id) };
        }
        ++it;

        byte* dataBuffer = nullptr;
        size_t dataBufferSize = 0;
        if (*it != nullptr) {
          // do nothing here, payloads are either empty
          // or will be cleaned by FairMQMessage dtor
        }
      }
    }
  }

  /// The user needs to define a member function with correct signature
  /// currently this is old school: buf,len pairs;
  /// In the end I'd like to move to array_view
  /// when this becomes available (either with C++17 or via GSL)
  template <typename T>
  bool ForEach(O2Message& parts, bool (T::*memberFunction)(const byte* headerBuffer, size_t headerBufferSize,
                                                           const byte* dataBuffer, size_t dataBufferSize))
  {
    if ((parts.Size() % 2) != 0)
      throw std::invalid_argument("number of parts in message not even (n%2 != 0)");

    for (auto it = parts.fParts.begin(); it != parts.fParts.end(); ++it) {

      byte* headerBuffer = nullptr;
      size_t headerBufferSize = 0;
      if (*it != nullptr) {
        headerBuffer = reinterpret_cast<byte*>((*it)->GetData());
        headerBufferSize = (*it)->GetSize();
      }
      ++it;

      byte* dataBuffer = nullptr;
      size_t dataBufferSize = 0;

      if (*it != nullptr) {
        // handle possible use of shared memory
        using namespace o2::Header;
        const BoostShmHeader* shmheader = get<BoostShmHeader>(headerBuffer, headerBufferSize);
        if (shmheader) {
          // here we expect the data to be in shmem
          if (!mShmManager) {
            mShmManager.reset(new o2::SharedMemory::Manager(mSharedMemorySize));
          }

          auto shmBlock = mShmManager->getBlock(shmheader->handle, shmheader->id);
          if (!shmBlock) {
            LOG(WARN) << "id mismatch or invalid block";
            continue;
          }

          dataBuffer = shmBlock->data();
          dataBufferSize = shmBlock->size();
          (static_cast<T*>(this)->*memberFunction)(headerBuffer, headerBufferSize, dataBuffer, dataBufferSize);
        } else {
          // here we expect to have the data in the fairmq message directly
          dataBuffer = reinterpret_cast<byte*>((*it)->GetData());
          dataBufferSize = (*it)->GetSize();
          (static_cast<T*>(this)->*memberFunction)(headerBuffer, headerBufferSize, dataBuffer, dataBufferSize);
        }
      }
    }
    return true;
  }

protected:
  std::unique_ptr<o2::SharedMemory::Manager> mShmManager;
  size_t mSharedMemorySize = 2000000000;

private:
};
}
}
#endif /* O2DEVICE_H_ */
