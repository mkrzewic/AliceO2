# @author Mikolaj Krzewicki

set(MODULE_NAME "FileSink")

O2_SETUP(NAME ${MODULE_NAME})

# Define the source and header files
set(SRCS
  src/FileSink.cxx
  src/GenerateData.cxx
  src/Analysis.cxx
)

set(HEADERS
  include/${MODULE_NAME}/FileSink.h
  include/GenerateData/GenerateData.h
  include/Analysis/Analysis.h
)

set(LIBRARY_NAME ${MODULE_NAME})
set(BUCKET_NAME O2MessageMonitor_bucket)

O2_GENERATE_LIBRARY()

# Define application
O2_GENERATE_EXECUTABLE(
  EXE_NAME runFileSink
  SOURCES src/runFileSink.cxx
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
  EXE_NAME runGenerateData
  SOURCES src/runGenerateData.cxx
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${BUCKET_NAME}
)

O2_GENERATE_EXECUTABLE(
  EXE_NAME runAnalysis
  SOURCES src/runAnalysis.cxx
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${BUCKET_NAME}
)

ADD_DEFINITIONS()

set(TEST_SRCS
  test/testFileSink.cxx
)

O2_GENERATE_TESTS(
  MODULE_LIBRARY_NAME ${LIBRARY_NAME}
  BUCKET_NAME ${BUCKET_NAME}
  TEST_SRCS ${TEST_SRCS}
)

install(FILES test/runFileSink.sh DESTINATION bin PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
install(FILES test/FileSink.json DESTINATION share/config PERMISSIONS OWNER_READ OWNER_WRITE GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
