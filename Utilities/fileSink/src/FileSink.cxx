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

/// @file O2MessageMonitor.cxx
///
/// @since 2014-12-10
/// @author M. Krzewicki <mkrzewic@cern.ch>

#include <FairMQLogger.h>
#include <options/FairMQProgOptions.h>
#include "Headers/DataHeader.h"
#include "FileSink/FileSink.h"

using namespace std;
using namespace o2::header;
using namespace o2::Base;

//__________________________________________________________________________________________________
void O2MessageMonitor::InitTask()
{
}

//__________________________________________________________________________________________________
void O2MessageMonitor::Run()
{
  while (CheckCurrentState(RUNNING)) {
    O2Message message;
    Receive(message, "data");
    LOG(INFO) << "== New message=============================";

    // message is always multi-part, so we need to handle that
    // the ForEach() method provides a transparent way to handle the O2 data protocol
    // that makes sure the metadata parts describe the proper data parts.
    ForEach(message, &O2MessageMonitor::HandleO2frame);
}

//__________________________________________________________________________________________________
bool O2MessageMonitor::HandleO2frame(const byte* headerBuffer, size_t headerBufferSize, const byte* dataBuffer,
                                     size_t dataBufferSize)
{
  AppendToDisk(headerBuffer, headerBufferSize);
  AppendToDisk(dataBuffer, dataBufferSize);

  /*
  const DataHeader* dataHeader = o2::header::get<DataHeader*>(headerBuffer);
  if ((*dataHeader) == gDataDescriptioVertexCoordinates) {
    //code to handle Vertex Coodinates data

    //this definition should live elsewhere
    struct vtx {
      float x, y, z;
    };

    vtx* ourData = reinterpret_cast<vtx*>(dataBuffer);

    float sum = 0.;
    for () { sum+=ourData[i].getX(); }
*/
  }

  return true;
}
