SET(LIB_NAME Components)
project(${LIB_NAME})

add_library(${LIB_NAME}
        Source/RxDriver.cpp
        Source/TxDriver.cpp
        Source/ArgParser.cpp
        Source/BitrateCalc.cpp
        Source/ContinuousBuffer.cpp
        Source/Cpu.cpp
        Source/CyclicTask.cpp
        Source/DataMover.cpp
        Source/HwDecoder.cpp
        Source/DurationTimer.cpp
        Source/HwEncoder.cpp
        Source/H2f.cpp
        Source/HardwareCodec.cpp
        Source/SwEncoder.cpp
        Source/KernelPatternGenerator.cpp
        Source/KernelPatternVerifier.cpp
        Source/PacketQueue.cpp
        Source/PatternGenerator.cpp
        Source/PatternVerifier.cpp
        Source/RingBuffer.cpp
        Source/RxPipeline.cpp
        Source/SingleWorker.cpp
        Source/TxPipeline.cpp
        Source/Ui.cpp
        Source/UserspacePatternGenerator.cpp
        Source/UserspacePatternVerifier.cpp
        Source/cma_api.cpp
        Source/SwDecoder.cpp
)

target_include_directories(${LIB_NAME}
        PUBLIC
        Include
        "${LibNL_INCLUDE_DIR}"
        "../../../drivers"

        PRIVATE
        Source
)

target_link_libraries(${LIB_NAME}
        PUBLIC

        PRIVATE
        fmt
        ${LibNL_LIBRARIES}
        # $<$<OR:$<STREQUAL:"${CMAKE_SYSTEM_NAME}","Windows">,$<STREQUAL:"${CMAKE_SYSTEM_NAME}","Linux">>:GTest::gtest_main>
)

target_compile_options(${LIB_NAME}
        PRIVATE
        # $<$<STREQUAL:"${CMAKE_SYSTEM_NAME}","Linux">:--coverage>
)