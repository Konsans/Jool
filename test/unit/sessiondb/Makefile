# It appears the -C's during the makes below prevent this include from happening
# when it's supposed to.
# For that reason, I can't just do "include ../common.mk". I need the absolute
# path of the file.
# Unfortunately, while the (as always utterly useless) working directory is (as
# always) brain-dead easy to access, the easiest way I found to get to the
# "current" directory is the mouthful below.
# And yet, it still has at least one major problem: if the path contains
# whitespace, `lastword $(MAKEFILE_LIST)` goes apeshit.
# This is the one and only reason why the unit tests need to be run in a
# space-free directory.
include $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))/../common.mk


SESSIONDB = sessiondb

obj-m += $(SESSIONDB).o

$(SESSIONDB)-objs += $(MIN_REQS)
$(SESSIONDB)-objs += ../../../src/mod/common/translation_state.o
$(SESSIONDB)-objs += ../../../src/mod/common/wrapper-config.o
$(SESSIONDB)-objs += ../../../src/mod/common/wrapper-global.o
$(SESSIONDB)-objs += ../../../src/mod/common/db/global.o
$(SESSIONDB)-objs += ../../../src/mod/common/db/rbtree.o
$(SESSIONDB)-objs += ../../../src/mod/common/db/bib/db.o
$(SESSIONDB)-objs += ../../../src/mod/common/db/bib/entry.o
$(SESSIONDB)-objs += ../../../src/mod/common/nl/attribute.o
$(SESSIONDB)-objs += ../impersonator/bib.o
$(SESSIONDB)-objs += ../impersonator/icmp_wrapper.o
$(SESSIONDB)-objs += ../impersonator/route.o
$(SESSIONDB)-objs += ../impersonator/stats.o
$(SESSIONDB)-objs += ../impersonator/xlator.o
$(SESSIONDB)-objs += sessiondb_test.o


all:
	make -C ${KERNEL_DIR} M=$$PWD;
modules:
	make -C ${KERNEL_DIR} M=$$PWD $@;
clean:
	make -C ${KERNEL_DIR} M=$$PWD $@;
test:
	sudo dmesg -C
	-sudo insmod $(SESSIONDB).ko && sudo rmmod $(SESSIONDB)
	sudo dmesg -tc | less
