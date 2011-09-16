#
#	wiiusecpp pywii Makefile
#

#
# Change this to your versions as appropriate.
#
CC = g++
SWIG = swig
PYTHON_VERSION = 2.6

####################################################
#
# You should not need to edit below this line.
#
####################################################

#
# Universal cflags
#
CFLAGS = -Wall -pipe -fPIC -funroll-loops

SWIGFLAGS =  -python -c++

ifeq ($(debug),1)
        OBJ_PREFIX = debug
        CFLAGS += -g -pg -DWITH_WIIUSE_DEBUG
else
        OBJ_PREFIX = release
        CFLAGS += -O2
endif

OBJ_DIR = $(OBJ_PREFIX)-$(shell $(CC) -v 2>&1|grep ^Target:|cut -d' ' -f2)

#
# Linking flags
#
LDFLAGS = -L../src/$(OBJ_DIR) -shared -lwiiusecpp

#
# Target binaries (always created as BIN)
#
BIN = ./$(OBJ_DIR)/_pywii.so

#
# Inclusion paths.
#
INCLUDES = -I/usr/include/python$(PYTHON_VERSION)/ -I../src/
SWIGINCLUDES = -I../src/

#
# Generate a list of object files
#
OBJS = $(OBJ_DIR)/pywii_wrap.o

###############################
#
# Build targets.
#
###############################

all: $(BIN)

clean:
	@-rm $(OBJS) 2> /dev/null

distclean:
	@-rm -rf debug-* release-* 2> /dev/null

install:
	@if [ -e $(BIN) ] && [ -e $(OBJ_DIR)/pywii.py ]; then \
		cp -v $(BIN) $(OBJ_DIR)/pywii.py /usr/lib/python$(PYTHON_VERSION)/ ; \
	fi

$(BIN): mkdir $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(BIN)

$(OBJ_DIR)/%.o: $(OBJ_DIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%_wrap.cpp: %.i
	$(SWIG) $(SWIGFLAGS) $(SWIGINCLUDES) -o $@ $< 

mkdir:
	@if [ ! -d $(OBJ_DIR) ]; then \
		mkdir $(OBJ_DIR); \
	fi

