/* 
 * Copyright (c) 2024 Haopeng Huang - All Rights Reserved
 * 
 * Licensed under the MIT License. You may use, distribute,
 * and modify this code under the terms of the MIT license.
 * You should have received a copy of the MIT license along
 * with this file. If not, see:
 * 
 * https://opensource.org/licenses/MIT
 */
#include "../_config.h"
#include "../errno.h"
#include "../mpistubs.h"
#include "../net.h"
#include "../memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

MYS_PUBLIC int mys_tcp_server(const char *bind_addr, int bind_port)
{
    return mys_tcp_server2(bind_addr, bind_port, SO_REUSEADDR);
}

MYS_PUBLIC int mys_tcp_server2(const char *bind_addr, int bind_port, int sock_opt)
{
    int sock = -1;
    int enable = 1;
    int family = strchr(bind_addr, ':') ? AF_INET6 : AF_INET;
    struct sockaddr_storage addr;
    socklen_t addr_len = 0;

    if ((sock = socket(family, SOCK_STREAM, 0)) == -1) {
        return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, sock_opt, &enable, sizeof(enable)) == -1) {
        close(sock);
        return -1;
    }

    if (family == AF_INET) {
        struct sockaddr_in *addr_in = (struct sockaddr_in *)&addr;
        memset(addr_in, 0, sizeof(*addr_in));
        addr_in->sin_family = AF_INET;
        addr_in->sin_port = htons(bind_port);
        if (inet_pton(AF_INET, bind_addr, &addr_in->sin_addr) <= 0) {
            close(sock);
            return -1;
        }
        addr_len = sizeof(*addr_in);
    } else if (family == AF_INET6) {
        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)&addr;
        memset(addr_in6, 0, sizeof(*addr_in6));
        addr_in6->sin6_family = AF_INET6;
        addr_in6->sin6_port = htons(bind_port);
        if (inet_pton(AF_INET6, bind_addr, &addr_in6->sin6_addr) <= 0) {
            close(sock);
            return -1;
        }
        addr_len = sizeof(*addr_in6);

        // Optionally set the socket to accept IPv4-mapped addresses
        if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &enable, sizeof(enable)) == -1) {
            close(sock);
            return -1;
        }
    }

    if (bind(sock, (struct sockaddr *)&addr, addr_len) < 0) {
        close(sock);
        return -1;
    }

    if (listen(sock, 1) < 0) {
        close(sock);
        return -1;
    }

    return sock;
}

MYS_PUBLIC int mys_tcp_client(const char *server_addr, int server_port)
{
    int sock = -1;
    struct sockaddr_storage addr;
    socklen_t addr_len;

    int family = strchr(server_addr, ':') ? AF_INET6 : AF_INET;

    if ((sock = socket(family, SOCK_STREAM, 0)) == -1) {
        return -1;
    }

    if (family == AF_INET) {
        struct sockaddr_in *addr_in = (struct sockaddr_in *)&addr;
        memset(addr_in, 0, sizeof(*addr_in));
        addr_in->sin_family = AF_INET;
        addr_in->sin_port = htons(server_port);
        if (inet_pton(AF_INET, server_addr, &addr_in->sin_addr) <= 0) {
            close(sock);
            return -1;
        }
        addr_len = sizeof(*addr_in);
    } else if (family == AF_INET6) {
        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)&addr;
        memset(addr_in6, 0, sizeof(*addr_in6));
        addr_in6->sin6_family = AF_INET6;
        addr_in6->sin6_port = htons(server_port);
        if (inet_pton(AF_INET6, server_addr, &addr_in6->sin6_addr) <= 0) {
            close(sock);
            return -1;
        }
        addr_len = sizeof(*addr_in6);
    }

    if (connect(sock, (struct sockaddr *)&addr, addr_len) < 0) {
        close(sock);
        return -1;
    }

    return sock;
}

MYS_PUBLIC int mys_udp_server(const char *bind_addr, int bind_port)
{
    int sock = -1;
    int family = strchr(bind_addr, ':') ? AF_INET6 : AF_INET;
    struct sockaddr_storage addr;
    socklen_t addr_len;

    if ((sock = socket(family, SOCK_DGRAM, 0)) == -1) {
        return -1;
    }

    if (family == AF_INET) {
        struct sockaddr_in *addr_in = (struct sockaddr_in *)&addr;
        memset(addr_in, 0, sizeof(*addr_in));
        addr_in->sin_family = AF_INET;
        addr_in->sin_port = htons(bind_port);
        if (inet_pton(AF_INET, bind_addr, &addr_in->sin_addr) <= 0) {
            close(sock);
            return -1;
        }
        addr_len = sizeof(*addr_in);
    } else if (family == AF_INET6) {
        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)&addr;
        memset(addr_in6, 0, sizeof(*addr_in6));
        addr_in6->sin6_family = AF_INET6;
        addr_in6->sin6_port = htons(bind_port);
        if (inet_pton(AF_INET6, bind_addr, &addr_in6->sin6_addr) <= 0) {
            close(sock);
            return -1;
        }
        addr_len = sizeof(*addr_in6);
    }

    if (bind(sock, (struct sockaddr *)&addr, addr_len) < 0) {
        close(sock);
        return -1;
    }

    return sock;
}

