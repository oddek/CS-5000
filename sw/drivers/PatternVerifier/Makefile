

ccflags-y := -std=gnu99 -Wno-declaration-after-statement -Wno-implicit-function-declaration -Wno-strict-prototypes

MOD_NAME := PatternVerifier

obj-m := ${MOD_NAME}.o 
${MOD_NAME}-objs := Driver.o Verifier.o Netlink.o

all: modules install
modules: check-build-env
	@echo $(PWD)
	@$(MAKE) -C $(KDIR) M=$(PWD) modules
install: check-install-env
	sudo cp $(MOD_NAME).ko $(LKM_INSTALL_PATH)/
	sudo cp load_verifier unload_verifier $(ROOTFS_HOME)/
clean:
	@$(MAKE) -C $(KDIR) M=$(PWD) clean
check-build-env:
ifndef ARCH
	$(error ARCH is undefined)
endif
ifndef CROSS_COMPILE
	$(error CROSS_COMPILE is undefined)
endif
ifndef KDIR
	$(error KDIR is undefined)
endif
check-install-env:
ifndef LKM_INSTALL_PATH
	$(error LKM_INSTALL_PATH is undefined)
endif

