## makefile: cmacs ############################################

APP=		key
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
   CPPFLAGS += -D _LINUX_  -g -Wno-deprecated
endif

# if this is darwin
ifeq ($(UNAME_S),darwin)
   CPPFLAGS += -D _OSX_  -g -Wno-deprecated
endif

# if this is darwin
ifeq ($(UNAME_S),sunos)
   CPPFLAGS += -D _SUNOS_  -g
endif


## Object & Libraries #########################################

LIB_CX_PLATFORM_LIB_DIR=../../lib/$(UNAME_S)

APP_OBJECT_DIR=$(UNAME_S)

LIB_CX_DIR=../../lib/linux
LIB_CX_BASE_NAME=libcx_base.a
LIB_CX_NET_NAME=libcx_net.a
LIB_CX_THREAD_NAME=libcx_thread.a
LIB_CX_DB_NAME=libcx_db.a
LIB_CX_NETAPP_NAME=libcx_netapp.a
LIB_CX_COMMANDLINE_NAME=libcx_commandline.a
LIB_CX_KEYBOARD_NAME=libcx_keyboard.a
LIB_CX_EDITBUFFER_NAME=libcx_editbuffer.a

CX_LIBS = \
	$(LIB_CX_PLATFORM_LIB_DIR)/$(LIB_CX_BASE_NAME)\
	$(LIB_CX_PLATFORM_LIB_DIR)/$(LIB_CX_KEYBOARD_NAME)

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


