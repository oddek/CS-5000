#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <string.h>
#include <errno.h>
#include <sys/resource.h>
#include <bpf/libbpf.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "migration.skel.h"
#include "preempted.skel.h"
#include "preempting.skel.h"

static volatile sig_atomic_t stop;
static struct migration_bpf *skel_migration;
static struct preempting_bpf *skel_preempting;
static struct preempted_bpf *skel_preempted;


static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
    return vfprintf(stderr, format, args);
}


static void sig_int(int signo)
{
    stop = 1;
}

int main(int argc, char **argv)
{
    if( argc < 2){
        printf("Required on arg\n", argv[0]);
        return 0;
    }

    libbpf_set_print(libbpf_print_fn);

    int err;

    if( strcmp(argv[1], "migration") == 0){
        skel_migration = migration_bpf__open_and_load();
        if (!skel_migration) {
            fprintf(stderr, "Failed to open BPF skeleton\n");
            return 1;
        }

        err = migration_bpf__attach(skel_migration);
    }
    else if( strcmp(argv[1], "preempting") == 0){
        skel_preempting = preempting_bpf__open_and_load();
        if (!skel_preempting) {
            fprintf(stderr, "Failed to open BPF skeleton\n");
            return 1;
        }

        err = preempting_bpf__attach(skel_preempting);
    }
    else if( strcmp(argv[1], "preempted") == 0){
        skel_preempted = preempted_bpf__open_and_load();
        if (!skel_preempted) {
            fprintf(stderr, "Failed to open BPF skeleton\n");
            return 1;
        }

        err = preempted_bpf__attach(skel_preempted);
    }
    else{
        printf("Invalid argument\n");
        return 0;
    }

    if (err) {
        fprintf(stderr, "Failed to attach BPF skeleton\n");
        goto cleanup;
    }

    if (signal(SIGINT, sig_int) == SIG_ERR) {
        fprintf(stderr, "can't set signal handler: %s\n", strerror(errno));
        goto cleanup;
    }

    printf("Successfully started! Please run `sudo cat /sys/kernel/debug/tracing/trace_pipe` "
           "to see output of the BPF programs.\n");

    while (!stop) {
        fprintf(stderr, ".");
        sleep(1);
    }

cleanup:
    if( strcmp(argv[1], "migration") == 0){
        migration_bpf__destroy(skel_migration);
    }
    else if( strcmp(argv[1], "preempting") == 0){
        preempting_bpf__destroy(skel_preempting);
    }
    else if( strcmp(argv[1], "preempted") == 0){
        preempted_bpf__destroy(skel_preempted);
    }
    return -err;
}