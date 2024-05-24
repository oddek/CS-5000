
#ifndef SUT_KERNELPATTERNVERIFIER_H
#define SUT_KERNELPATTERNVERIFIER_H

#include "PacketQueue.h"
#include "PatternVerifier.h"
#include <PatternVerifier/Netlink.h>
#include <fmt/core.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>

class KernelPatternVerifier : public PatternVerifier
{
  public:
    KernelPatternVerifier();
    void update() override;
    void reset() override;
    void verify(const uint8_t* buffer, size_t nBytes) override;
    void verify(const Packet<>& packet) override;

    Stats getStats() override;


  private:
    struct nl_sock* socket;
    int genl_family;
    static constexpr size_t socketBufferSize = 256U * 8192U;
};




#endif // SUT_KERNELPATTERNVERIFIER_H
