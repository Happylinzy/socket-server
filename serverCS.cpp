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
#define SERVERCPORT "22414"
#define HOSTNAME "127.0.0.1"


int main(){

    std::map <std::string, std::map<std::string, std::string>> user_cs_map;
    std::ifstream user_cs("cs.txt");
    if (user_cs.is_open()){
        while (user_cs.good())
        {
            std::string tmp_line, tmp_course_code, tmp_credit, tmp_professor, tmp_days, tmp_course_name;
            std::getline(user_cs, tmp_line);
            std::stringstream line_stream(tmp_line);
            std::getline(line_stream, tmp_course_code, ',');
            std::getline(line_stream, tmp_credit, ',');
            std::getline(line_stream, tmp_professor, ',');
            std::getline(line_stream, tmp_days, ',');
            std::getline(line_stream, tmp_course_name, ',');
            user_cs_map.insert(std::make_pair(tmp_course_code, std::map<std::string, std::string>()));
            user_cs_map[tmp_course_code].insert(std::make_pair("Credit", tmp_credit));
            user_cs_map[tmp_course_code].insert(std::make_pair("Professor", tmp_professor));
            user_cs_map[tmp_course_code].insert(std::make_pair("Days", tmp_days));
            user_cs_map[tmp_course_code].insert(std::make_pair("CourseName", tmp_course_name));
        }
        user_cs.close();
    }else{
        std::cout << "Can not open the file." << std::endl;
    }

    std::cout << "The ServerCS is up and running using UDP on port " << SERVERCPORT << "." << std::endl;

    int serverCS_socket;
    int getaddr_err_code, bind_err_code, recv_err_code=0, send_to_err_code;
    struct addrinfo serverCS_addrinfo, *serverCS_addrres;
    struct sockaddr_storage severM_addr;
    socklen_t serverM_addr_size = sizeof severM_addr;

    memset(&serverCS_addrinfo, 0, sizeof serverCS_addrres); // code from Beej
    serverCS_addrinfo.ai_family = AF_INET;
    serverCS_addrinfo.ai_socktype = SOCK_DGRAM;
    serverCS_addrinfo.ai_protocol = IPPROTO_UDP;

    getaddr_err_code = getaddrinfo(HOSTNAME, SERVERCPORT, &serverCS_addrinfo, &serverCS_addrres);
    if (getaddr_err_code != 0){
        // std::cout << getaddr_err_code << std::endl;
        perror("getaddrinfo failed");
        exit(1);
    }
    serverCS_socket = socket(AF_INET, SOCK_DGRAM, 0);
    bind_err_code = bind(serverCS_socket, serverCS_addrres->ai_addr, serverCS_addrres->ai_addrlen);
    if (bind_err_code < 0){
        perror("serverM to serverC bind failed");
        exit(1);
    }
    freeaddrinfo(serverCS_addrres);
    char query_information[20]={0};

    while (true)
    {
        recv_err_code = recvfrom(serverCS_socket, query_information, 20, 0, (struct sockaddr *)&severM_addr, &serverM_addr_size);
        if (recv_err_code < 0){
            perror("serverM to serverC receive failed");
            exit(1);
        }
        query_information[recv_err_code]='\0';
        std::string str_query_information = query_information;
        std::string course_code, query_category;
        course_code = str_query_information.substr(0, str_query_information.find(','));
        query_category = str_query_information.substr(str_query_information.find(',')+1, str_query_information.length()-1);
        std::cout << "The ServerCS received a request from the Main Server about the about the " << query_category << " of " << course_code << std::endl;
        // std::cout << query_category << query_category.length() << std::endl;

        auto target_map = user_cs_map[course_code];
        std::string query_result = target_map[query_category];
        // std::cout << "query_result is " << query_result << std::endl;
        if (query_result.length() == 0){
            std::cout << "Didn't find the course: " << course_code << std::endl;
            query_result = "0Didn't find the course: " + course_code;
        }else{
            std::cout << "The course information has been found: The " << query_category << " of " << course_code << " is " << query_result << std::endl;
            query_result = "1" + query_result;
        }
        send_to_err_code = sendto(serverCS_socket, query_result.c_str(), 256, 0, (const struct sockaddr *)&severM_addr, serverM_addr_size);
        if (send_to_err_code < 0){
            perror("serverC to serverM send failed");
            exit(1);
        }
        std::cout << "The ServerC finished sending the response to the Main Server." << std::endl;
    }
    close(serverCS_socket);
    return 0;
}
