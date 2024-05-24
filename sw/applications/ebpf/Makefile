
APPL = inject_ebpf
CFLAGS=-g -O2 -Wall -I /usr/include
LDFLAGS=../../rootfs/usr/lib/libbpf.so -lelf -lz -o inject_ebpf -L ../../buildroot/output/target/usr/lib -lzstd
CC = $(CROSS_COMPILE)gcc

CC_BPF=clang
CFLAGS_BPF=-g -O2 -Wall -target bpf -D__TARGET_ARCH_arm -I $(KDIR)


all: build install

build: check-build-env build-migration build-preempted build-preempting build-inject

build-migration:
	$(CC_BPF) $(CFLAGS_BPF) -c migration.bpf.c -o migration.bpf.o
	llvm-strip -g migration.bpf.o
	bpftool gen skeleton migration.bpf.o > migration.skel.h

build-preempted:
	$(CC_BPF) $(CFLAGS_BPF) -c preempted.bpf.c -o preempted.bpf.o
	llvm-strip -g preempted.bpf.o
	bpftool gen skeleton preempted.bpf.o > preempted.skel.h

build-preempting:
	$(CC_BPF) $(CFLAGS_BPF) -c preempting.bpf.c -o preempting.bpf.o
	llvm-strip -g preempting.bpf.o
	bpftool gen skeleton preempting.bpf.o > preempting.skel.h

build-inject: $(APPL)
	$(CC) $(CFLAGS) inject.o $(LDFLAGS)

$(APPL): inject.o 
	$(CC) $(LDFLAGS) $(C_FLAGS) -c $^ -o $@

install:
	sudo cp inject_ebpf $(ROOTFS_HOME)

clean:
	$(RM) *.o $(APPL)


check-build-env:
ifndef CROSS_COMPILE
	$(error CROSS_COMPILE is undefined)
endif
ifndef KDIR
	$(error KDIR is undefined)
endif
check-install-env:
ifndef ROOTFS_HOME
	$(error ROOTFS_HOME is undefined)
endif

