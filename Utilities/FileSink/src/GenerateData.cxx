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
#include "ReconstructionDataFormats/Track.h"
#include "GenerateData/GenerateData.h"

#include <TFile.h>
#include <TList.h>
#include <TNtupleD.h>

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
  // check socket type of data channel
  std::string type;
  std::vector<FairMQChannel>& subChannels = fChannels["data"];
  if (subChannels.size() > 0) {
    type = subChannels[0].GetType();
  }

  auto dataResource = o2::memoryResources::getTransportAllocator(subChannels[0].Transport());
  
  O2Message message;

  LOG(INFO) << "== Add new message=============================";

  TFile *f = TFile::Open("run2.root");//, "RECREATE");
  /*
  TList *list = (TList*) gFile->Get("test");

  if(list)
    LOG(INFO) << "have list "<<list->GetName();
  else
    LOG(INFO) << "no list";
  
  //TNtupleD *ntuple = (TNtupleD*)list->FindObject("ntuple");
  //TNtupleD *ntuple = (TNtupleD*)list->First();
  TObject* obj = list->First();

  if(obj)
    LOG(INFO) << "ntuple var "<<obj->GetTitle();
  else
    LOG(INFO) << "no ntuple found";
  */

  TNtupleD *ntuple = (TNtupleD*)f->Get("ntuple");
  if(ntuple)
    LOG(INFO) << "ntuple var "<<ntuple->GetNvar();
  else
    LOG(INFO) << "no ntuple found";

   Double_t px, py, pz;
   Double_t pt, eta, phi, vert;
   //vert:pt:eta:phi

   ntuple->SetBranchAddress("vert",&vert);
   ntuple->SetBranchAddress("pt",&pt);
   ntuple->SetBranchAddress("eta",&eta);
   ntuple->SetBranchAddress("phi",&phi);


   size_t track_entries = (size_t)ntuple->GetEntries();
   size_t nVertex_track{track_entries};


   size_t size_double = nVertex_track*sizeof(Double_t);
   auto vertexMessage = NewMessageFor("data", 0, size_double);//FairMQMessagePtr
   auto ptMessage = NewMessageFor("data", 0, size_double);//FairMQMessagePtr
   auto etaMessage = NewMessageFor("data", 0, size_double);//FairMQMessagePtr
   auto phiMessage = NewMessageFor("data", 0, size_double);//FairMQMessagePtr

   Double_t* vtxArray = static_cast<Double_t*>(vertexMessage->GetData());
   Double_t* ptArray = static_cast<Double_t*>(ptMessage->GetData());
   Double_t* etaArray = static_cast<Double_t*>(etaMessage->GetData());
   Double_t* phiArray = static_cast<Double_t*>(phiMessage->GetData());
   
   for(Int_t i=0; i<track_entries; i++){
     //xxx
     ntuple->GetEntry(i);
     LOG(INFO) << "ntuple vertex "<<vert<<", pt: "<<pt<<", eta: "<<eta<<", phi: "<<phi;
     vtxArray[i] = vert;
     ptArray[i] = pt;
     etaArray[i] = eta;
     phiArray[i] = phi;
   }
   
   AddDataBlock(message,
	      { dataResource, DataHeader{ gDataDescriptionVertex, gDataOriginAny, DataHeader::SubSpecificationType{ 0 }, size_double }},
	      std::move(vertexMessage));
   
   AddDataBlock(message,
	      { dataResource, DataHeader{ gDataDescriptionTrackPt, gDataOriginAny, DataHeader::SubSpecificationType{ 0 }, size_double }},
	      std::move(ptMessage));
   
   AddDataBlock(message,
	      { dataResource, DataHeader{ gDataDescriptionTrackEta, gDataOriginAny, DataHeader::SubSpecificationType{ 0 }, size_double }},
	      std::move(etaMessage));
   
   AddDataBlock(message,
	      { dataResource, DataHeader{ gDataDescriptionTrackPhi, gDataOriginAny, DataHeader::SubSpecificationType{ 0 }, size_double }},
	      std::move(phiMessage));
   

	 //xxx

	 

	 /*
   
   size_t nentries = (size_t)ntuple->GetEntries();

  //decide how big the array aill be
  size_t nVertex{10};
  
  using Vertex=o2::dataformats::Vertex<>;
  
  size_t size = nVertex*sizeof(Vertex);
  
  //allocate memory for data
  auto vertexMessage = NewMessage(size);//FairMQMessagePtr
  
  Vertex* vtxArray = static_cast<Vertex*>(vertexMessage->GetData());

  /*
  for (int i=0; i<nVertex; ++i) {
    vtxArray[i] = Vertex(Point3D<float>{(float)i, (float)i, (float)i}, {2., 2., 2., 2., 2., 2.}, 1000, 1);
  }* /
  
  AddMessage(message,
	     { DataHeader{ gDataDescriptionVertex, gDataOriginAny, DataHeader::SubSpecificationType{ 0 }, size }},
	     std::move(vertexMessage));


  //decide how big the array aill be
  //size_t nTrack{10};
  size_t nTrack{nentries};

  //using TrackPar=o2::dataformats::TrackPar;
  using TrackPar=o2::track::TrackPar;
  size_t size_track = nTrack*sizeof(TrackPar);

  auto trackMessage = NewMessage(size_track);//FairMQMessagePtr

  TrackPar* track = static_cast<TrackPar*>(trackMessage->GetData());
  /*
  for (int i=0; i<nTrack; ++i) {
    track[i] = TrackPar({i*1.3, 1.4, 1.5}, {2.3, 2.4, 2.5}, 1, true);

    //constructor of TrackPar: TrackPar(const array<float, 3>& xyz, const array<float, 3>& pxpypz, int charge, bool sectorAlpha)
    }* /
  
  for (Int_t i=0;i<nentries;i++) {
    ntuple->GetEntry(i);
    LOG(INFO) << "ntuple var x "<<px<<", var y: "<<py<<", var z: "<<pz;
    track[i] = TrackPar({px, py, pz}, {2.3, 2.4, 2.5}, 1, true);
    //LOG(INFO) << "track pt "<<track[i].getTheta();
  }
  
  
  AddMessage(message,
	     { DataHeader{ gDataDescriptionTracks, gDataOriginAny, DataHeader::SubSpecificationType{ 0 }, size_track }},
	     std::move(trackMessage));

  */
  
  Send(message, "data");

}