MYS_PUBLIC int mys_udp_client(const char *server_addr, int server_port)
{
    int sock = -1;
    struct sockaddr_storage addr;

    int family = strchr(server_addr, ':') ? AF_INET6 : AF_INET;

    if ((sock = socket(family, SOCK_DGRAM, 0)) == -1) {
        return -1;
    }

    if (family == AF_INET) {
        struct sockaddr_in *addr_in = (struct sockaddr_in *)&addr;
        memset(addr_in, 0, sizeof(*addr_in));
        addr_in->sin_family = AF_INET;
        addr_in->sin_port = htons(server_port);
        if (inet_pton(AF_INET, server_addr, &addr_in->sin_addr) <= 0) {
            close(sock);
            return -1;
        }
    } else if (family == AF_INET6) {
        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)&addr;
        memset(addr_in6, 0, sizeof(*addr_in6));
        addr_in6->sin6_family = AF_INET6;
        addr_in6->sin6_port = htons(server_port);
        if (inet_pton(AF_INET6, server_addr, &addr_in6->sin6_addr) <= 0) {
            close(sock);
            return -1;
        }
    }

    return sock;
}


/* mpic++ -I${MYS_DIR}/include test-tcp-main.cpp && mpirun -n 2 ./a.out

#define MYS_IMPL
#include <mys.h>

#include <stdio.h>
#include <mpi.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <string.h>
#include <unistd.h>

void run_tcp_server(const char *bind_addr, int bind_port) {
    int server_fd, client_fd;
    int bytes_received;
    char buffer[1024] = {0};
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    AS_NE_INT(-1, server_fd = mys_tcp_server(bind_addr, bind_port));

    MPI_Barrier(MPI_COMM_WORLD);
    AS_NE_INT(-1, client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len));
    ILOG_SELF("TCP Server: Connection accepted from %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    int flag = 1;
    AS_NE_INT(-1, setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)));
    AS_NE_INT(-1, bytes_received = recv(client_fd, buffer, sizeof(buffer), 0));
    ILOG_SELF("TCP Server: Received '%s' (%d bytes)", buffer, bytes_received);

    close(client_fd);
    close(server_fd);
}

void run_tcp_client(const char *server_addr, int server_port) {
    int client_fd;
    int bytes_sent;
    char message[] = "Hello from TCP client!";

    MPI_Barrier(MPI_COMM_WORLD);
    AS_NE_INT(-1, client_fd = mys_tcp_client(server_addr, server_port));

    int flag = 1;
    AS_NE_INT(-1, setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)));
    AS_NE_INT(-1, bytes_sent = send(client_fd, message, strlen(message), 0));
    ILOG_SELF("TCP Client: Sent '%s' (%d bytes)", message, bytes_sent);

    close(client_fd);
}

void run_udp_server(const char *bind_addr, int bind_port) {
    int server_fd;
    int bytes_received;
    char buffer[1024];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_fd = mys_udp_server(bind_addr, bind_port);

    MPI_Barrier(MPI_COMM_WORLD);
    bytes_received = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_addr_len);
    ILOG_SELF("UDP Server: Received '%s' (%d bytes) from %s:%d", buffer, bytes_received, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    close(server_fd);
}

void run_udp_client(const char *server_addr, int server_port) {
    int client_fd;
    int bytes_sent;
    char message[] = "Hello from UDP client!";
    struct sockaddr_in server_addr_in;

    client_fd = mys_udp_client(server_addr, server_port);

    memset(&server_addr_in, 0, sizeof(server_addr_in));
    server_addr_in.sin_family = AF_INET;
    server_addr_in.sin_port = htons(server_port);
    inet_pton(AF_INET, server_addr, &server_addr_in.sin_addr);

    MPI_Barrier(MPI_COMM_WORLD);
    bytes_sent = sendto(client_fd, message, strlen(message), 0, (struct sockaddr *)&server_addr_in, sizeof(server_addr_in));
    ILOG_SELF("UDP Client: Sent '%s' (%d bytes)", message, bytes_sent);

    close(client_fd);
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Set the server's IP to one of `hostname -I` if it run remotely
    if (rank == 0) {
        run_tcp_server("127.0.0.1", 31101);
    } else if (rank == 1) {
        run_tcp_client("127.0.0.1", 31101);
    }

    if (rank == 0) {
        run_udp_server("127.0.0.1", 31101);
    } else if (rank == 1) {
        run_udp_client("127.0.0.1", 31101);
    }

    MPI_Finalize();
    return 0;
}
*/