#include <boost/interprocess/managed_shared_memory.hpp>
#include <iostream>
#include <cstring>
#include <zmq.h>
#include <cstdlib> //std::system
#include <cstddef>
#include <cassert>
#include <utility>
#include <chrono>
#include <thread>
#include "../include/O2Device/SharedMemory.h"

using namespace o2;
int main(int argc, char* argv[]) {
  namespace bi = boost::interprocess;

  //Construct managed shared memory
  SharedMemory::Manager segman(20000000);

  void* zmqContext = zmq_ctx_new();
  //RECEIVER
  void* receiver = zmq_socket(zmqContext, ZMQ_PULL);
  zmq_connect(receiver, "tcp://localhost:2223");
  //zmq_setsockopt(receiver, ZMQ_SUBSCRIBE, "", 0);
  //int timeout = 1000;
  //zmq_setsockopt(receiver, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
  //zmq_setsockopt(receiver, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));

  const size_t messageLen = 50000;
  char message[messageLen];

  unsigned int i = 0;
  {
    int rc = zmq_recv(receiver, message, messageLen, 0);
    ++i;
    if (rc==0) return 0;

    if (rc<0) {printf("rc: %i, err: %s message: %s\n",rc, zmq_strerror(errno), &message[0]);}
  }
  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  int niter = 1000000;
  //for  (int i=0; i<niter; ++i) {
  while(true) {
    int rc = zmq_recv(receiver, message, messageLen, 0);
    if (rc==0) break;
    if (rc<0) {printf("rc: %i, err: %s message: %s\n",rc, zmq_strerror(errno), &message[0]);}
    ++i;
    //if (rc>0) message[rc] = '\0';

    SharedMemory::HandleType handle = *reinterpret_cast<SharedMemory::HandleType*>(&message[0]);
    auto block = segman.getBlock(handle,0);

    //std::cout << "handle: " << handle << " ptr " << segman.Segment()->get_address_from_handle(handle) << std::endl;
    //printf("data: %s\n", block->data());
  }

  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  std::time_t end_time = std::chrono::system_clock::to_time_t(end);
  std::cout << "finished receiving " << i << " messages in " << elapsed_seconds.count() << "s\n";

  zmq_close(receiver);
  zmq_ctx_term(zmqContext);

  return 0;
  }
