# Kernel makefile
# This makefile was automatically generated. Do not hand-modify
# Generated at Tue Aug  3 13:16:24 1993

# Template for kernel build makefile.
# Below, %C -> compile rules, %l -> link files, %b = "before" files,

# %B -> "before" rules, %a = "after" files, %A -> "after" rules,
# %r -> clean files and %% -> %.


PREFLAGS = 
POSTFLAGS = $(TARGET)

DRVOBJS = obj/conf.o /TB/K81/conf/streams/Stub.o obj/cohmain.o obj/console.o /TB/K81/conf/em87/Stub.o obj/fd.o obj/at.o obj/aha.o obj/hai.o obj/ss.o obj/patch.o /TB/K81/conf/patch/Driver.o 

all: pre $(TARGET) post

pre: 
	for i in $<; do if [ -x $$i ]; then $$i $(PREFLAGS); fi; done

$(TARGET): $(K386LIB)/k0.o $(DRVOBJS) $(K386LIB)/k386.a
	ld -K -o $(TARGET) -e stext $(K386LIB)/k0.o $(DRVOBJS) \
			$(K386LIB)/k386.a

post: 
	for i in $<; do if [ -x $$i ]; then $$i $(POSTFLAGS); fi; done

obj/conf.o : conf.c conf.h
	$(CC) $(CFLAGS) -o obj/conf.o -c conf.c

conf.h: mtune stune
	echo confpath=$(CONFPATH)
	$(CONFPATH)/bin/devadm -t
	if [ "obj/*" ] ; then rm -f obj/* ; else true ; fi

conf.c: mdevice sdevice
	@echo "You need to run \"make\" in response to a configuration change"
	exit 1

obj/cohmain.o : /TB/K81/conf/cohmain/Space.c
	$(CC) $(CFLAGS) -I/TB/K81/conf/cohmain -I. -o obj/cohmain.o  -c /TB/K81/conf/cohmain/Space.c

obj/console.o : /TB/K81/conf/console/Space.c
	$(CC) $(CFLAGS) -I/TB/K81/conf/console -I. -o obj/console.o  -c /TB/K81/conf/console/Space.c

obj/fd.o : /TB/K81/conf/fd/Space.c
	$(CC) $(CFLAGS) -I/TB/K81/conf/fd -I. -o obj/fd.o  -c /TB/K81/conf/fd/Space.c

obj/at.o : /TB/K81/conf/at/Space.c
	$(CC) $(CFLAGS) -I/TB/K81/conf/at -I. -o obj/at.o  -c /TB/K81/conf/at/Space.c

obj/aha.o : /TB/K81/conf/aha/Space.c
	$(CC) $(CFLAGS) -I/TB/K81/conf/aha -I. -o obj/aha.o  -c /TB/K81/conf/aha/Space.c

obj/hai.o : /TB/K81/conf/hai/Space.c
	$(CC) $(CFLAGS) -I/TB/K81/conf/hai -I. -o obj/hai.o  -c /TB/K81/conf/hai/Space.c

obj/ss.o : /TB/K81/conf/ss/Space.c
	$(CC) $(CFLAGS) -I/TB/K81/conf/ss -I. -o obj/ss.o  -c /TB/K81/conf/ss/Space.c

obj/patch.o : /TB/K81/conf/patch/Space.c
	$(CC) $(CFLAGS) -I/TB/K81/conf/patch -I. -o obj/patch.o  -c /TB/K81/conf/patch/Space.c



clean:
	rm conf.c conf.h
	if [ "obj/cohmain.o obj/console.o obj/fd.o obj/at.o obj/aha.o obj/hai.o obj/ss.o obj/patch.o " ] ; then rm -f obj/cohmain.o obj/console.o obj/fd.o obj/at.o obj/aha.o obj/hai.o obj/ss.o obj/patch.o  ; else true ; fi

