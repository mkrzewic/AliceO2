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

/// @file FileSink.h
///
/// @since 2018-05-31
/// @author M. Zimmermann <mazimm@cern.ch>

#ifndef ANALYSIS_H
#define ANALYSIS_H


#include "O2Device/O2Device.h"

class Analysis : public o2::Base::O2Device
{
 public:
  Analysis() = default;
  ~Analysis() override = default;

 protected:
  void Run() override;
  void InitTask() override;
  bool HandleO2frame(const byte* headerBuffer, size_t headerBufferSize, const byte* dataBuffer, size_t dataBufferSize);

 private:
  long long mDelay{ 1000 };
  long long mIterations{ 10 };
  long long mLimitOutputCharacters{ 1024 };
};

#endif /* ANALYSIS_H */
