
#ifndef SUT_KERNELPATTERNGENERATOR_H
#define SUT_KERNELPATTERNGENERATOR_H

#include "PatternGenerator.h"
#include <PatternGenerator/Generator.h>
#include <PatternGenerator/Netlink.h>
#include <fmt/core.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>

class KernelPatternGenerator : public PatternGenerator
{
  public:
    KernelPatternGenerator(bool createThread);
    void generateAndEnqueueMessage(size_t messageSize) override;
    void reset() override;
    void update() override;
    int receiveMessage(struct nl_msg* msg);

  private:
    struct nl_sock* socket{nl_socket_alloc()};
    int genl_family;
    pthread_attr_t thread_attr{};
    pthread_t thread{0};
    static constexpr sched_param realTimePriority{79};
    static constexpr size_t socketBufferSize = 256U * 8192U;
};

#endif // SUT_KERNELPATTERNGENERATOR_H
