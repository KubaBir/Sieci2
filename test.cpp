#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

int main() {
    std::vector<std::tuple<std::string, int, std::vector<std::string>, bool>> clients;
    clients.push_back(std::make_tuple("kuba", 4, std::vector<std::string>{"kuba"}, false));

    std::tuple<std::string, int, std::vector<std::string>, bool> user;

    for (const auto &client : clients) {
        if (std::get<0>(client) == "kuba") {
            user = client;
            break;
        }
    }
    clients.erase(
        std::remove_if(
            clients.begin(), clients.end(),
            [&user](const std::tuple<std::string, int, std::vector<std::string>, bool> &client) {
                return client == user;
            }),
        clients.end());

    std::get<3>(user) = true;

    clients.push_back(user);

    for (const auto &client : clients) {
        if (std::get<0>(client) == "kuba") {
            user = client;
            break;
        }
    }
    std::cout << users[1] << "\n";
}
