
#include <vmlinux.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_tracing.h>

#define ARM_cpsr    uregs[16]
#define user_mode(regs) (((regs)->ARM_cpsr & 0xf) == 0)
#define MODE_MASK    0x0000001f
#define processor_mode(regs) \
    ((regs)->ARM_cpsr & MODE_MASK)

static const char *processor_modes[] = {
        "USER_26", "FIQ_26", "IRQ_26", "SVC_26", "UK4_26", "UK5_26", "UK6_26", "UK7_26",
        "UK8_26", "UK9_26", "UK10_26", "UK11_26", "UK12_26", "UK13_26", "UK14_26", "UK15_26",
        "USER_32", "FIQ_32", "IRQ_32", "SVC_32", "UK4_32", "UK5_32", "MON_32", "ABT_32",
        "UK8_32", "UK9_32", "HYP_32", "UND_32", "UK12_32", "UK13_32", "UK14_32", "SYS_32"
};

SEC("tp_btf/sched_switch")
int BPF_PROG(sched_switch, bool preempt, struct task_struct *prev, struct task_struct *next) 
{
    if (bpf_strncmp(next->comm, 16, "PreemptionTest") == 0)
    {
        if(preempt)
        {
            struct pt_regs *prev_regs = (struct pt_regs *) bpf_task_pt_regs(prev);
            struct pt_regs *next_regs = (struct pt_regs *) bpf_task_pt_regs(next);

            if(!user_mode(prev_regs))
            {
                bpf_printk("%s preempt %d-%s KERN, Mod: %d-%s\n", next->comm, prev->pid, prev->comm, processor_mode(prev_regs), processor_modes[processor_mode(prev_regs)]);
            }
        }
    }

    return 0;
}


char LICENSE[] SEC("license") = "GPL";





