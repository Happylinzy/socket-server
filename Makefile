CC = g++

all: serverM serverC serverCS serverEE client

serverM: serverM.cpp serverM.h
		$(CC) -o serverM serverM.cpp

serverC: serverC.cpp
		$(CC) -o serverC serverC.cpp

serverCS: serverCS.cpp
		$(CC) -o serverCS serverCS.cpp

serverEE: serverEE.cpp
		$(CC) -o serverEE serverEE.cpp

client: client.cpp
		$(CC) -o client client.cpp

clean:
		$(RM) serverM serverC serverCS serverEE client