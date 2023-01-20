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
#include <sstream>
#include <map>
#define SERVERCPORT "23414"
#define HOSTNAME "127.0.0.1"


int main(){

    std::map <std::string, std::map<std::string, std::string>> user_ee_map;
    std::ifstream user_ee("ee.txt");
    if (user_ee.is_open()){
        while (user_ee.good())
        {
            std::string tmp_line, tmp_course_code, tmp_credit, tmp_professor, tmp_days, tmp_course_name;
            std::getline(user_ee, tmp_line);
            std::stringstream line_stream(tmp_line);
            std::getline(line_stream, tmp_course_code, ',');
            std::getline(line_stream, tmp_credit, ',');
            std::getline(line_stream, tmp_professor, ',');
            std::getline(line_stream, tmp_days, ',');
            std::getline(line_stream, tmp_course_name, ',');
            user_ee_map.insert(std::make_pair(tmp_course_code, std::map<std::string, std::string>()));
            user_ee_map[tmp_course_code].insert(std::make_pair("Credit", tmp_credit));
            user_ee_map[tmp_course_code].insert(std::make_pair("Professor", tmp_professor));
            user_ee_map[tmp_course_code].insert(std::make_pair("Days", tmp_days));
            user_ee_map[tmp_course_code].insert(std::make_pair("CourseName", tmp_course_name));
        }
        user_ee.close();
    }else{
        std::cout << "Can not open the file." << std::endl;
    }

    std::cout << "The ServerEE is up and running using UDP on port " << SERVERCPORT << "." << std::endl;

    int serverEE_socket;
    int getaddr_err_code, bind_err_code, recv_err_code=0, send_to_err_code;
    struct addrinfo serverEE_addrinfo, *serverEE_addrres;
    struct sockaddr_storage severM_addr;
    socklen_t serverM_addr_size = sizeof severM_addr;

    memset(&serverEE_addrinfo, 0, sizeof serverEE_addrres); // code from Beej
    serverEE_addrinfo.ai_family = AF_INET;
    serverEE_addrinfo.ai_socktype = SOCK_DGRAM;
    serverEE_addrinfo.ai_protocol = IPPROTO_UDP;

    getaddr_err_code = getaddrinfo(HOSTNAME, SERVERCPORT, &serverEE_addrinfo, &serverEE_addrres);
    if (getaddr_err_code != 0){
        // std::cout << getaddr_err_code << std::endl;
        perror("getaddrinfo failed");
        exit(1);
    }
    serverEE_socket = socket(AF_INET, SOCK_DGRAM, 0);
    bind_err_code = bind(serverEE_socket, serverEE_addrres->ai_addr, serverEE_addrres->ai_addrlen);
    if (bind_err_code < 0){
        perror("serverM to serverEE bind failed");
        exit(1);
    }
    freeaddrinfo(serverEE_addrres);
    char query_information[20]={0};

    while (true)
    {
        recv_err_code = recvfrom(serverEE_socket, query_information, 20, 0, (struct sockaddr *)&severM_addr, &serverM_addr_size);
        if (recv_err_code < 0){
            perror("serverM to serverC receive failed");
            exit(1);
        }
        query_information[recv_err_code]='\0';
        std::string str_query_information = query_information;
        std::string course_code, query_category;
        course_code = str_query_information.substr(0, str_query_information.find(','));
        query_category = str_query_information.substr(str_query_information.find(',')+1, str_query_information.length()-1);
        std::cout << "The ServerEE received a request from the Main Server about the about the " << query_category << " of " << course_code << std::endl;

        auto target_map = user_ee_map[course_code];
        std::string query_result = target_map[query_category];
        if (query_result.length() == 0){
            std::cout << "Didn't find the course: " << course_code << std::endl;
            query_result = "0Didn't find the course: " + course_code;
        }else{
            std::cout << "The course information has been found: The " << query_category << " of " << course_code << " is " << query_result << std::endl;
            query_result = "1" + query_result;
        }
        send_to_err_code = sendto(serverEE_socket, query_result.c_str(), 256, 0, (const struct sockaddr *)&severM_addr, serverM_addr_size);
        if (send_to_err_code < 0){
            perror("serverEE to serverM send failed");
            exit(1);
        }
        std::cout << "The ServerEE finished sending the response to the Main Server." << std::endl;
    }
    close(serverEE_socket);
    return 0;
}
