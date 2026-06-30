## makefile: cxutfstringlist_test ############################################

APP=		cxutfstringlist_test
CPP=        g++
SHELL=		/bin/sh
MOVE=		mv
COPY=		ln -s
RM=			rm -rf
TOUCH=		touch
LIBRARIAN=	ar rvu
INC=        -I../..

## Platform ###################################################


# Platform detection + flags live in cx/platform.mk (single source of truth).
include ../../cx/platform.mk

## Object & Libraries #########################################

LIB_CX_PLATFORM_LIB_DIR=../../lib/$(PLATFORM)

APP_OBJECT_DIR=$(PLATFORM)

LIB_CX_BASE_NAME=libcx_base.a
LIB_CX_EDITBUFFER_NAME=libcx_editbuffer.a

CX_LIBS = \
	$(LIB_CX_PLATFORM_LIB_DIR)/$(LIB_CX_EDITBUFFER_NAME) \
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
