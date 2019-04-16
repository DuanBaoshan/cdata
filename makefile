TARGET := hello
OBJS := cdata_list.o cdata_dblist.o cdata_sglist.o cdata_os_adapter.o test.o 

STRIP_CMD := strip

ifeq ($(MAKECMDGOALS), release)
CFLAGS += -D_RELEASE_VERSION_ -D_DEBUG_LEVEL_=3
CXXFLAGS += -D_RELEASE_VERSION_ -D_DEBUG_LEVEL_=3
else
CFLAGS += -g
CXXFLAGS += -g
endif

CFLAGS += -I./ -Wall -Werror
CXXFLAGS += -I./ -Wall -Werror

LD_FLAGS += -lrt -lpthread

all release:$(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LD_FLAGS)
	$(STRIP_TARGET)
clean:
	rm -rf $(TARGET) $(OBJS)

ifeq ($(MAKECMDGOALS), release)
define STRIP_TARGET
$(STRIP_CMD) $(TARGET)
endef
endif