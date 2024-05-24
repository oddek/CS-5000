
#include "../common/common.h"
#include "Verifier.h"
#include <linux/module.h>

#define MIN_PATTERN_VALUE 1U
#define MAX_PATTERN_VALUE 250U


void verifierReset(Verifier* verifier)
{
    verifier->expected = MIN_PATTERN_VALUE;
    verifier->prevValCounter = 0U;
    verifier->errors = 0U;
    verifier->bytesVerified = 0U;
}

void verifierVerify(Verifier* verifier, const uint8_t* buffer, size_t nBytes)
{
    // pr_info("Buf size: %u, Cont: %u\n", nBytes, buffer[0]);
    
    for ( size_t i = 0U; i < nBytes; i++ )
    {
        if ( verifier->expected != buffer[i] )
        {
            // pr_info( "Err - Exp: %u, Rec: %u\n", verifier->expected, buffer[i] );
            verifier->errors++;
            verifier->expected = buffer[i];
        }

        else
        {
            // pr_info( "Succ - Exp: %u, Rec: %u\n", verifier->expected, buffer[i] );
            verifier->bytesVerified++;
            verifier->totalBytesVerified++;
            verifier->prevValCounter++;

            // if ( verifier->prevValCounter == sizeof( uint64_t ) )
            if ( verifier->prevValCounter == 1U )
            {
                verifier->prevValCounter = 0U;
                verifier->expected = ( verifier->expected == MAX_PATTERN_VALUE ) ? MIN_PATTERN_VALUE : verifier->expected + 1;
            }
        }
    }
}

void verifierInit(Verifier* verifier)
{
    verifierReset(verifier);
}

void verifierDeInit(Verifier* verifier)
{

}
