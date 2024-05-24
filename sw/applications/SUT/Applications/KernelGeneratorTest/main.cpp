#include "KernelPatternVerifier.h"
#include "KernelPatternGenerator.h"
#include "UserspacePatternVerifier.h"
#include <fmt/core.h>
#include <thread>

KernelPatternVerifier kernelVerifier;
UserspacePatternVerifier verifier{1};
KernelPatternGenerator generator{true};


int main(int argc, char** argv)
{
    while(true)
    {
        const auto packet = generator.get();

        kernelVerifier.put(packet);
        verifier.put(packet);

        usleep(1000);

        const auto kernelVerifierStats = kernelVerifier.getStats();
        const auto verifierStats = verifier.getStats();
        fmt::println("KernelVerifier:\n\tErrors: {}\n\tBytesReceived: {}\n\tTotalBytesRecieved: {}", kernelVerifierStats.errors, kernelVerifierStats.bytesVerified, kernelVerifierStats.totalBytesVerified);
        fmt::println("Verifier:\n\tErrors: {}\n\tBytesReceived: {}\n\tTotalBytesRecieved: {}", verifierStats.errors, verifierStats.bytesVerified, verifierStats.totalBytesVerified);

        kernelVerifier.reset();
        verifier.reset();
    }
}
