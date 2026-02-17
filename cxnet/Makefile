## makefile: cxnet_test ############################################

APP=		cxnet_test
CPP=        g++
SHELL=		/bin/sh
MOVE=		mv
COPY=		ln -s
RM=			rm -rf
TOUCH=		touch
LIBRARIAN=	ar rvu
INC=        -I../..

## Platform ###################################################


# get the OS
UNAME_S := $(shell uname -s | tr '[A-Z]' '[a-z]' )

# if this is linux
ifeq ($(UNAME_S),linux)
   	CPPFLAGS = -D _LINUX_  -g -Wno-deprecated
endif

#if this is OSX
ifeq ($(UNAME_S), darwin)
	ARCH := $(shell uname -m | tr '[A-Z]' '[a-z]' )
	CPPFLAGS = -D _OSX_ -g -Wno-deprecated
endif

ifeq ($(UNAME_S), linux)
	ARCH := $(shell uname -m | tr '[A-Z]' '[a-z]' )
    CCFLAGS = -D _LINUX_
endif

ifeq ($(UNAME_S), darwin)
    CCFLAGS = -D _OSX_
endif

#if this is SUNOS or SOLARIS
ifeq ($(UNAME_S),sunos)
    UNAME_R := $(shell uname -r)

	ifeq ($(UNAME_R), 4.1.3)
        CPPFLAGS = -D _SUNOS_ -g
        PLATFORM_LIBS = -lc
	endif

	ifeq ($(UNAME_R), 4.1.4)
        CPPFLAGS = -D _SUNOS_ -g
        PLATFORM_LIBS = -lc
	endif

	ifeq ($(UNAME_R), 5.6)
        CPPFLAGS = -D _SOLARIS6_ -g
        PLATFORM_LIBS = -lsocket -lnsl
	endif

	ifeq ($(UNAME_R), 5.7)
        CPPFLAGS = -D _SOLARIS6_ -g
        PLATFORM_LIBS = -lsocket -lnsl
	endif

    ifeq ($(UNAME_R), 5.10)
        CPPFLAGS = -D _SOLARIS10_  -g
        PLATFORM_LIBS = -lsocket -lnsl
    endif
endif

#if this is IRIX
ifeq ($(UNAME_S),irix)
    UNAME_R := $(shell uname -r)
    ifeq ($(UNAME_R), 6.5)
        CPPFLAGS = -D _IRIX6_ -g
    endif
endif

#if this is NETBSD
ifeq ($(UNAME_S),netbsk)
	CPPFLAGS = -D _NETBSD_ -g
endif

#if this is NETBSD
ifeq ($(UNAME_S),netbsd)
	CPPFLAGS = -D _NETBSD_ -g
endif

## Object & Libraries #########################################

LIB_CX_PLATFORM_LIB_DIR=../../lib/$(UNAME_S)_$(ARCH)

APP_OBJECT_DIR=$(UNAME_S)_$(ARCH)

LIB_CX_BASE_NAME=libcx_base.a
LIB_CX_NET_NAME=libcx_net.a

CX_LIBS = \
	$(LIB_CX_PLATFORM_LIB_DIR)/$(LIB_CX_NET_NAME) \
	$(LIB_CX_PLATFORM_LIB_DIR)/$(LIB_CX_BASE_NAME)


ALL_LIBS = $(CX_LIBS) $(PLATFORM_LIBS)

OBJECTS = \
	$(APP_OBJECT_DIR)/$(APP).o


## Targets ##################################################


ALL: MAKE_OBJ_DIR $(APP_OBJECT_DIR)/$(APP)


MAKE_OBJ_DIR:
	mkdir -p $(APP_OBJECT_DIR)


cleanupmac:
	$(RM) ._*

clean:
	$(RM) \
	$(APP_OBJECT_DIR)/$(APP) \
	$(APP_OBJECT_DIR)/*.o   \
	$(APP_OBJECT_DIR)/*.dbx \
	$(APP_OBJECT_DIR)/*.i   \
	$(APP_OBJECT_DIR)/*.ixx \
	$(APP_OBJECT_DIR)/core  \
	$(APP_OBJECT_DIR)/a.out \
	$(APP_OBJECT_DIR)/*.a



$(APP_OBJECT_DIR)/$(APP): $(APP).cpp $(OBJECTS)
	$(CPP) $(CPPFLAGS) $(INC) $(OBJECTS) -o $(APP_OBJECT_DIR)/$(APP) $(ALL_LIBS)


## Conversions################### ########################


$(APP_OBJECT_DIR)/$(APP).o : $(APP).cpp



.PRECIOUS: $(CX_LIBS)
.SUFFIXES: .cpp .C .cc .cxx .o

$(OBJECTS):
	$(CPP) $(CPPFLAGS) $(INC) -c $? -o $@



test:
	./$(APP_OBJECT_DIR)/$(APP)
