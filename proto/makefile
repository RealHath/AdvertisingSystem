


# proto源文件
PROTOFILE = $(wildcard $(PWD)/*.proto)
PROTOSRCS = $(wildcard *.pb.cc)
PROTODIRS = $(notdir $(PROTOSRCS))
PROTOOBJS = $(patsubst %.pb.cc, %.pb.o, $(PROTODIRS))


.PHONY:all clean

all:
	protoc --proto_path=$(PWD) --cpp_out=$(PWD) $(PROTOFILE)

clean:
	rm -f *.pb.*