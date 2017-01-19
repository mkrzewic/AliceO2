#include <boost/interprocess/managed_shared_memory.hpp>
namespace bi = boost::interprocess;

int main(int argc, char* argv[]) {
  bi::managed_shared_memory{ bi::open_or_create, "O2shmem", 1000 };
  bi::shared_memory_object::remove("O2shmem");
  return 0;
  }
