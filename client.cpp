// #include <serverC.h>
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
#include <string>
#include <iostream>
#define TCPPORTNUM "25414"
#define HOSTNAME "127.0.0.1"
#define PASS 2
#define FAIL_NO_USER 0
#define FAIL_PASS_NO_MATCH 1

int main(){
    std::cout << "The client is up and running." << std::endl;
    std::string user_name, user_password;
    int client_socket;
    for (int n = 2; n >= 0; n--)
    {
        std::cout << "Please enter the username:";
        std::getline(std::cin, user_name, '\n');        // username 5-50 chars  lower case characters
        std::cout << "Please enter the password:";
        std::getline(std::cin, user_password, '\n');        // password 5-50 chars  case sensitive, allow special characters

        int getadd_err_code, connect_err_code, send_err_code, recv_err_code;
        struct addrinfo serverM_addrinfo, *serverM_addrres;
        memset(&serverM_addrinfo, 0, sizeof serverM_addrinfo);
        serverM_addrinfo.ai_family = AF_UNSPEC;
        serverM_addrinfo.ai_socktype = SOCK_STREAM;
        getadd_err_code = getaddrinfo(HOSTNAME, TCPPORTNUM, &serverM_addrinfo, &serverM_addrres);
        if (getadd_err_code < 0){
            perror("getaddrinfo failed");
            exit(1);
        }
        client_socket = socket(serverM_addrres->ai_family, SOCK_STREAM, 0);
        connect_err_code = connect(client_socket, serverM_addrres->ai_addr, serverM_addrres->ai_addrlen);
        if (connect_err_code < 0){
            perror("client to serverM connect failed");
            exit(1);
        }
        freeaddrinfo(serverM_addrres);
        // char name_password_middle[50] = {0};
        // for (int i = 0; i < 50; i++){
        //     name_password_middle[i] = '#';
        // }
        std::string name_password = user_name + "*##################################################*" + user_password;
        send_err_code = send(client_socket, name_password.c_str(), 256, 0);
        if (send_err_code < 0){
            perror("client to serverM send failed");
            close(client_socket);
            exit(1);
        }
        std::cout << user_name << " sent an authentication request to the main server." << std::endl;

        int authen_res[10] = {0};
        recv_err_code = recv(client_socket, authen_res, 10, 0);
        if (recv_err_code < 0){
            perror("serverM to client recv failed");
            close(client_socket);
            exit(1);
        }
        std::cout << user_name << " received the result of authentication using TCP over port " << TCPPORTNUM << ".";
        switch (authen_res[0])
        {
        case PASS:
            std::cout << " Authentication is successful" << std::endl;
            break;
        case FAIL_NO_USER:
            std::cout << " Authentication failed: Username Does not exist" << std::endl;
            std::cout << "Attempts remaining:" << n << std::endl;
            if (n == 0){
                std::cout << "Authentication Failed for 3 attempts. Client will shut down" << std::endl;
                close(client_socket);
                exit(1);
            }
            continue;
        case FAIL_PASS_NO_MATCH:
            std::cout << " Authentication failed: Password does not match" << std::endl;
            std::cout << "Attempts remaining:" << n << std::endl;
            if (n == 0){
                std::cout << "Authentication Failed for 3 attempts. Client will shut down" << std::endl;
                close(client_socket);
                exit(1);
            }
            continue;
        default:
            break;
        }
        if (authen_res[0] = PASS){
            close(client_socket);
            break;
        }

    }
    while (true)
    {
        std::string course_code;
        std::string query_category;
        std::string query_information;
        std::cout << "Please enter the course code to query:" << std::endl;
        std::cin >> course_code;
        std::cout << "Please enter the category (Credit / Professor / Days / CourseName):" << std::endl;
        std::cin >> query_category;

        int send_err_code, recv_err_code, getadd_err_code, connect_err_code;
        struct addrinfo serverM_addrinfo, *serverM_addrres;
        memset(&serverM_addrinfo, 0, sizeof serverM_addrinfo);
        serverM_addrinfo.ai_family = AF_UNSPEC;
        serverM_addrinfo.ai_socktype = SOCK_STREAM;
        getadd_err_code = getaddrinfo(HOSTNAME, TCPPORTNUM, &serverM_addrinfo, &serverM_addrres);
        if (getadd_err_code < 0){
            perror("getaddrinfo failed");
            exit(1);
        }
        client_socket = socket(serverM_addrres->ai_family, SOCK_STREAM, 0);
        connect_err_code = connect(client_socket, serverM_addrres->ai_addr, serverM_addrres->ai_addrlen);
        if (connect_err_code < 0){
            perror("client to serverM connect failed");
            exit(1);
        }
        freeaddrinfo(serverM_addrres);

        query_information = course_code + "," + query_category;
        send_err_code = send(client_socket, query_information.c_str(), 20, 0);
        if (send_err_code < 0){
            perror("client to serverM send failed");
            close(client_socket);
            exit(1);
        }
        std::cout << user_name << " sent an authentication request to the main server." << std::endl;

        char query_result[256]={0};
        recv_err_code = recv(client_socket, query_result, 256, 0);
        if (recv_err_code < 0){
            perror("serverM to client recv failed");
            close(client_socket);
            exit(1);
        }
        std::cout << "The client received the response from the Main server using TCP over port " << TCPPORTNUM << std::endl;
        
        std::string str_query_result = query_result;
        if (str_query_result[0] == '0'){
            std::cout << "Didn't find the course: " << course_code << std::endl;
        }else{
            str_query_result = str_query_result.substr(1, str_query_result.length() - 1);
            std::cout << "The " << query_category << " of " << course_code << " is " << str_query_result << std::endl;
        }

        close(client_socket);
        std::cout << "-----Start a new request-----" << std::endl;
    }
}