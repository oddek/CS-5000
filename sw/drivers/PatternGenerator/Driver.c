
#include "../common/common.h"
#include "Generator.h"
#include "Netlink.h"
#include "RingBuffer.h"

#include <linux/module.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/completion.h>

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Kent Odde" );
MODULE_DESCRIPTION( "Pattern Generator Through Generic Netlink Driver, with char interface" );
MODULE_VERSION( "1.0" );

Generator generator = {0};
RingBuffer ringBuffer = {0};
bool asked = false;
uint32_t askedLength = 0;


static struct task_struct* generator_thread;
DECLARE_WAIT_QUEUE_HEAD(wait_queue_etx);

static DECLARE_COMPLETION(thread_done);

typedef enum
{
    WaitFlag_Wait = 0U,
    WaitFlag_Buffer,
    WaitFlag_Exit
} WaitFlag;

static WaitFlag waitFlag = WaitFlag_Wait;

static dev_t first;
static struct cdev c_dev;
static struct class *cl;

static ssize_t writeFunc(struct file *f, const char __user *buf, size_t len, loff_t *off);

static struct file_operations fops =
{
  .owner = THIS_MODULE,
  .write = writeFunc
};

static ssize_t writeFunc(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    unsigned long long res;
    int ret = kstrtoull_from_user(buf, len, 10, &res);

    if(!ringBufferIsFull(&ringBuffer))
    {
        ringBufferPut(&ringBuffer, res);
        wake_up_interruptible(&wait_queue_etx);
        return len;
    }

    return -1;
}

static void generateAndSendPackage(size_t nBytes)
{
    // generatorReset(&generator);

    size_t remainingBytes = nBytes;

    while(remainingBytes)
    {
        const size_t bytesToGenerate = (remainingBytes > PATTERN_GENERATOR_MAX_PACKET_SIZE) ? PATTERN_GENERATOR_MAX_PACKET_SIZE : remainingBytes;

        uint8_t buffer[PATTERN_GENERATOR_MAX_PACKET_SIZE];
        generatorGenerate(&generator, buffer, bytesToGenerate);

        PacketMetaData metaData = Packet_MetaData_None;

        if( remainingBytes == nBytes && remainingBytes == bytesToGenerate)
        {
            metaData = Packet_MetaData_Only;
        }

        else if( remainingBytes == nBytes )
        {
            metaData = Packet_MetaData_First;
        }
        else if(remainingBytes == bytesToGenerate)
        {
            metaData = Packet_MetaData_Last;
        }

        remainingBytes -= bytesToGenerate;
        netlinkTransmitBuffer(buffer, bytesToGenerate, metaData, nBytes);
    }
}

static inline bool hasBeenAsked()
{
    bool beenAsked = READ_ONCE(asked);

    return beenAsked;
}

// #define POLL

static int generatorWorkerAsk(void* arg)
{
    #ifdef POLL
    pr_info("GeneratorWorker in POLL mode\n");
    #else
    pr_info("GeneratorWorker in EVENT mode\n");
    #endif
    while(true)
    {
        #ifdef POLL
        while(true)
        {
        #endif

        #ifndef POLL
            if( !hasBeenAsked())
            {
                wait_event_interruptible(wait_queue_etx, hasBeenAsked() || waitFlag != WaitFlag_Wait );
            }

            if( waitFlag == WaitFlag_Exit)
            {
                kthread_complete_and_exit(&thread_done, 0);
            }
            // else
            // {
            //     pr_info("Event worker was asked\n");
            // }

        #else

            if( hasBeenAsked() && waitFlag != WaitFlag_Exit)
            {
                break;
            }

            if( waitFlag == WaitFlag_Exit)
            {
                kthread_complete_and_exit(&thread_done, 0);
            }

            usleep_range(10, 100);

        }

        #endif

        generateAndSendPackage(askedLength);

        WRITE_ONCE(asked, false);

        if( waitFlag == WaitFlag_Exit)
        {
            kthread_complete_and_exit(&thread_done, 0);
        }
    }

    pr_info("Worker killed\n");
    kthread_complete_and_exit(&thread_done, 0);
}

static int generatorWorker(void* arg)
{
    while(true)
    {
        if( ringBufferIsEmpty(&ringBuffer) || !hasBeenAsked())
        {
            wait_event_interruptible(wait_queue_etx, (!ringBufferIsEmpty(&ringBuffer) && hasBeenAsked()) || waitFlag != WaitFlag_Wait );
        }

        if( waitFlag == WaitFlag_Exit)
        {
            return 0;
        }

        uint32_t word = ringBufferGet(&ringBuffer);
        pr_debug("Read word %u, from ringBuffer\n", word);

        if( word )
        {
            generateAndSendPackage(word);
            WRITE_ONCE(asked, false);
        }
    }

    pr_info("Worker killed\n");

    return 0;
}


static int test_init( void )
{
    ringBufferInit(&ringBuffer);
    generatorInit(&generator);
    if(netlinkInit() < 0)
    {
        goto netlink_cleanup;
    }

    if (alloc_chrdev_region(&first, 0, 1, "GeneratorChrDevRegion") < 0)
    {
        goto netlink_cleanup;
    }

	#if LINUX_VERSION_CODE <= KERNEL_VERSION(6,6,0) 
        if ((cl = class_create(THIS_MODULE, "GeneratorChrClass")) == NULL)
	#else
        if ((cl = class_create("GeneratorChrClass")) == NULL)
	#endif
    {
        goto alloc_chrdev_cleanup;
    }

    if (device_create(cl, NULL, first, NULL, "generator") == NULL)
    {
        goto class_cleanup;
    }

    cdev_init(&c_dev, &fops);
    if (cdev_add(&c_dev, first, 1) == -1)
    {
        goto device_cleanup;
    }

    // generator_thread = kthread_create(generatorWorkerAsk, NULL, "GeneratorThread");
    generator_thread = kthread_create(generatorWorker, NULL, "GeneratorThread");
    if( IS_ERR(generator_thread))
    {
        complete(&thread_done);
    }
    // sched_set_fifo(generator_thread);
    // sched_set_normal(generator_thread, -15);

    if( !generator_thread)
    {
        goto device_cleanup;
    }

    wake_up_process(generator_thread);
    pr_info("Init Success\n");
    return 0;

device_cleanup:
    pr_err("Device cleanup\n");
    device_destroy(cl, first);
class_cleanup:
    pr_err("Class cleanup\n");
    class_destroy(cl);
alloc_chrdev_cleanup:
    pr_err("Chrdev cleanup\n");
    unregister_chrdev_region(first, 1);
netlink_cleanup:
    pr_err("Netlink cleanup\n");
    netlinkDeInit();

    return -1;
}

static void test_exit( void )
{
    pr_info("Killing worker\n");
    waitFlag = WaitFlag_Exit;
    wake_up_interruptible(&wait_queue_etx);

    pr_info("Netlink deinit\n");
    netlinkDeInit();
    pr_info("Device delete\n");
    cdev_del(&c_dev);
    pr_info("Device destroy\n");
    device_destroy(cl, first);
    pr_info("Class destroy\n");
    class_destroy(cl);
    pr_info("Chrdev unregister\n");
    unregister_chrdev_region(first, 1);
    pr_info("Exit sucess\n");
}

module_init( test_init );
module_exit( test_exit );