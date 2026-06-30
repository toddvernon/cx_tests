## makefile: cxcommandcompleter_example ############################################

APP=		cxcommandcompleter_example
TEST_APP=	cxcommandcompleter_test
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
LIB_CX_COMMANDCOMPLETER_NAME=libcx_commandcompleter.a

CX_LIBS = \
	$(LIB_CX_PLATFORM_LIB_DIR)/$(LIB_CX_COMMANDCOMPLETER_NAME) \
	$(LIB_CX_PLATFORM_LIB_DIR)/$(LIB_CX_BASE_NAME)


ALL_LIBS = $(CX_LIBS) $(PLATFORM_LIBS)

OBJECTS = \
	$(APP_OBJECT_DIR)/$(APP).o

TEST_OBJECTS = \
	$(APP_OBJECT_DIR)/$(TEST_APP).o


## Targets ##################################################


ALL: MAKE_OBJ_DIR $(APP_OBJECT_DIR)/$(APP) $(APP_OBJECT_DIR)/$(TEST_APP)


MAKE_OBJ_DIR:
	mkdir -p $(APP_OBJECT_DIR)


cleanupmac:
	$(RM) ._*

clean:
	$(RM) \
	$(APP_OBJECT_DIR)/$(APP) \
	$(APP_OBJECT_DIR)/$(TEST_APP) \
	$(APP_OBJECT_DIR)/*.o   \
	$(APP_OBJECT_DIR)/*.dbx \
	$(APP_OBJECT_DIR)/*.i   \
	$(APP_OBJECT_DIR)/*.ixx \
	$(APP_OBJECT_DIR)/core  \
	$(APP_OBJECT_DIR)/a.out \
	$(APP_OBJECT_DIR)/*.a



$(APP_OBJECT_DIR)/$(APP): $(OBJECTS)
	$(CPP) $(CPPFLAGS) $(INC) $(OBJECTS) -o $(APP_OBJECT_DIR)/$(APP) $(ALL_LIBS)

$(APP_OBJECT_DIR)/$(TEST_APP): $(TEST_OBJECTS)
	$(CPP) $(CPPFLAGS) $(INC) $(TEST_OBJECTS) -o $(APP_OBJECT_DIR)/$(TEST_APP) $(ALL_LIBS)


## Conversions################### ########################


$(APP_OBJECT_DIR)/$(APP).o : $(APP).cpp
	$(CPP) $(CPPFLAGS) $(INC) -c $? -o $@

$(APP_OBJECT_DIR)/$(TEST_APP).o : $(TEST_APP).cpp
	$(CPP) $(CPPFLAGS) $(INC) -c $? -o $@


.PRECIOUS: $(CX_LIBS)
.SUFFIXES: .cpp .C .cc .cxx .o


example:
	./$(APP_OBJECT_DIR)/$(APP)

test:
	./$(APP_OBJECT_DIR)/$(TEST_APP)
