// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "Framework/ConfigParamSpec.h"
#include "FairLogger.h"
#include <lzma.h>
#include <iomanip>

using namespace o2::framework;

// This has to be declared before including runDataProcessing.h
void customize(std::vector<ConfigParamSpec>& workflowOptions) {
  workflowOptions.push_back({ "num-zippers", VariantType::Int, 1,
                              {"Number of compressors"} });
  workflowOptions.push_back({ "sleep", VariantType::Int, 0,
                              {"Sleep (secs) at each producer iteration"} });
  workflowOptions.push_back({ "stop-at-iter", VariantType::Int, -1,
                              {"Stop at iterations (-1 == don't stop)"} });
  workflowOptions.push_back({ "how-many", VariantType::Int, 20000,
                              {"How many int to generate per iteration"} });
  workflowOptions.push_back({ "algo", VariantType::String, "lzma",
                              {"What compression algo to use (lzma, cat)"} });
}

#include "Framework/runDataProcessing.h"
#include "Framework/ParallelContext.h"
#include "Framework/ControlService.h"

template <typename T>
std::stringstream catAry(const T &iterable) {
  std::stringstream buf;
  for (auto i : iterable) {
    if (buf.tellp()) buf << ", ";
    buf << i;
  }
  return buf;
}

std::stringstream catLabels(ProcessingContext &ctx) {
  std::list<const char *> labels;
  for (auto i : ctx.inputs()) {
    labels.push_back((i.spec)->binding.c_str());
  }
  return catAry(labels);
}

void defaultAmend(DataProcessorSpec &spec, size_t idx) {
  LOG(WARN) << "Using default amender for device " << spec.name << FairLogger::endl;
  for (auto &i : spec.inputs) {
    i.subSpec = idx;
  }
  for (auto &o : spec.outputs) {
    o.subSpec = idx;
  }
}

size_t cat(const uint8_t *inbuf, size_t sz, uint8_t *outdata) {
  memcpy(outdata, inbuf, sz);
  return sz;
}

size_t lzmaCompress(const uint8_t *inbuf, size_t sz, uint8_t *outdata) {
  lzma_stream lzs = LZMA_STREAM_INIT;
  lzma_action lza = LZMA_RUN;
  lzma_ret ret = lzma_easy_encoder(&lzs, 9, LZMA_CHECK_NONE);  // xz -9
  assert(ret == LZMA_OK);

  // Mini output buffer (will dump on outdata when full/finished)
  uint8_t outbuf[BUFSIZ];

  // Init LZMA state machine
  lzs.next_in = nullptr;
  lzs.avail_in = 0;
  lzs.next_out = outbuf;
  lzs.avail_out = sizeof(outbuf);

  // Total bytes read
  size_t nRead = 0;
  size_t nWritten = 0;

  while (true) {

    if (lzs.avail_in == 0 && nRead < sz) {
      // Fill input buffer if empty
      lzs.next_in = &inbuf[nRead];
      lzs.avail_in = sz-nRead; // all remaining bytes
      if (lzs.avail_in > BUFSIZ) lzs.avail_in = BUFSIZ; // truncate at BUFSIZ
      nRead += lzs.avail_in;
      if (nRead == sz) lza = LZMA_FINISH;
    }
    ret = lzma_code(&lzs, lza);
    if (lzs.avail_out == 0 || ret == LZMA_STREAM_END) {
      // We need to empty the output data
      size_t nToWrite = sizeof(outbuf) - lzs.avail_out;
      memcpy(&outdata[nWritten], outbuf, nToWrite);
      nWritten += nToWrite;

      // Reset output reference
      lzs.next_out = outbuf;
      lzs.avail_out = sizeof(outbuf);
    }

    if (ret != LZMA_OK) {
      assert(ret == LZMA_STREAM_END);
      break;
    }
  }
  lzma_end(&lzs);
  LOG(INFO) << "LZMA: in: " << sz << " / out: " << nWritten
            << " / ratio: " << std::setprecision(3)
            << ((float)nWritten/(float)sz) << FairLogger::endl;
  return nWritten;
}

