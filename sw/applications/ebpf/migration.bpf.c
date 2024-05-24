#include <vmlinux.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_tracing.h>

static bool doPrint = false;

SEC("tp_btf/sched_switch")
int BPF_PROG(sched_switch, bool preempt, struct task_struct *prev, struct task_struct *next) 
{
    if (bpf_strncmp(prev->comm, 16, "PreemptionTest") == 0) {
        if (preempt) {
            doPrint = true;
            bpf_printk("PREEMPTION, Next Comm is %s, pid is %d\n", next->comm, next->pid);
        }
    }

    return 0;
}

SEC("tp_btf/sched_migrate_task")
int BPF_PROG(sched_migrate_task, struct task_struct* p, int new_cpu)
{
    char curr_comm[16];
    bpf_get_current_comm(curr_comm, 16);

    if ((bpf_strncmp(curr_comm, 16, "migration/0") == 0) ||
        (bpf_strncmp(curr_comm, 16, "migration/1") == 0))
    {
        if( doPrint)
        {
            bpf_printk("%s, migrating %s to CPU %d\n", curr_comm, p->comm, new_cpu);
            doPrint = false;
        }
    }

    return 0;
}

char LICENSE[] SEC("license") = "GPL";





