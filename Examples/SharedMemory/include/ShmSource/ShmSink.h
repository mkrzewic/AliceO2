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

/// @file ShmSink.h
///
/// @since 2017-01-01
/// @author M. Krzewicki <mkrzewic@cern.ch>


#ifndef SHMSINK_H_
#define SHMSINK_H_

#include "O2Device/O2Device.h"

class ShmSink : public o2::Base::O2Device
{
  public:
    ShmSink();
    virtual ~ShmSink();

  protected:
    virtual void InitTask();
    bool HandleData(o2::Base::O2Message& parts, int index);
    bool HandleO2frame(const byte* headerBuffer, size_t headerBufferSize,
        const byte* dataBuffer,   size_t dataBufferSize);

  private:
    int mCounter = 0;
};

#endif /*SHMSINK_H_*/