WorkflowSpec defineDataProcessing(ConfigContext const &config) {

  WorkflowSpec w;

  auto numZippers = config.options().get<int>("num-zippers");
  auto sleepSec = config.options().get<int>("sleep");
  auto stopAtIterations = config.options().get<int>("stop-at-iter");
  auto howMany = config.options().get<int>("how-many");
  auto algo = config.options().get<std::string>("algo");

  std::function<size_t(const uint8_t *inbuf, size_t sz, uint8_t *outdata)> compressFn = nullptr;

  if (algo == "lzma") {
    compressFn = lzmaCompress;
  }
  else {
    assert(algo == "cat");
    compressFn = cat;
  }

  // Data producer: we produce chunks of random data
  w.push_back({
    "Producer",
    Inputs{},
    { OutputSpec{"TST", "ARR"} },
    AlgorithmSpec{
      AlgorithmSpec::InitCallback{
        [sleepSec, howMany, stopAtIterations](InitContext &setup) {
          auto iterCount = std::make_shared<int>(0);
          auto askedToQuit = std::make_shared<bool>(false);

          return [iterCount, askedToQuit, sleepSec, howMany, stopAtIterations](ProcessingContext &ctx) {

            if (*askedToQuit) return;

            if (sleepSec > 0) {
              LOG(INFO) << "Producer is sleeping..." << FairLogger::endl;
              sleep(sleepSec);
            }

            // Send random data to the single output (it's parallelized over time)
            auto prodData = ctx.outputs().make<int>(Output{"TST", "ARR"}, howMany);
            LOG(INFO) << "Producer is sending data" << FairLogger::endl;
            int i = 0;
            for (auto &p : prodData) {
              if (i == 0) { p = *iterCount; }
              else        { p = rand() % 1000; }
              i++;
            }
            ++(*iterCount);

            if (stopAtIterations > 0 && *iterCount >= stopAtIterations) {
              LOG(WARNING) << "Reached " << (*iterCount) << " iterations: stopping" << FairLogger::endl;
              *askedToQuit = true;
              ctx.services().get<ControlService>().readyToQuit(true);
            }
          };
        }
      }
    }
  });

  // Parallel compressors
  w.push_back(timePipeline(
    {
      "Compressor",
      { InputSpec{{"arrCons"}, "TST", "ARR"} },
      { OutputSpec{"TST", "ZIP"} },
      AlgorithmSpec{

        AlgorithmSpec::InitCallback{
          [compressFn](InitContext &setup) {
            return [compressFn](ProcessingContext &ctx) {

              auto inp = ctx.inputs().get("arrCons");
              auto arrowCons = DataRefUtils::as<int>(inp);
              auto arrowBytes = (o2::header::get<o2::header::DataHeader*>(inp.header))->payloadSize;
              LOG(INFO) << "Compressor received " << arrowBytes << " bytes " << FairLogger::endl;

              // We create a buffer where to place our data, and compress there
              size_t outbufSz = arrowBytes * 2;
              uint8_t *outbuf = new uint8_t[outbufSz];
              size_t nWritten = compressFn((const uint8_t *)(&arrowCons[0]), arrowBytes, outbuf);

              // Framework adopts our memory chunk and disposes of it
              ctx.outputs().adoptChunk(Output{"TST", "ZIP"},
                                       reinterpret_cast<char *>(outbuf), nWritten,
                                       &o2::header::Stack::freefn, nullptr);
            };
          }
        }
      }
    },
    numZippers
  ));

  // Single writer
  w.push_back({
    "Writer",
    { InputSpec{"zipCons", "TST", "ZIP"} },
    {},
    AlgorithmSpec{
      AlgorithmSpec::InitCallback{
        [](InitContext &setup) {
          auto fout = std::make_shared<std::fstream>("/tmp/dpl_writer.txt", std::fstream::out);
          return [fout](ProcessingContext &ctx) {
            for (auto &inp : ctx.inputs()) {
              auto zipMsg = &DataRefUtils::as<char>(inp)[0];
              unsigned int zipSz = (o2::header::get<o2::header::DataHeader*>(inp.header))->payloadSize;
              LOG(INFO) << "Writer has received " << zipSz
                        << " bytes, writing to disk" << FairLogger::endl;

              // We keep the format very simple: an int with the size and the buffer
              (*fout).write((const char *)(&zipSz), 4);
              (*fout).write(zipMsg, zipSz);
              (*fout).flush();
            }
          };
        }
      }
    }
  });

  return w;
}
