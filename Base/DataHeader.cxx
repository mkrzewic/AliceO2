#include "DataHeader.h"

const char* AliceO2::Base::DataHeader::sMagicString = "O2 ";

using namespace AliceO2::Base;

//possible data origins
const DataOrigin gDataOriginAny    ("***");
const DataOrigin gDataOriginInvalid("   ");
const DataOrigin gDataOriginTPC    ("TPC");
const DataOrigin gDataOriginTRD    ("TRD");
const DataOrigin gDataOriginTOF    ("TOF");

//possible data types
const DataDescription gDataDescriptionAny     ("***************");
const DataDescription gDataDescriptionInvalid ("               ");
const DataDescription gDataDescriptionRawData ("RAWDATA        ");
const DataDescription gDataDescriptionClusters("CLUSTERS       ");
const DataDescription gDataDescriptionTracks  ("TRACKS         ");

//possible serialization types
const PayloadSerialization gPayloadSerializationAny    ("*******");
const PayloadSerialization gPayloadSerializationInvalid("       ");
const PayloadSerialization gPayloadSerializationNone   ("NONE   ");
const PayloadSerialization gPayloadSerializationROOT   ("ROOT   ");
const PayloadSerialization gPayloadSerializationFlatBuf("FLATBUF");

//_________________________________________________________________________________________________
AliceO2::Base::DataHeader::DataHeader()
  : magicStringInt(*reinterpret_cast<const int32_t*>(sMagicString))
  , dataOriginInt()
  , dataDescriptionInt()
  , payloadSerializationInt()
  , subSpecification()
  , flags()
  , headerVersion(sVersion)
  , headerSize()
  , payloadSize()
{
}

//_________________________________________________________________________________________________
AliceO2::Base::DataHeader::DataHeader(const DataHeader& that)
  : magicStringInt(that.magicStringInt)
  , dataOriginInt(that.dataOriginInt)
  , dataDescriptionInt{that.dataDescriptionInt[0], that.dataDescriptionInt[1]}
  , payloadSerializationInt(that.payloadSerializationInt)
  , subSpecification(that.subSpecification)
  , flags(that.flags)
  , headerVersion(that.headerVersion)
  , headerSize(that.headerSize)
  , payloadSize(that.payloadSize)
{
}

//_________________________________________________________________________________________________
void AliceO2::Base::DataHeader::print() const
{
  printf("Data header version %i, flags: %i\n",headerVersion, flags);
  printf("  origin       : %s\n", dataOrigin);
  printf("  description  : %s\n", dataDescription);
  printf("  serialization: %s\n", payloadSerialization);
  printf("  sub spec.    : %lli\n", subSpecification);
  printf("  header size  : %i\n", headerSize);
  printf("  payloadSize  : %i\n", payloadSize);
}

//_________________________________________________________________________________________________
DataHeader& AliceO2::Base::DataHeader::operator=(const DataHeader& that)
{
  magicStringInt = that.magicStringInt;
  dataOriginInt = that.dataOriginInt;
  dataDescriptionInt[0] = that.dataDescriptionInt[0];
  dataDescriptionInt[1] = that.dataDescriptionInt[1];
  payloadSerializationInt = that.payloadSerializationInt;
  subSpecification = that.subSpecification;
  flags = that.flags;
  headerVersion = that.headerVersion;
  headerSize = that.headerSize;
  payloadSize = that.payloadSize;
  return *this;
}

//_________________________________________________________________________________________________
bool AliceO2::Base::DataHeader::operator==(const DataOrigin& that)
{
  return (that.dataOriginInt == gDataOriginAny.dataOriginInt ||
          that.dataOriginInt == dataOriginInt );
}

//_________________________________________________________________________________________________
bool AliceO2::Base::DataHeader::operator==(const DataDescription& that)
{
  return (that.dataDescriptionInt == gDataDescriptionAny.dataDescriptionInt ||
          that.dataDescriptionInt == dataDescriptionInt );
}

//_________________________________________________________________________________________________
bool AliceO2::Base::DataHeader::operator==(const PayloadSerialization& that)
{
  return (that.payloadSerializationInt == gPayloadSerializationAny.payloadSerializationInt ||
          that.payloadSerializationInt == payloadSerializationInt );
}

//_________________________________________________________________________________________________
bool AliceO2::Base::DataHeader::operator==(const DataHeader& that)
{
  return( magicStringInt == that.magicStringInt &&
          dataOriginInt == that.dataOriginInt &&
          dataDescriptionInt == that.dataDescriptionInt &&
          subSpecification == that.subSpecification );
}
