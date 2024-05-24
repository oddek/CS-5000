#include <vmlinux.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_tracing.h>

SEC("tp_btf/sched_switch")
int BPF_PROG(sched_switch, bool preempt, struct task_struct *prev, struct task_struct *next) 
{
    if (bpf_strncmp(prev->comm, 16, "PreemptionTest") == 0) 
    {
        if(preempt)
        {
            bpf_printk("Preempted by: %d - %s\n", next->pid, next->comm);
        }
    }

    return 0;
}


char LICENSE[] SEC("license") = "GPL";





