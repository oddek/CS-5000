

#include "H2f.h"
#include "HardwareCodec.h"
#include "HwDecoder.h"
#include "HwEncoder.h"
#include "UserspacePatternGenerator.h"
#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cstdlib>
#include <fmt/ranges.h>
#include <iostream>
#include <vector>

void decoderTestDecimation()
{
    HardwareCodec<CodecType::Decode> hardwareEncoder{4};

    assert(hardwareEncoder.isEmpty());
    assert(!hardwareEncoder.isFull());

//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//    fmt::println("Writing {:#08X}", 0x11112222U);
//    hardwareEncoder.putWord(0x11112222U);
//    usleep(10000);
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//    assert(hardwareEncoder.isEmpty());
//    fmt::println("Writing {:#08X}", 0x33334444U);
//    hardwareEncoder.putWord(0x33334444U);
//    usleep(10000);
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//    assert(hardwareEncoder.isEmpty());
//    fmt::println("Writing {:#08X}", 0x55556666U);
//    hardwareEncoder.putWord(0x55556666U);
//    usleep(10000);
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//    assert(hardwareEncoder.isEmpty());
//    fmt::println("Writing {:#08X}", 0x77778888U);
//    hardwareEncoder.putWord(0x77778888U);
//    usleep(10000);
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//    assert(!hardwareEncoder.isEmpty());
//    fmt::println("Read {:#08X}", hardwareEncoder.getWord());
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//    assert(hardwareEncoder.isEmpty());
//
//    hardwareEncoder.putWord(0x11112222U);
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//    assert(hardwareEncoder.isEmpty());
//    hardwareEncoder.putWord(0x33334444U);
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//    assert(hardwareEncoder.isEmpty());
//    hardwareEncoder.putWord(0x55556666U);
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//    assert(hardwareEncoder.isEmpty());
//    hardwareEncoder.putWord(0x77778888U);
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//    assert(!hardwareEncoder.isEmpty());
//
//    hardwareEncoder.putWord(0x9999AAAA);
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//    hardwareEncoder.putWord(0xBBBBCCCC);
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//    hardwareEncoder.putWord(0xDDDDEEEE);
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//    hardwareEncoder.putWord(0xFFFFAAAAU);
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//
//
//    fmt::println("{:#08X}", hardwareEncoder.getWord());
//    fmt::println("{:#08X}", hardwareEncoder.getWord());
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//    fmt::println("{:#08X}", hardwareEncoder.getWord());
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//
//    fmt::println("{:#08X}", hardwareEncoder.getWord());
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//
//    fmt::println("{:#08X}", hardwareEncoder.getWord());
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//
//    fmt::println("{:#08X}", hardwareEncoder.getWord());
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//
//    fmt::println("{:#08X}", hardwareEncoder.getWord());
//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//    assert(hardwareEncoder.isEmpty());


    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
    fmt::println("Empty: {}", hardwareEncoder.isEmpty());

    hardwareEncoder.putWord(0x01010101U);
    hardwareEncoder.putWord(0x01010101U);
    hardwareEncoder.putWord(0x02020202U);
    hardwareEncoder.putWord(0x02020202U);
    hardwareEncoder.putWord(0x03030303U);
    hardwareEncoder.putWord(0x03030303U);
    hardwareEncoder.putWord(0x04040404U);
    hardwareEncoder.putWord(0x04040404U);

    fmt::println("{:#08X}", hardwareEncoder.getWord());
    fmt::println("{:#08X}", hardwareEncoder.getWord());

    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
    fmt::println("Empty: {}", hardwareEncoder.isEmpty());

    hardwareEncoder.putWord(0x01010101U);
    hardwareEncoder.putWord(0x02020202U);
    hardwareEncoder.putWord(0x03030303U);
    hardwareEncoder.putWord(0x04040404U);
    hardwareEncoder.putWord(0x05050505U);
    hardwareEncoder.putWord(0x06060606U);
    hardwareEncoder.putWord(0x07070707U);
    hardwareEncoder.putWord(0x08080808U);

    fmt::println("{:#08X}", hardwareEncoder.getWord());
    fmt::println("{:#08X}", hardwareEncoder.getWord());

    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
    fmt::println("Empty: {}", hardwareEncoder.isEmpty());

//    0x01010101U;

//    fmt::println("{:#08X}", hardwareEncoder.getWord());
//    fmt::println("{:#08X}", hardwareEncoder.getWord());

//    0x01010101U;
//    0x02020202U;
//    0x03030303U;
//    0x04040404U;
//
//    0x01020304;

//    fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
//    fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
//    fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//
//    fmt::println("{:#08X}", hardwareEncoder.getWord());

}

