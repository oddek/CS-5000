

#include "../common/common.h"
#include "Netlink.h"
#include <net/genetlink.h>
#include "Verifier.h"

extern Verifier verifier; // From Driver.c

static int handleVerifyRequest( struct sk_buff* skb, struct genl_info* info );
static int handleResetRequest( struct sk_buff* skb, struct genl_info* info );
static int handleGetStatsRequest( struct sk_buff* skb, struct genl_info* info );

struct nla_policy const verifier_policy[VER_MAX + 1U] =
{
    [VER_LENGTH] = { .type = NLA_U32 },
    [VER_PAYLOAD] = { .type = NLA_BINARY },
    [VER_ERRORS] = { .type = NLA_U32 },
    [VER_BYTES_VERIFIED] = { .type = NLA_U64 },
    [VER_TOTAL_BYTES_VERIFIED] = { .type = NLA_U64 }
};

static const struct genl_ops ops[] =
{
    {
        .cmd = VERIFY,
        .doit = handleVerifyRequest,
    },
    {
        .cmd = VERIFIER_RESET,
        .doit = handleResetRequest,
    },
    {
        .cmd = GET_STATS,
        .doit = handleGetStatsRequest
    }
};

static struct genl_family sample_family =
{
    .name = VERIFIER_NETLINK_FAMILY,
    .version = VERIFIER_NETLINK_VERSION,
    .maxattr = VER_MAX,
    .policy = verifier_policy,
    .module = THIS_MODULE,
    .ops = ops,
    .n_ops = ARRAY_SIZE( ops ),
};


static int handleGetStatsRequest( struct sk_buff* skb, struct genl_info* info )
{
    struct sk_buff* msg = genlmsg_new( NLMSG_DEFAULT_SIZE, GFP_KERNEL );

    if( msg == NULL )
    {
        pr_err( "msg is NULL\n" );
        return -EINVAL;
    }

    void* hdr = genlmsg_put( msg, info->snd_portid, info->snd_seq, &sample_family, 0, GET_STATS  );

    if( hdr == NULL )
    {
        pr_err( "hdr is NULL\n" );
        return -EINVAL;
    }

    int ret = nla_put_u32( msg, VER_ERRORS, verifier.errors );

    if( ret )
    {
        pr_err( "Could not put errors\n" );
        return -EINVAL;
    }

    ret = nla_put_u64_64bit( msg, VER_BYTES_VERIFIED, verifier.bytesVerified, 0 );

    if( ret )
    {
        pr_err( "Could not put bytes verified\n" );
        return -EINVAL;
    }

    ret = nla_put_u64_64bit( msg, VER_TOTAL_BYTES_VERIFIED, verifier.totalBytesVerified, 0 );

    if( ret )
    {
        pr_err( "Could not put bytes verified\n" );
        return -EINVAL;
    }

    genlmsg_end( msg, hdr );

    ret = genlmsg_reply( msg, info );

    if( ret )
    {
        pr_err( "transmit message error: %d\n", ret );
        return -EINVAL;
    }

    return 0;
}

static int handleVerifyRequest( struct sk_buff* skb, struct genl_info* info )
{
    if ( !info->attrs[VER_LENGTH] )
    {
        pr_err( "Invalid request: Missing length attribute.\n" );
        return -EINVAL;
    }

    if ( !info->attrs[VER_PAYLOAD] )
    {
        pr_err( "Invalid request: Missing payload attribute.\n" );
        return -EINVAL;
    }

    const size_t nBytes = nla_get_u32( info->attrs[VER_LENGTH] );
    const uint8_t* buffer = ( uint8_t* )nla_data( info->attrs[VER_PAYLOAD] );

    verifierVerify(&verifier, buffer, nBytes);

    return 0;
}

static int handleResetRequest( struct sk_buff* skb, struct genl_info* info )
{
    pr_info( "Reseting\n" );
    verifierReset(&verifier);
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