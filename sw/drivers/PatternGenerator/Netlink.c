
#include "../common/common.h"
#include "Netlink.h"
#include <net/genetlink.h>
#include "Generator.h"
#include "RingBuffer.h"



extern Generator generator;
extern RingBuffer ringBuffer;
extern bool asked;
extern uint32_t askedLength;
extern wait_queue_head_t wait_queue_etx;

static uint32_t userPid = 0U;



// int handleSetupRequest( struct sk_buff* skb, struct genl_info* info );
int handleGenerateRequest( struct sk_buff* skb, struct genl_info* info );
int handleResetRequest( struct sk_buff* skb, struct genl_info* info );


struct nla_policy const generate_request_policy[SAI_MAX + 1U] =
{
    [SAI_LENGTH] = { .type = NLA_U32 },
    [SAI_PAYLOAD] = { .type = NLA_BINARY },
    [SAI_META_DATA] = { .type = NLA_U8 },
    [SAI_TOTAL_LENGTH] = { .type = NLA_U32 }
};

static const struct genl_ops ops[] =
{
    {
        .cmd = GENERATE,
        .doit = handleGenerateRequest,
    },
    {
        .cmd = RESET,
        .doit = handleResetRequest,
    },
    // {
    //     .cmd = SETUP,
    //     .doit = handleSetupRequest,
    // },
};

static struct genl_family sample_family =
{
    .name = GENERATOR_NETLINK_FAMILY,
    .version = GENERATOR_NETLINK_VERSION,
    .maxattr = SAI_MAX,
    .policy = generate_request_policy,
    .module = THIS_MODULE,
    .ops = ops,
    .n_ops = ARRAY_SIZE( ops ),
};

bool netlinkTransmitBuffer(const uint8_t* buffer, size_t nBytes, PacketMetaData metaData, size_t totalMessageSize)
{

    // printk("TX: %u, Meta: %u\n\t", nBytes, (uint8_t)metaData);
    // for(size_t i = 0; i < nBytes; i++)
    // {
    //     printk("%u, ", buffer[i]);
    // }
    // printk("\n\n");


    struct sk_buff* msg = genlmsg_new( NLMSG_DEFAULT_SIZE, GFP_KERNEL );

    if( msg == NULL )
    {
        pr_err( "msg is NULL\n" );
        return false;
    }

    void* hdr = genlmsg_put( msg, userPid, 0, &sample_family, 0, GENERATE  );

    if( hdr == NULL )
    {
        pr_err( "hdr is NULL\n" );
        return false;
    }

    int ret = nla_put_u32( msg, SAI_LENGTH, nBytes );

    if( ret )
    {
        pr_err( "Could not put nBytes\n" );
        return false;
    }

    ret = nla_put_u8( msg, SAI_META_DATA, (uint8_t)metaData );

    if( ret )
    {
        pr_err( "Could not put meta data\n" );
        return false;
    }

    ret = nla_put( msg, SAI_PAYLOAD, nBytes, buffer );

    if( ret )
    {
        pr_err( "Could not put payload\n" );
        return false;
    }

    ret = nla_put_u32( msg, SAI_TOTAL_LENGTH, totalMessageSize );

    if( ret )
    {
        pr_err( "Could not put total message size\n" );
        return false;
    }

    genlmsg_end( msg, hdr );

    ret = genlmsg_unicast( &init_net, msg, userPid );

    if( ret )
    {
        pr_err( "transmit message error: %d\n", ret );
        return false;
    }


    return true;
}

int handleGenerateRequest( struct sk_buff* skb, struct genl_info* info )
{
    // printk("Generate request received\n");
    userPid = info->snd_portid;

    if ( !info->attrs[SAI_LENGTH] )
    {
        pr_err( "Invalid request: Missing length attribute.\n" );
        return -EINVAL;
    }

    const size_t nBytes = nla_get_u32( info->attrs[SAI_LENGTH] );

    // uint8_t buffer[PATTERN_GENERATOR_MAX_PACKET_SIZE];

    // generatorGenerate2(&generator, buffer, nBytes);
    // const PacketMetaData metaData = Packet_MetaData_Only;

    // netlinkTransmitBuffer(buffer, nBytes, metaData, nBytes);

    const bool alreadyAsked = READ_ONCE(asked);

    if( !alreadyAsked)
    {
        WRITE_ONCE(askedLength, nBytes); 
        WRITE_ONCE(asked, true);

        wake_up_interruptible(&wait_queue_etx);
    }

    return 0;
}

int handleResetRequest( struct sk_buff* skb, struct genl_info* info )
{
    userPid = info->snd_portid;
    pr_info( "Reseting\n" );
    WRITE_ONCE(asked, false);
    generatorReset(&generator);
    ringBufferClear(&ringBuffer);
    return 0;
}

int netlinkInit()
{
    int ret = genl_register_family( &sample_family );
    if( ret != 0)
    {
        pr_err("genl register returned %d \n", ret );
    }

    return ret;
}

void netlinkDeInit()
{
    genl_unregister_family( &sample_family );
}