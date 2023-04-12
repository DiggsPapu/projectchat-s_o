# PROYECTO CHAT

## SISTEMAS OPERATIVOS


# projectchat-s_o
COMPILATION
g++ -std=c++17 serverex.cpp project.pb.cc -o server `pkg-config --cflags --libs protobuf`
g++ -std=c++17 clientex.cpp project.pb.cc -o client `pkg-config --cflags --libs protobuf`
./server 1404
./client dieggspapu 127.0.0.1 1404
To copy stuff: "/mnt/c/Users/Windows 10/Documents/UVG/CODING/Semestre 5"
