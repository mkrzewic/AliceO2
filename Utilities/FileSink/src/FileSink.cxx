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

/// @file FileSink.cxx
///
/// @since 2014-12-10
/// @author M. Krzewicki <mkrzewic@cern.ch>

#include <FairMQLogger.h>
#include <options/FairMQProgOptions.h>
#include "Headers/DataHeader.h"
#include "FileSink/FileSink.h"
#include "boost/iostreams/device/mapped_file.hpp"
#include <gsl/gsl>

using namespace std;
using namespace o2::header;
using namespace o2::Base;
using span = gsl::span<const byte>;

//__________________________________________________________________________________________________
void FileSink::InitTask()
{
}

//__________________________________________________________________________________________________
void FileSink::Run()
{
  uint64_t index{0};
  while (CheckCurrentState(RUNNING)) {
    O2Message message;
    int ret = Receive(message, "input");
    if (ret<=0) {
      LOG(ERROR) << "error or timeout, jumping out";
      break;
    }

    //this is ugly but for now FairMQParts does not internally account for this.
    size_t messageSize{ 0 };
    forEach(message,
            [&](span header, span payload) { messageSize += header.size() + payload.size(); });

    LOG(DEBUG) << "== New message, nparts: "<<message.fParts.size() << " size: " << messageSize;

    //map a file
    std::string fileName = mFileNameBase + std::to_string(index) + ".o2";
    boost::iostreams::mapped_file_params  params;
    params.path = fileName;
    params.flags = boost::iostreams::mapped_file_sink::readwrite;
    params.offset = 0;
    params.new_file_size = messageSize;
    params.length = messageSize;
    boost::iostreams::mapped_file_sink mf;
    mf.open(params);
    if (!mf.is_open()) {
      LOG(ERROR) << "file " << fileName << " could not be opened";
    }

    // dump all message parts to file
    byte* here = reinterpret_cast<byte*>(mf.data());
    forEach(message, [&](span headerBuffer, span payloadBuffer) {
      here = std::copy(headerBuffer.begin(), headerBuffer.end(), here);
      here = std::copy(payloadBuffer.begin(), payloadBuffer.end(), here);
    });
    ++index;
  }
}