void encoderTestInterpolation()
{
    {
        HardwareCodec<CodecType::Encode> hardwareEncoder{8};

        assert(hardwareEncoder.isEmpty());
        assert(!hardwareEncoder.isFull());

        hardwareEncoder.putWord(0x01020304U);
        fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
        fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
        fmt::println("Empty: {}", hardwareEncoder.isEmpty());
//        assert(!hardwareEncoder.isEmpty());
        fmt::println("{:#08X}", hardwareEncoder.getWord());

//        assert(!hardwareEncoder.isEmpty());
        fmt::println("Writes: {}", hardwareEncoder.getWritesPerformed());
        fmt::println("Reads: {}", hardwareEncoder.getReadsPerformed());
        fmt::println("Empty: {}", hardwareEncoder.isEmpty());
        fmt::println("{:#08X}", hardwareEncoder.getWord());

//        assert(!hardwareEncoder.isEmpty());
        fmt::print("{:#08X}", hardwareEncoder.getWord());
        assert(!hardwareEncoder.isEmpty());
        fmt::println("{:#08X}", hardwareEncoder.getWord());
        assert(!hardwareEncoder.isEmpty());
        fmt::print("{:#08X}", hardwareEncoder.getWord());
        assert(!hardwareEncoder.isEmpty());
        fmt::println("{:#08X}", hardwareEncoder.getWord());
        assert(!hardwareEncoder.isEmpty());
        fmt::print("{:#08X}", hardwareEncoder.getWord());
        assert(!hardwareEncoder.isEmpty());
        fmt::println("{:#08X}", hardwareEncoder.getWord());
        assert(hardwareEncoder.isEmpty());
    }
    {
        HardwareCodec<CodecType::Encode> hardwareEncoder{4};

        assert(hardwareEncoder.isEmpty());
        assert(!hardwareEncoder.isFull());

        hardwareEncoder.putWord(0x11223344U);
        assert(!hardwareEncoder.isEmpty());
        assert(hardwareEncoder.getWord() == 0x11111111U);
//        fmt::println("{:#08X}", hardwareEncoder.getWord());
        assert(!hardwareEncoder.isEmpty());
//        fmt::println("{:#08X}", hardwareEncoder.getWord());
        assert(hardwareEncoder.getWord() == 0x22222222U);
        assert(!hardwareEncoder.isEmpty());
        fmt::println("{:#08X}", hardwareEncoder.getWord());
//        assert(hardwareEncoder.getWord() == 0x33333333U);
        assert(!hardwareEncoder.isEmpty());
        fmt::println("{:#08X}", hardwareEncoder.getWord());
//        assert(hardwareEncoder.getWord() == 0x44444444U);
        assert(hardwareEncoder.isEmpty());

        hardwareEncoder.putWord(0x01020304U);
        assert(!hardwareEncoder.isEmpty());
        //        assert(hardwareEncoder.getWord() == 0x01010101U);
        fmt::println("{:#08X}", hardwareEncoder.getWord());
        assert(!hardwareEncoder.isEmpty());
        fmt::println("{:#08X}", hardwareEncoder.getWord());
        //        assert(hardwareEncoder.getWord() == 0x02020202U);
        assert(!hardwareEncoder.isEmpty());
        fmt::println("{:#08X}", hardwareEncoder.getWord());
        //        assert(hardwareEncoder.getWord() == 0x03030303U);
        assert(!hardwareEncoder.isEmpty());
        fmt::println("{:#08X}", hardwareEncoder.getWord());
        //        assert(hardwareEncoder.getWord() == 0x44444444U);
        assert(hardwareEncoder.isEmpty());
    }

    {
        HardwareCodec<CodecType::Encode> hardwareEncoder{2};
        assert(hardwareEncoder.isEmpty());
        assert(!hardwareEncoder.isFull());
        hardwareEncoder.putWord(0x11223344U);
        assert(!hardwareEncoder.isEmpty());
        assert(hardwareEncoder.getWord() == 0x11112222U);
        assert(!hardwareEncoder.isEmpty());
        assert(hardwareEncoder.getWord() == 0x33334444U);
        assert(hardwareEncoder.isEmpty());
    }
    {
        HardwareCodec<CodecType::Encode> hardwareEncoder{1};
        assert(hardwareEncoder.isEmpty());
        assert(!hardwareEncoder.isFull());
        hardwareEncoder.putWord(0x11223344U);
        assert(!hardwareEncoder.isEmpty());
        assert(hardwareEncoder.getWord() == 0x11223344U);
        assert(hardwareEncoder.isEmpty());
    }
}

void encoderTest2(size_t iterations)
{
    srand(55);
    UserspacePatternGenerator generator;
    UserspacePatternVerifier verifier{1};
    HwEncoder encoder{1};

    for(size_t i = 0; i < iterations; i++)
    {
        generator.reset();
        verifier.reset();
        encoder.reset();
        const size_t totalMessageBytes = (rand() % 64_kiB) + 64_kiB;
        generator.generateAndEnqueueMessage((totalMessageBytes));

        while(true)
        {
            const auto packet = generator.get();
            encoder.put(packet);
            encoder.update();
            const auto encodedPacket = encoder.get();
            verifier.put(encodedPacket);

            if(packet.getMetaData() == Packet<>::MetaData::LastPacket)
            {
                break;
            }
        }
        encoder.printStats();
        const auto verifierStats = verifier.getStats();
        fmt::println("Verifier - "
                     "Sent: {:>11}  "
                     "Rcv: {:>11}  "
                     "Fifo: {:>11}  "
                     "Err: {:>11}",
                     0,
                     convertBytesToString(verifierStats.bytesVerified), 0,
                     verifierStats.errors);
    }
}

