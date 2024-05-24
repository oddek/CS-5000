

#include "KernelPatternVerifier.h"

static int verifierCallback(nl_msg* msg, void* arg)
{
    auto* verifier = static_cast<KernelPatternVerifier*>(arg);
    auto* header = static_cast<struct genlmsghdr*>(nlmsg_data(nlmsg_hdr(msg)));

    if (header == nullptr)
    {
        throw std::runtime_error("Kernel Verifier: Received header is NULL");
    }

    std::array<nlattr*, VER_MAX + 1> attributes{};
    if( int ret = nla_parse(attributes.data(), VER_MAX, genlmsg_attrdata(header, 0), genlmsg_attrlen(header, 0), nullptr); ret != 0)
    {
        throw std::runtime_error(fmt::format("Kernel Verifier: nla_parse returned {}", ret));
    }

    if ((attributes.at(VER_ERRORS) == nullptr) || (attributes.at(VER_BYTES_VERIFIED) == nullptr) || (attributes.at(VER_TOTAL_BYTES_VERIFIED) == nullptr))
    {
        fmt::println("Missing Attributes");
    }

    verifier->stats.errors = nla_get_u32(attributes.at(VER_ERRORS));
    verifier->stats.bytesVerified = nla_get_u64(attributes.at(VER_BYTES_VERIFIED));
    verifier->stats.totalBytesVerified = nla_get_u64(attributes.at(VER_TOTAL_BYTES_VERIFIED));

    return 0;
}

KernelPatternVerifier::KernelPatternVerifier() : socket(nl_socket_alloc())
{
    genl_connect(socket);
    genl_family = genl_ctrl_resolve(socket, VERIFIER_NETLINK_FAMILY);

    if (genl_family < 0)
    {
        throw std::runtime_error("Kernel Verifier: Genl resolve failed");
    }

    nl_socket_disable_seq_check(socket);
    nl_socket_set_buffer_size(socket, socketBufferSize, socketBufferSize);
    nl_socket_disable_auto_ack(socket);

    const int ret = nl_socket_modify_cb(socket, NL_CB_VALID,
                                  NL_CB_CUSTOM,
                                  verifierCallback, this);

    if (ret < 0)
    {
        throw std::runtime_error("Kernel Verifier: Callback modify failed");
    }

    KernelPatternVerifier::reset();
}

void KernelPatternVerifier::reset()
{
    struct nl_msg* msg;
    msg = nlmsg_alloc();
    genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, genl_family, 0, 0, VERIFIER_RESET, NLM_F_REQUEST);
    nl_send_auto(socket, msg); /* Send message */
    nlmsg_free(msg);

    packetQueue.clear();
}

KernelPatternVerifier::Stats KernelPatternVerifier::getStats()
{
    struct nl_msg* msg = nlmsg_alloc();
    genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, genl_family, 0, 0, GET_STATS, NLM_F_REQUEST);
    nl_send_auto(socket, msg); /* Send message */
    nlmsg_free(msg);
    int ret = nl_recvmsgs_default(socket);

    if( ret != 0)
    {
        const auto error = fmt::format("Kernel Verifier: Get Stats return value is {}", ret);
        throw std::runtime_error(error);
    }

    return stats;
}
void KernelPatternVerifier::update()
{
    while( !packetQueue.isEmpty())
    {
        const auto packet = packetQueue.get();

        struct nl_msg* msg = nlmsg_alloc();
        genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ,  genl_family, 0, 0, VERIFY, NLM_F_REQUEST);
        nla_put_u32(msg, VER_LENGTH, packet.getSize());
        nla_put(msg, VER_PAYLOAD, static_cast<int>(packet.getSize()), packet.getPayload().data());
        nl_send_auto(socket, msg); /* Send message */
        nlmsg_free(msg);
    }
}

void KernelPatternVerifier::verify(const Packet<>& packet)
{
    verify(packet.getPayload().data(), packet.getSize());
}

void KernelPatternVerifier::verify(const uint8_t* buffer, size_t nBytes)
{
    struct nl_msg* msg = nlmsg_alloc();
    genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ,  genl_family, 0, 0, VERIFY, NLM_F_REQUEST);
    nla_put_u32(msg, VER_LENGTH, nBytes);
    nla_put(msg, VER_PAYLOAD, static_cast<int>(nBytes), buffer);
    nl_send_auto(socket, msg); /* Send message */
    nlmsg_free(msg);
}
