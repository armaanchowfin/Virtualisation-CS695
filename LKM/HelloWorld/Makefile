KDIR = /lib/modules/`uname -r`/build

# The kbuild system knows that an external module is being built due to the "M=<dir>".
kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean
