
#include "../common/common.h"
#include "Generator.h"
#include <linux/string.h>

#define MIN_PATTERN_VALUE 1U
#define MAX_PATTERN_VALUE 250U

void generatorReset(Generator* generator)
{
    generator->currentValue = MIN_PATTERN_VALUE;
    generator->sameValueCounter = sizeof( uint64_t );
    generator->currentMessageSize = 0U;
    generator->currentMessageBytesGenerated = 0U;
}

void generatorGenerate(Generator* generator, uint8_t* buffer, size_t nBytes)
{
    size_t bytesLeftToGenerate = nBytes;

    while ( bytesLeftToGenerate != 0U )
    {
        *buffer = generator->currentValue;
        bytesLeftToGenerate--;
        buffer++;
        generator->currentValue = ( generator->currentValue == MAX_PATTERN_VALUE ) ? MIN_PATTERN_VALUE : generator->currentValue + 1U;
    }
}

void generatorGenerate2(Generator* generator, uint8_t* buffer, size_t nBytes)
{
    size_t bytesLeftToGenerate = nBytes;

    while ( bytesLeftToGenerate != 0U )
    {
        if ( bytesLeftToGenerate >= sizeof( uint64_t ) )
        {
            memset( buffer, generator->currentValue, generator->sameValueCounter );

            bytesLeftToGenerate -= generator->sameValueCounter;
            generator->currentValue = ( ( generator->currentValue == MAX_PATTERN_VALUE ) ? MIN_PATTERN_VALUE : ( generator->currentValue + 1U ) );
            buffer += generator->sameValueCounter;
            generator->sameValueCounter = sizeof( uint64_t );
        }

        else
        {
            const size_t bytesToWrite = ( bytesLeftToGenerate < generator->sameValueCounter ) ? bytesLeftToGenerate : generator->sameValueCounter;
            memset( buffer, generator->currentValue, bytesToWrite );

            generator->sameValueCounter -= bytesToWrite;
            bytesLeftToGenerate -= bytesToWrite;
            buffer += bytesToWrite;

            if ( generator->sameValueCounter == 0U )
            {
                generator->sameValueCounter = sizeof( uint64_t );
                generator->currentValue = ( generator->currentValue == MAX_PATTERN_VALUE ) ? MIN_PATTERN_VALUE : generator->currentValue + 1U;
            }
        }
    }
}

void generatorInit(Generator* generator)
{
    generatorReset(generator);
}