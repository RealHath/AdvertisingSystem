include $(BRPC_PATH)/config.mk

WORKPATH = $(HOME)/AdvertisingSystem

# cpp源文件
SRCS = $(wildcard *.cpp $(PWD)/ao/*.cpp $(WORKPATH)/util/ao/*.cpp)
DIRS = $(notdir $(SRCS))
OBJS = $(patsubst %.cpp, %.o, $(DIRS))

# proto源文件
PROTOFILE = $(wildcard $(WORKPATH)/proto/*.proto)
PROTOSRCS = $(wildcard *.pb.cc)
PROTODIRS = $(notdir $(PROTOSRCS))
PROTOOBJS = $(patsubst %.pb.cc, %.pb.o, $(PROTODIRS))

# -I指定头文件目录
INCLUDE = -I$(PWD) -I$(PWD)/include -I$(HDRS) -I$(WORKPATH)/proto  \
-I$(BRPC_INCLUDE_PATH) -I$(BRPC_PATH)/src \
-I$(WORKPATH)/util\
-I$(WORKPATH)/util/include\

# -L指定库文件目录，-l指定静态库名字(去掉文件名中的lib前缀和.a后缀)
LIB += -L$(WORKPATH)/lib -L$(LIBS) -L/usr/lib64/mysql\
-lprotobuf -lbrpc -lgflags -lmysqlclient -ljsoncpp -luuid
# 开启编译warning和设置优化等级
# CFLAGS = -Wall -O2
CFLAGS += $(CPPFLAGS) -DNDEBUG -O2 -D__const__= -pipe -W -Wall -fPIC -fno-omit-frame-pointer -faligned-new\

TARGET = mysql_main

.PHONY:all clean proto

all:proto $(TARGET)
# 链接时候指定库文件目录及库文件名
$(TARGET):$(PROTOOBJS) $(OBJS)
	@echo "> link"
	$(CXX) -o $@ $^ $(LIB) $(DYNAMIC_LINKINGS) $(STATIC_LINKINGS)
	rm -f *.o *.pb.*
 
# 编译时候指定头文件目录
%.o:%.cpp
	@echo "> compile *.o"
	$(CXX) -c $(SRCS) $(INCLUDE) $(CFLAGS) `mysql_config --cflags --libs` 

%.pb.o:%.pb.cc
	@echo "> compile *.pb.o"
	$(CXX) -c $(PROTOSRCS) $(INCLUDE) $(CFLAGS)

clean:
	rm -f *.o $(TARGET) *.pb.*

proto:
	@echo "> compile *.pb.cc"
	$(PROTOC) --proto_path=$(WORKPATH)/proto --cpp_out=$(PWD) $(PROTOFILE)