void encoderTest(size_t iterations)
{
    srand(55);
    UserspacePatternGenerator generator;
    UserspacePatternVerifier verifier{1};
    HwEncoder encoder{1};

        for(size_t i = 0; i < iterations; i++)
        {
            const uint16_t nBytes = rand() % 4096;
            generator.generateAndEnqueueMessage((nBytes));
            const auto packet = generator.get();
            assert(packet.getMetaData() == Packet<>::MetaData::LastPacket);
            encoder.put(packet);
            encoder.update();
            const auto encodedPacket = encoder.get();
            verifier.put(encodedPacket);
//            assert(verifier.getErrors() == 0U);
            encoder.printStats();
        }
}

void decoderTest(size_t iterations)
{
    srand(55);
    UserspacePatternGenerator generator;
    HwDecoder decoder{};

    for (size_t i = 0; i < iterations; i++)
    {
        const uint16_t nBytes = rand() % 4096;
        generator.generateAndEnqueueMessage((nBytes));
        const auto packet = generator.get();
        assert(packet.getMetaData() == Packet<>::MetaData::LastPacket);
        decoder.decode(packet);
        decoder.printStats();
    }
}

template <CodecType Type> void continousTest(HardwareCodec<Type>& hardwareCodec)
{
    //    HardwareCodec<CodecType::Decode> hardwareDecoder;
    std::vector<uint32_t> input(4096);
    std::vector<uint32_t> expected(4096);
    std::vector<uint32_t> output;

    std::generate(input.begin(), input.end(), [] {
        static size_t n = 0;
        return n++;
    });
    std::generate(expected.begin(), expected.end(), [] {
        static size_t n = 0;
        return n++;
    });

    while (true)
    {
        while (!hardwareCodec.isEmpty())
        {
            output.push_back(hardwareCodec.getWord());
        }

        while (!hardwareCodec.isFull() && !input.empty())
        {
            hardwareCodec.putWord(input.front());
            input.erase(input.begin());
        }

        if (input.empty() && hardwareCodec.isEmpty())
        {
            break;
        }
    }
    assert(expected == output);
}

template <CodecType Type> void simpleTest(HardwareCodec<Type>& hardwareCodec)
{

    assert(hardwareCodec.isEmpty());
    assert(!hardwareCodec.isFull());

    //    assert(hardwareCodec.getWritesPerformed() == 0U);
    hardwareCodec.putWord(1);
    //    assert(hardwareCodec.getWritesPerformed() == 1U);

    assert(!hardwareCodec.isEmpty());

    hardwareCodec.putWord(2);
    //    assert(hardwareCodec.getWritesPerformed() == 2U);
    hardwareCodec.putWord(3);
    //    assert(hardwareCodec.getWritesPerformed() == 3U);

    //    assert(hardwareCodec.getReadsPerformed() == 0U);
    assert(hardwareCodec.getWord() == 1);
    //    assert(hardwareCodec.getReadsPerformed() == 1U);
    assert(hardwareCodec.getWord() == 2);
    //    assert(hardwareCodec.getReadsPerformed() == 2U);
    assert(hardwareCodec.getWord() == 3);
    //    assert(hardwareCodec.getReadsPerformed() == 3U);

    assert(hardwareCodec.isEmpty());

    hardwareCodec.putWord(3);

    assert(hardwareCodec.getWord() == 3);

    assert(hardwareCodec.isEmpty());

    for (size_t i = 0; i < hardwareCodec.CAPACITY; i++)
    {
        hardwareCodec.putWord(i);
    }

    assert(hardwareCodec.isFull());
    assert(!hardwareCodec.isEmpty());

    for (size_t i = 0; i < hardwareCodec.CAPACITY; i++)
    {
        assert(hardwareCodec.getWord() == i);
    }

    assert(hardwareCodec.isEmpty());
}

int main()
{
    std::cout << " Testing HW Codecs..\n";
    H2f::init();
    HardwareCodec<CodecType::Decode> hardwareDecoder;
    HardwareCodec<CodecType::Encode> hardwareEncoder;

    fmt::println("SimpleTest");
//    simpleTest(hardwareDecoder);
//    simpleTest(hardwareEncoder);

//    fmt::println("ContinousTest: HwDecoder");
//    continousTest(hardwareDecoder);
//    fmt::println("ContinousTest: HwEncoder");
//    continousTest(hardwareEncoder);
//
//    fmt::println("DecoderTests..");
//    decoderTest(10);
//    encoderTest(10);
//    encoderTest2(10);

    fmt::println("Decimation test..");
    decoderTestDecimation();
    fmt::println("Interpolation test..");
    encoderTestInterpolation();

    std::cout << "All tests passed\n";
    return 0;
}