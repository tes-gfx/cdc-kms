obj-m := cdc.o
cdc-y := cdc_drv.o \
         cdc_kms.o \
         cdc_crtc.o \
         cdc_plane.o \
         cdc_lvdscon.o \
         cdc_hdmicon.o \
         cdc_hdmienc.o \
         cdc_encoder.o \
         cdc_hw.o \
         altera_pll.o \
         hw/cdc_global.o \
         hw/cdc_layer.o \
         hw/cdc_int.o
ccflags-y := -DDISABLE_ASSERTIONS -I$(src)/hw/

SRC := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC)

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install

clean:
	rm -f *.o *~ core .depend .*.cmd *.ko *.mod.c
	rm -f Module.markers Module.symvers modules.order
	rm -rf .tmp_versions Modules.symvers
	rm ./hw/*.o
