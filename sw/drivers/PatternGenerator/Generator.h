

#ifndef PATTERN_GENERATOR_GENERATOR_H
#define PATTERN_GENERATOR_GENERATOR_H

#include <linux/types.h>

typedef enum
{
    NotDone,
    Done
} GeneratorState;

typedef struct
{
    uint8_t currentValue;
    uint8_t sameValueCounter;
    size_t currentMessageSize;
    size_t currentMessageBytesGenerated;
} Generator;

void generatorReset(Generator* generator);
void generatorGenerate(Generator* generator, uint8_t* buffer, size_t nBytes);
void generatorGenerate2(Generator* generator, uint8_t* buffer, size_t nBytes);
void generatorInit(Generator* generator);






#endif