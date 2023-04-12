all: server client

server: serverex.cpp proyect.pb.cc
	g++ -o server serverex.cpp proyect.pb.cc -lpthread -lprotobuf

client: clientex.cpp proyect.pb.cc
	g++ -o client clientex.cpp proyect.pb.cc -lpthread -lprotobuf

proyect.pb.cc: proyect.proto
	protoc -I=. --cpp_out=. proyect.proto