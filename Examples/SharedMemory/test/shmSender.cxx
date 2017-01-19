#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/smart_ptr/shared_ptr.hpp>
#include <boost/interprocess/containers/list.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <iostream>
#include <cstring>
#include <zmq.h>
#include <cstdlib> //std::system
#include <cstddef>
#include <cassert>
#include <utility>
#include <chrono>
#include <thread>
#include "O2Device/SharedMemory.h"

using namespace o2;
namespace bi = boost::interprocess;

int main(int argc, char* argv[]) {

	void* zmqContext = zmq_ctx_new();
	void* sender = zmq_socket(zmqContext, ZMQ_PUSH);
	int rc = zmq_bind(sender, "tcp://*:2223");
	if (rc<0) printf("zmq connection err: %s\n",zmq_strerror(errno));

  //Construct managed shared memory
  SharedMemory::Manager segman(20000000);

  int nErrors = 0;
  size_t mMsgSize = 50000;
  std::string shmPointerID;

  int niter = 1000000;
  int i = 0;
  for (i=0; i<niter; ++i) {
    auto blockPtr = segman.allocate(mMsgSize);
    while (!blockPtr) {
      printf("waiting\n");
      segman.waitForMemory(boost::posix_time::seconds(10));
      blockPtr = segman.allocate(mMsgSize);
    }

    std::string datastr{"some data "};
    datastr+=std::to_string(blockPtr->getID());
    strcpy((char*)blockPtr->data(),datastr.c_str());

    auto handle = blockPtr->getHandle(segman);
    rc = zmq_send(sender, &handle, sizeof(handle), 0);

    if (rc<0) printf("send rc %i, error: %s\n", rc, zmq_strerror(errno));
    if (rc<0) ++nErrors;
  }

  zmq_send(sender, 0, 0 ,0);
  printf("sent messages: %d, errors: %d\n",i,nErrors);
	zmq_close(sender);
	zmq_ctx_term(zmqContext);
	return 0;
}
