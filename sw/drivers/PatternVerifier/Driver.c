
#include "../common/common.h"
#include <linux/module.h>
#include "Verifier.h"
#include "Netlink.h"

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Kent Odde" );
MODULE_DESCRIPTION( "Pattern Verifier Through Generic Netlink Driver, with char interface" );
MODULE_VERSION( "1.0" );
#define pr_fmt(fmt) "%s:%s: " fmt, KBUILD_MODNAME, __func__

Verifier verifier = {0};

static int test_init( void )
{
    verifierInit(&verifier);
    return netlinkInit();
}

static void test_exit( void )
{
    netlinkDeInit();
}

module_init( test_init );
module_exit( test_exit );