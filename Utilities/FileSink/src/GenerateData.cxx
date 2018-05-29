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
#include "Headers/NameHeader.h"
#include "ReconstructionDataFormats/Vertex.h"
#include "GenerateData/GenerateData.h"

using namespace std;
using namespace o2::header;
using namespace o2::Base;

using NameHeader48 = NameHeader<48>; // header holding 16 characters

//__________________________________________________________________________________________________
void GenerateData::InitTask()
{
}

//__________________________________________________________________________________________________
void GenerateData::Run()
{
  while (CheckCurrentState(RUNNING)) {
    O2Message message;

        LOG(INFO) << "== Add new message=============================";

	//decide how big the array aill be
	size_t nVertex{10};

	using Vertex=o2::dataformats::Vertex<>;
	
  size_t size = nVertex*sizeof(Vertex);

	//allocate memory for data
	auto vertexMessage = NewMessage(size);

	Vertex* vtxArray = static_cast<Vertex*>(vertexMessage->GetData());

	for (int i=0; i<nVertex; ++i) {
	  vtxArray[i] = Vertex(Point3D<float>{1., 1., 1.}, {2., 2., 2., 2., 2., 2.}, 1000, 1);
	}
	
    AddMessage(message,
	       { DataHeader{ gDataDescriptionVertex, gDataOriginAny, DataHeader::SubSpecificationType{ 0 }, size }},
	       std::move(vertexMessage));
    Send(message, "data");
    ChangeState(PAUSE);

  }
}

