include $(BRPC_PATH)/config.mk

WORKPATH = $(HOME)/AdvertisingSystem

# cpp源文件
SRCS = $(wildcard *.cpp $(PWD)/ao/*.cpp)
DIRS = $(notdir $(SRCS))
OBJS = $(patsubst %.cpp, %.o, $(DIRS))

# proto源文件
PROTOFILE = $(wildcard $(WORKPATH)/proto/*.proto)
PROTOSRCS = $(wildcard *.pb.cc)
PROTODIRS = $(notdir $(PROTOSRCS))
PROTOOBJS = $(patsubst %.pb.cc, %.pb.o, $(PROTODIRS))

# -I指定头文件目录
INCLUDE = -I$(PWD) -I$(PWD)/include -I$(HDRS)  \
-I$(BRPC_INCLUDE_PATH) -I$(BRPC_PATH)/src

# -L指定库文件目录，-l指定静态库名字(去掉文件名中的lib前缀和.a后缀)
LIB += -L$(WORKPATH)/lib -L$(LIBS)\
-lprotobuf -lbrpc -lgflags
# 开启编译warning和设置优化等级
# CFLAGS = -Wall -O2
CFLAGS += $(CPPFLAGS) -DNDEBUG -O2 -D__const__= -pipe -W -Wall -Wno-unused-parameter -fPIC -fno-omit-frame-pointer -faligned-new

TARGET = test

.PHONY:all clean

all:clean proto $(TARGET)
# 链接时候指定库文件目录及库文件名
$(TARGET): $(OBJS) $(PROTOOBJS)
	$(CXX) -o $@ $^ $(LIB) $(DYNAMIC_LINKINGS) $(STATIC_LINKINGS)
 
# 编译时候指定头文件目录
%.o:%.cpp
	$(CXX) -c $(SRCS) $(INCLUDE) $(CFLAGS)

%.pb.o:%.pb.cc
	$(CXX) -c $(PROTOSRCS) $(INCLUDE) $(CFLAGS) 

clean:
	rm -f *.o $(TARGET) *.pb.*

proto:
	$(PROTOC) -I=$(WORKPATH)/proto/ --cpp_out=./ $(PROTOFILE)
