all: server client

server: serverex.cpp project.pb.cc
	g++ -o server serverex.cpp project.pb.cc -lpthread -lprotobuf

client: clientex.cpp project.pb.cc
	g++ -o client clientex.cpp project.pb.cc -lpthread -lprotobuf

project.pb.cc: project.proto
	protoc -I=. --cpp_out=. project.proto