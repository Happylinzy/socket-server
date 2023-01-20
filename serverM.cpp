#include "serverM.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#define TCPPORTNUM "25414"
#define UDPPORTNUM "24414"
#define SERVERCPORT 21414
#define SERVERCSPORT 22414
#define SERVEREEPORT 23414
#define HOSTNAME "127.0.0.1"
#define PASS 2

int main(){

    std::cout << "The main server is up and running" << std::endl;
    // TCP socket
    int client_serverM_socket, client_msg_socket;
    int getaddr_err_code, bind_err_code, listern_err_code, read_err_code, send_to_err_code, recv_err_code, send_err_code;
    struct addrinfo serverM_addrinfo, *serverM_addrres;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size;
    client_addr_size = sizeof client_addr;
    memset(&serverM_addrinfo, 0, sizeof serverM_addrinfo); // code from Beej
    serverM_addrinfo.ai_family = AF_UNSPEC;
    serverM_addrinfo.ai_socktype = SOCK_STREAM;
    getaddr_err_code = getaddrinfo(HOSTNAME, TCPPORTNUM, &serverM_addrinfo, &serverM_addrres);
    if (getaddr_err_code != 0){
        // std::cout << "Error code " << getaddr_err_code << std::endl;
        perror("serverM getaddrinfo failed");
        exit(1);
    }
    client_serverM_socket = socket(serverM_addrres->ai_family, SOCK_STREAM, 0);
    bind_err_code = bind(client_serverM_socket, serverM_addrres->ai_addr, serverM_addrres->ai_addrlen);
    if (bind_err_code < 0){
        perror("serverM bind failed");
        exit(1);
    }
    listern_err_code = listen(client_serverM_socket, 10);
    if (listern_err_code < 0){
        perror("serverM listern failed");
        exit(1);
    }
    
    // UDP socket to serverC
    int serverM_UDP_socket;
    struct sockaddr_in serverC_addrinfo;
    serverC_addrinfo.sin_family = AF_INET;
    serverC_addrinfo.sin_port = htons(SERVERCPORT);
    serverC_addrinfo.sin_addr.s_addr = inet_addr(HOSTNAME);
    serverM_UDP_socket = socket(AF_INET, SOCK_DGRAM, 0);
    char user_name[64]={0};
    char user_password[64]={0};
    char name_password[256]={0};
    int serverC_authen_res[10] = {0};
    socklen_t serverC_addrsize = sizeof serverC_addrinfo;
    std::string str_user_name, str_user_password;
    std::string str_name_pasword_middle = "*##################################################*";
    while (true)
    {
        client_msg_socket = accept(client_serverM_socket, (struct sockaddr *)&client_addr, &client_addr_size);
        read_err_code = recv(client_msg_socket, name_password, 256, 0);
        if (read_err_code < 0){
            perror("client to serverM recv failed");
            exit(1);
        }
        name_password[read_err_code]='\0';
        std::string str_name_password = name_password;
        str_user_name = str_name_password.substr(0, str_name_password.find("*##################################################*"));
        str_user_password = str_name_password.substr(str_name_password.find("*##################################################*") + 52, str_name_password.length());
        std::cout << "The main server received the authentication for " << str_user_name << " using TCP over port " << TCPPORTNUM << std::endl;

        strcpy(user_name, str_user_name.c_str());
        strcpy(user_password, str_user_password.c_str());
        EncryptChars(user_name);
        EncryptChars(user_password);
        std::string encrypted_name_password = (std::string)user_name + str_name_pasword_middle + (std::string)user_password;

        send_to_err_code = sendto(serverM_UDP_socket, encrypted_name_password.c_str(), 256, 0, (const struct sockaddr *)&serverC_addrinfo, sizeof(serverC_addrinfo));
        if (send_to_err_code < 0){
            perror("send failed");
            exit(1);
        }   
        std::cout << "The main server sent an authentication request to serverC." << std::endl;

        recv_err_code = recvfrom(serverM_UDP_socket, serverC_authen_res, 10, 0, (struct sockaddr *)&serverC_addrinfo, &serverC_addrsize);
        if (recv_err_code < 0){
            perror("serverC to serverM receive failed");
            exit(1);
        } 
        std::cout << "The main server received the result of the authentication request from ServerC using UDP over port " << UDPPORTNUM << "." << std::endl;

        send_err_code = send(client_msg_socket, serverC_authen_res, 10, 0);
        if (send_err_code < 0){
            perror("serverM to client send failed");
            exit(1);
        }  
        std::cout << "The main server sent the authentication result to the client." << std::endl;
        close(client_msg_socket);
        if (*serverC_authen_res == PASS){
            break;
        }
    }
    // UDP socket to serverCS
    struct sockaddr_in serverCS_addrinfo;
    serverCS_addrinfo.sin_family = AF_INET;
    serverCS_addrinfo.sin_port = htons(SERVERCSPORT);
    serverCS_addrinfo.sin_addr.s_addr = inet_addr(HOSTNAME);
    // UDP socket to serverEE
    struct sockaddr_in serverEE_addrinfo;
    serverEE_addrinfo.sin_family = AF_INET;
    serverEE_addrinfo.sin_port = htons(SERVEREEPORT);
    serverEE_addrinfo.sin_addr.s_addr = inet_addr(HOSTNAME);
    while (true)
    {
        client_msg_socket = accept(client_serverM_socket, (struct sockaddr *)&client_addr, &client_addr_size);
        char query_information[20]={0};
        read_err_code = 0;
        while (read_err_code == 0)
        {
            read_err_code = recv(client_msg_socket, query_information, 20, 0);
            if (read_err_code < 0){
                perror("client to serverM recv failed");
                exit(1);
            }
        }
        query_information[read_err_code]='\0';
        std::string str_query_information = query_information;
        std::string course_code = str_query_information.substr(0, str_query_information.find(','));
        std::string query_category = str_query_information.substr(str_query_information.find(',')+1, str_query_information.length()-1);
        std::cout << "The main server received from " << str_user_name << " to query course " << course_code << " about " << 
                    query_category << "using TCP over port " << TCPPORTNUM << "."<< std::endl;

        std::string department = course_code.substr(0,2);
        // std::cout << "department is " << department << std::endl;


        char query_result[256] = {0};

        if (department == "EE"){
            send_to_err_code = sendto(serverM_UDP_socket, query_information, 20, 0, (const struct sockaddr *)&serverEE_addrinfo, sizeof(serverEE_addrinfo));
            if (send_to_err_code < 0){
                perror("send failed");
                exit(1);
            }  
            std::cout << "The main server sent a request to serverEE" << std::endl;
            socklen_t serverEE_addrsize = sizeof serverEE_addrinfo;
            recv_err_code = recvfrom(serverM_UDP_socket, query_result, 256, 0, (struct sockaddr *)&serverEE_addrinfo, &serverEE_addrsize);
            if (recv_err_code < 0){
                perror("serverC to serverM receive failed");
                exit(1);
            }
            std::cout << "The main server received the response from serverEE using UDP over port " << UDPPORTNUM << "." << std::endl;
        }else{
            send_to_err_code = sendto(serverM_UDP_socket, query_information, 20, 0, (const struct sockaddr *)&serverCS_addrinfo, sizeof(serverCS_addrinfo));
            if (send_to_err_code < 0){
                perror("send failed");
                exit(1);
            }
            std::cout << "The main server sent a request to serverCS" << std::endl;
            socklen_t serverCS_addrsize = sizeof serverCS_addrinfo;
            recv_err_code = recvfrom(serverM_UDP_socket, query_result, 256, 0, (struct sockaddr *)&serverCS_addrinfo, &serverCS_addrsize);
            if (recv_err_code < 0){
                perror("serverC to serverM receive failed");
                exit(1);
            }
            std::cout << "The main server received the response from serverCS using UDP over port " << UDPPORTNUM << "." << std::endl;
        }

        send_err_code = send(client_msg_socket, query_result, 256, 0);
        if (send_err_code < 0){
            perror("serverM to client send failed");
            exit(1);
        }  
        std::cout << "The main server sent the query information to the client." << std::endl;
        close(client_msg_socket);
    }
    close(serverM_UDP_socket);
    shutdown(client_serverM_socket, SHUT_RDWR);
}




void EncryptChars(char *chars){
    for (size_t i = 0; i < strlen(chars); i++)
    {
        chars[i] = EncryptChar(chars[i]);
    }
}

char EncryptChar(const char c){
    if (c >= '0' && c <= '9'){
        return c + 4 > '9' ? c-6 : c+4;
    }else if (c >= 'a' && c <= 'z'){
        return c + 4 > 'z' ? c-22 : c+4;
    }else if (c >= 'A' && c <= 'Z'){
        return c + 4 > 'Z' ? c-22 : c+4;
    }
    return c;
}