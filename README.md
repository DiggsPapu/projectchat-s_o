# PROYECTO CHAT

## SISTEMAS OPERATIVOS

## Compilacion del programa

Para correr el programa debe ejecutar los siguientes comandos:

- g++ -std=c++17 serverex.cpp project.pb.cc -o server `pkg-config --cflags --libs protobuf`
- g++ -std=c++17 clientex.cpp project.pb.cc -o client `pkg-config --cflags --libs protobuf`

luego de eso, en el root del directorio, ingrese los siguientes comandos:

Para abrir el servidor:
./server 1404

Para abrir el cliente:
./client dieggspapu 127.0.0.1 1404


To copy stuff: "/mnt/c/Users/Windows 10/Documents/UVG/CODING/Semestre 5"
