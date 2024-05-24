
#ifndef KERNEL_PATTERN_VERIFIER_NETLINK_H
#define KERNEL_PATTERN_VERIFIER_NETLINK_H

#define VERIFIER_NETLINK_FAMILY "Ver NL Fam"
#define VERIFIER_NETLINK_VERSION 1

#define PATTERN_VERIFIER_MAX_PACKET_SIZE 4012U

//Commands
enum VerifierOperations
{
    VERIFY,
    GET_STATS,
    VERIFIER_RESET,
};

//Attributes
enum verifier_attribute_ids
{
    VER_LENGTH = 1U,
    VER_PAYLOAD,
    VER_ERRORS,
    VER_BYTES_VERIFIED,
    VER_TOTAL_BYTES_VERIFIED,

    VER_COUNT,
#define VER_MAX (VER_COUNT - 1)
};

int netlinkInit();
void netlinkDeInit();

#endif