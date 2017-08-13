#############################################################################
# Automatically generated from ./examples/ktsert/Makefile.in
# Build options from
#############################################################################

CC  = gcc

####### Compiling application flags
SYSCONF_CFLAGS	= -g -O2 -march=x86-64 -fPIC -Wall -fstack-protector-all

####### Linking applications flags
SYSCONF_LINK	= $(CC)
SYSCONF_LFLAGS	= `pkg-config gtk+-2.0 --libs` -fPIC 
#-fPIE
SYSCONF_LIBS	= $(SYSCONF_LFLAGS) -lm

####### Linking shared libraries
SYSCONF_LINK_SHLIB	= $(CC)
#SYSCONF_LINK_SHLIB	= $(CC) --strip-all
SYSCONF_LINK_TARGET_SHARED = lib$(TARGET).so.$(VER_MAJ).$(VER_MIN).$(VER_PATCH)
SYSCONF_LINK_LIB_SHARED	= $(SYSCONF_LINK_SHLIB) -shared -Wl,-soname,lib$(TARGET).so.$(VER_MAJ) \
     $(LFLAGS) -o $(SYSCONF_LINK_TARGET_SHARED) \
     $(OBJECTS) $(LIBS)

# Linking static libraries
SYSCONF_AR = ar cqs
SYSCONF_LINK_TARGET_STATIC = lib$(TARGET).a
#SYSCONF_LINK_LIB_STATIC	= rm -f $(DESTDIR)$(SYSCONF_LINK_TARGET_STATIC) ;
SYSCONF_LINK_LIB_STATIC	= rm -f $(SYSCONF_LINK_TARGET_STATIC) ; \
	$(SYSCONF_AR) $(SYSCONF_LINK_TARGET_STATIC) $(OBJECTS)
#	$(SYSCONF_AR) $(DESTDIR)$(SYSCONF_LINK_TARGET_STATIC) $(OBJECTS)

####### Default link type (static linking is still used where required)
SYSCONF_LINK_LIB	= $(SYSCONF_LINK_LIB_SHARED)
SYSCONF_LINK_TARGET	= $(SYSCONF_LINK_TARGET_SHARED)

SYSCONF_LINK_SLIB		= $(SYSCONF_LINK_LIB_STATIC)
SYSCONF_LINK_STARGET	= $(SYSCONF_LINK_TARGET_STATIC)

#############################################################################
CFLAGS	= $(SYSCONF_CFLAGS) `pkg-config --cflags gtk+-2.0`
LFLAGS	= $(SYSCONF_LFLAGS)
LIBS	= $(SYSCONF_LIBS)

####### Target
DESTDIR = /usr/lib64/
VER_MAJ = 1
VER_MIN = 0
VER_PATCH = 0
TARGET	= gtkpiechart
#TARGET1 = lib$(TARGET).so.$(VER_MAJ)

####### Implicit rules

#.SUFFIXES: .cpp .cxx .tcc .cc .C .c
.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

OBJECTS= gtkpiechart.o

####### Build rules
all: $(DESTDIR)$(SYSCONF_LINK_TARGET) rstatic

$(DESTDIR)$(SYSCONF_LINK_TARGET): $(OBJECTS)
	$(SYSCONF_LINK_LIB)

rstatic: $(DESTDIR)$(SYSCONF_LINK_STARGET)

$(DESTDIR)$(SYSCONF_LINK_STARGET): $(OBJECTS)
	$(SYSCONF_LINK_SLIB)

install:
	-mkdir -p /usr/include/gtkpiechart
	-cp -a gtkpiechart.h /usr/include/gtkpiechart/
	-ln -fs $(SYSCONF_LINK_TARGET_SHARED) lib$(TARGET).so
	-ln -fs $(SYSCONF_LINK_TARGET_SHARED) lib$(TARGET).so.$(VER_MAJ)
	-ln -fs $(SYSCONF_LINK_TARGET_SHARED) lib$(TARGET).so.$(VER_MAJ).$(VER_MIN)
	-cp -a lib$(TARGET).so* $(DESTDIR)
	-cp -a lib$(TARGET).a $(DESTDIR)

clean:
	-rm -f gtkpiechart.o
	-rm -f $(SYSCONF_LINK_TARGET_SHARED) \
		lib$(TARGET).so.$(VER_MAJ).$(VER_MIN) \
		lib$(TARGET).so.$(VER_MAJ) \
		lib$(TARGET).so

