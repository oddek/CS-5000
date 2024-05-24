
#ifndef KERNEL_PATTERN_VERIFIER_VERIFIER_H
#define KERNEL_PATTERN_VERIFIER_VERIFIER_H

#include <linux/types.h>

typedef struct
{
    uint8_t expected;
    uint8_t prevValCounter;
    size_t errors;
    uint64_t bytesVerified;
    uint64_t totalBytesVerified;
} Verifier;

void verifierReset(Verifier* verifier);
void verifierVerify(Verifier* verifier, const uint8_t* buffer, size_t nBytes);
void verifierInit(Verifier* verifier);
void verifierDeInit(Verifier* verifier);


#endif