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
#include <fstream>
#include <map>
#define SERVERCPORT "21414"
#define HOSTNAME "127.0.0.1"
#define PASS 2
#define FAIL_NO_USER 0
#define FAIL_PASS_NO_MATCH 1


int main(){

    std::cout << "The ServerC is up and running using UDP on port " << SERVERCPORT << "." << std::endl;

    int serverC_socket;
    int getaddr_err_code, bind_err_code, recv_err_code=0, send_to_err_code;
    struct addrinfo serverC_addrinfo, *serverC_addrres;
    struct sockaddr_storage severM_addr;
    socklen_t serverM_addr_size = sizeof severM_addr;

    memset(&serverC_addrinfo, 0, sizeof serverC_addrres); // code from Beej
    serverC_addrinfo.ai_family = AF_INET;
    serverC_addrinfo.ai_socktype = SOCK_DGRAM;
    serverC_addrinfo.ai_protocol = IPPROTO_UDP;

    getaddr_err_code = getaddrinfo(HOSTNAME, SERVERCPORT, &serverC_addrinfo, &serverC_addrres);
    if (getaddr_err_code != 0){
        // std::cout << getaddr_err_code << std::endl;
        perror("getaddrinfo failed");
        exit(1);
    }
    serverC_socket = socket(AF_INET, SOCK_DGRAM, 0);
    bind_err_code = bind(serverC_socket, serverC_addrres->ai_addr, serverC_addrres->ai_addrlen);
    if (bind_err_code < 0){
        perror("serverM to serverC bind failed");
        exit(1);
    }

    freeaddrinfo(serverC_addrres);

    char encrypted_name_password[256]={0};

    std::map <std::string, std::string> user_cred_map;
    std::ifstream user_cred("cred.txt");
    if (user_cred.is_open()){
        while (user_cred.good())
        {
            std::string tmp_line, tmp_user_name, tmp_user_password;
            std::getline(user_cred, tmp_line);
            tmp_user_name = tmp_line.substr(0, tmp_line.find(','));
            tmp_user_password = tmp_line.substr(tmp_line.find(',')+1, tmp_line.length()-1);
            user_cred_map[tmp_user_name] = tmp_user_password;
        }
        user_cred.close();
    }else{
        std::cout << "Can not open the file." << std::endl;
    }

    while (true)
    {
        recv_err_code = recvfrom(serverC_socket, encrypted_name_password, 256, 0, (struct sockaddr *)&severM_addr, &serverM_addr_size);
        if (recv_err_code < 0){
            perror("serverM to serverC receive failed");
            exit(1);
        }
        encrypted_name_password[recv_err_code]='\0';
        std::string str_name_password = encrypted_name_password;
        std::string encrypted_user_name, encrypted_user_password;

        encrypted_user_name = str_name_password.substr(0, str_name_password.find("*##################################################*"));
        encrypted_user_password = str_name_password.substr(str_name_password.find("*##################################################*") + 52, str_name_password.length()-1);

        std::cout << "The ServerC received an authentication request from the Main Server" << std::endl;
        
        int password_check_flag = 0;
        const std::string target_password =  user_cred_map[encrypted_user_name].substr(0, encrypted_user_password.length());
        if (target_password == encrypted_user_password){
            password_check_flag = PASS;
        }else if(target_password.empty()){
            password_check_flag = FAIL_NO_USER;
        }else{
            password_check_flag = FAIL_PASS_NO_MATCH;
        }

        int buffer[10]={password_check_flag};
        for (int i = 0; i < 10; i++)
        {
            buffer[i] = password_check_flag;
        }
        send_to_err_code = sendto(serverC_socket, buffer, 10, 0, (const struct sockaddr *)&severM_addr, serverM_addr_size);
        if (send_to_err_code < 0){
            perror("serverC to serverM send failed");
            exit(1);
        } 
        std::cout << "The ServerC finished sending the response to the Main Server." << std::endl;
    }
    close(serverC_socket);
    return 0;
}
