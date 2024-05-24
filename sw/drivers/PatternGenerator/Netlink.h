
#ifndef PATTERN_GENERATOR_NETLINK_H
#define PATTERN_GENERATOR_NETLINK_H

#define GENERATOR_NETLINK_FAMILY "Gen NL Fam"
#define GENERATOR_NETLINK_VERSION 1

#define PATTERN_GENERATOR_MAX_PACKET_SIZE 4012U

#include <linux/types.h>

typedef enum 
{
    Packet_MetaData_None,
    Packet_MetaData_Only,
    Packet_MetaData_First,
    Packet_MetaData_Last
}PacketMetaData;

//Commands
enum GeneratorOperations
{
    GENERATE,
    RESET,
    SETUP
};

//Attributes
enum generate_attribute_ids
{
    SAI_LENGTH = 1U,
    SAI_PAYLOAD,
    SAI_META_DATA,
    SAI_TOTAL_LENGTH,

    SAI_COUNT,
#define SAI_MAX (SAI_COUNT - 1)
};


bool netlinkTransmitBuffer(const uint8_t* buffer, size_t nBytes, PacketMetaData metaData, size_t totalMessageSize);
int netlinkInit();
void netlinkDeInit();


#endif