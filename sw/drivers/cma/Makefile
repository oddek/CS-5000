
ccflags-y := -std=gnu99 -Wno-declaration-after-statement -Wno-implicit-function-declaration -Wno-strict-prototypes

MOD_NAME := cma

obj-m := $(MOD_NAME).o

all: modules install
modules: check-build-env
	@echo $(PWD)
	@$(MAKE) -C $(KDIR) M=$(PWD) modules
install: check-install-env
	sudo mkdir -p $(LKM_INSTALL_PATH)/dma
	sudo cp $(MOD_NAME).ko $(LKM_INSTALL_PATH)/dma/
	sudo cp load_cma unload_cma $(ROOTFS_HOME)/
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

