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

/// @file Analysis.cxx
///
/// @since 2018-05-31
/// @author M. Zimmermann <mazimm@cern.ch>

#include <chrono>
#include <thread> // this_thread::sleep_for

#include <FairMQLogger.h>
#include <options/FairMQProgOptions.h>
#include "Headers/DataHeader.h"
#include "Headers/NameHeader.h"
#include "Analysis/Analysis.h"

#include "ReconstructionDataFormats/Vertex.h"
#include "ReconstructionDataFormats/Track.h"

#include <TH1F.h>
#include <TFile.h>

using namespace std;
using namespace o2::header;
using namespace o2::Base;
using namespace o2::dataformats;
using namespace o2::track;

//__________________________________________________________________________________________________
void Analysis::InitTask()
{

}

//__________________________________________________________________________________________________
void Analysis::Run()
{
  
  // check socket type of data channel
  std::string type;
  std::vector<FairMQChannel> subChannels = fChannels["data"];
  if (subChannels.size() > 0) {
    type = subChannels[0].GetType();
  }

  while (CheckCurrentState(RUNNING) && (--mIterations) != 0) {
    this_thread::sleep_for(chrono::milliseconds(mDelay));

    O2Message message;

    // message in;
    Receive(message, "data");
    LOG(INFO) << "== New message=============================";
    
    forEach(message, [&](auto header, auto data){
        this->Analysis::HandleO2frame(header.data(), header.size(), data.data(), data.size());
    });
    message.fParts.clear();

  }
}

//__________________________________________________________________________________________________
bool Analysis::HandleO2frame(const byte* headerBuffer, size_t headerBufferSize, const byte* dataBuffer,
                                     size_t dataBufferSize)
{
  //hexDump("headerBuffer", headerBuffer, headerBufferSize);
  //hexDump("dataBuffer", dataBuffer, dataBufferSize, 1000);

  LOG(INFO) << "== Handle O2 frame ==";
  
  const DataHeader* dataHeader = get<DataHeader*>(headerBuffer);
  if (!dataHeader) {
    LOG(INFO) << "data header empty!";
    return false;
  }
  
  if ((*dataHeader) == gDataDescriptionVertex) {
    LOG(INFO) << "data header is Vertex Header!";

    //vtx* ourData = reinterpret_cast<vtx*>(dataBuffer);
    auto vtxArray = reinterpret_cast<const Double_t*>(dataBuffer);

    int entries_vtx = dataBufferSize/sizeof(vtxArray[0]);
    LOG(INFO) << " entries "<<entries_vtx;
    
    for(int i=0; i<entries_vtx; i++){
      //LOG(INFO) << " Vertex content "<<i<<" is " << vtxArray[i];
    }
    
  }else{
    LOG(INFO) << "data header is not Vertex Header!";
  }


 
  if ((*dataHeader) == gDataDescriptionTrackPt) {
    TH1F* test = new TH1F("trackPt", "tracks;p_{T};count", 101, 0.0, 10.1);
    
    LOG(INFO) << " in gDataDescriptionTrackPt ";
    auto trackArray = reinterpret_cast<const Double_t*>(dataBuffer);
    int entries = dataBufferSize/sizeof(trackArray[0]);
    for(Int_t i=0; i<entries; i++){
      test->Fill(trackArray[i]);
    }

    TFile *f = TFile::Open("test_file.root", "RECREATE");
    test->Write();
    f->Close();
    
  }else{
    LOG(INFO) << "data header is not track Header!";
  }
  
  /*
  const NameHeader<0>* nameHeader = get<NameHeader<0>*>(headerBuffer);
  if (nameHeader) {
    size_t sizeNameHeader = nameHeader->size();
  }
  */
  return true;
}
