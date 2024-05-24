
#include "KernelPatternGenerator.h"
#include <fmt/core.h>
#include <stdexcept>

extern "C"
{
    static void* generatorUpdateWorker(void* arg)
    {
        auto* generator = static_cast<KernelPatternGenerator*>(arg);
        while (true)
        {
            generator->update();
        }
    }

    static int generatorCallback(struct nl_msg* msg, void* arg)
    {
        auto* generator = static_cast<KernelPatternGenerator*>(arg);
        return generator->receiveMessage(msg);
    }
}

KernelPatternGenerator::KernelPatternGenerator(bool createThread) : socket(nl_socket_alloc())
{
    genl_connect(socket);
    genl_family = genl_ctrl_resolve(socket, GENERATOR_NETLINK_FAMILY);

    if (genl_family < 0)
    {
        throw std::runtime_error(fmt::format("Kernel Generator: Genl resolve failed: {}", genl_family));
    }

    nl_socket_disable_seq_check(socket);
    nl_socket_set_buffer_size(socket, socketBufferSize, socketBufferSize);
    nl_socket_disable_auto_ack(socket);

    const int ret = nl_socket_modify_cb(socket, NL_CB_VALID, NL_CB_CUSTOM, generatorCallback, this);

    if (ret < 0)
    {
        throw std::runtime_error(fmt::format("Kernel Generator: Callback modify failed: {}", ret));
    }

    if( createThread)
    {


        {
            if (pthread_attr_init(&thread_attr) != 0)
            {
                throw std::runtime_error("CyclicTask: Failed to init pthread attributes");
            }

            if (pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO) != 0)
            {
                throw std::runtime_error("CyclicTask: Failed to set pthread scheduling policy");
            }

            if (pthread_attr_setschedparam(&thread_attr, &realTimePriority) != 0)
            {
                throw std::runtime_error("CyclicTask: Failed to set pthread scheduling parameters");
            }

            if (pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED) != 0)
            {
                throw std::runtime_error("CyclicTask: Failed to set pthread inherit sched");
            }
        }




        if (pthread_create(&thread, &thread_attr, generatorUpdateWorker, this) != 0)
        {
            throw std::runtime_error("KernelPatternGenerator: Failed to create Thread");
        }

        if (pthread_setname_np(thread, "Generator") != 0)
        {
            throw std::runtime_error("CyclicTask: Failed to set thread name");
        }


    }

    KernelPatternGenerator::reset();
}

void KernelPatternGenerator::generateAndEnqueueMessage(size_t totalSize)
{
    nl_msg* msg = nlmsg_alloc();
    genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, genl_family, 0, 0, GENERATE, NLM_F_REQUEST);
    nla_put_u32(msg, SAI_LENGTH, totalSize);
    int ret = nl_send_auto(socket, msg); /* Send message */
    if( ret < 0)
    {
        throw std::runtime_error(fmt::format("reset calll to generator returned {}", ret));
    }
    nlmsg_free(msg);
}

void KernelPatternGenerator::reset()
{
    nl_msg* msg = nlmsg_alloc();
    genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, genl_family, 0, 0, RESET, 1);
    int ret = nl_send_auto(socket, msg);
    if( ret < 0)
    {
        throw std::runtime_error(fmt::format("reset calll to generator returned {}", ret));
    }

    nlmsg_free(msg);

    packetQueue.clear();
}

int KernelPatternGenerator::receiveMessage(struct nl_msg* msg)
{
    auto* header = static_cast<struct genlmsghdr*>(nlmsg_data(nlmsg_hdr(msg)));

    if (header == nullptr)
    {
        throw std::runtime_error("KernelPatternGenerator: Header in received message is NULL");
    }

    std::array<nlattr*, SAI_MAX + 1> attributes{};

    if (const auto ret =
            nla_parse(attributes.data(), SAI_MAX, genlmsg_attrdata(header, 0), genlmsg_attrlen(header, 0), nullptr);
        ret != 0)
    {
        throw std::runtime_error(fmt::format("KernelPatternGenerator: nla parse returns {}", ret));
    }

    if ((attributes[SAI_LENGTH] == nullptr) || (attributes[SAI_PAYLOAD] == nullptr) ||
        (attributes[SAI_META_DATA] == nullptr) || (attributes[SAI_TOTAL_LENGTH] == nullptr))
    {
        throw std::runtime_error(fmt::format("KernelPatternGenerator: nla attributes missing"));
    }

    const auto length = nla_get_u32(attributes.at(SAI_LENGTH));
    const auto totalLength = nla_get_u32(attributes.at(SAI_TOTAL_LENGTH));
    const auto metaData = static_cast<Packet<>::MetaData>(nla_get_u8(attributes.at(SAI_META_DATA)));
    auto* payload = reinterpret_cast<uint8_t*>(nla_get_string(attributes.at(SAI_PAYLOAD)));

    (void)length;
    (void)totalLength;
    (void)metaData;
    (void)payload;

    Packet<> packet(payload, length);
    packet.setMetaData(metaData);
    packet.setTotalMessageSize(totalLength);

    packetQueue.put(packet);

    if( packet.isFirstPacket())
    {
        bytesGenerated = 0U;
    }

    if( packet.isLastPacket())
    {
        messagesGenerated++;
    }

    bytesGenerated += length;
    totalBytesGenerated += length;
    packetsGenerated++;

    return NL_OK;
}

void KernelPatternGenerator::update()
{
    if (int ret = nl_recvmsgs_default(socket); ret != 0)
    {
        if( ret != -5)
        {
            throw std::runtime_error(fmt::format("KernelPatternGenerator: nl_recvmesgs returned {}", ret));
        }
        else
        {
            fmt::println("nl_recvmsgs_default returned NOMEM");
        }
    }
}

