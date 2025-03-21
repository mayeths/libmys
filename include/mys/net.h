/* 
 * Copyright (c) 2025 Haopeng Huang - All Rights Reserved
 * 
 * Licensed under the MIT License. You may use, distribute,
 * and modify this code under the terms of the MIT license.
 * You should have received a copy of the MIT license along
 * with this file. If not, see:
 * 
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include "_config.h"
#include "macro.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

/**
 * @brief Creates and binds a TCP server socket (IPv4 or IPv6) with socket option `SO_REUSEADDR`.
 *
 * @param bind_addr The address to bind to (IPv4 or IPv6).
 * @param bind_port The port to bind the server to.
 * 
 * @return int Returns the socket file descriptor on success, or -1 on error.
 *
 * @note
 * Binding to "0.0.0.0" (IPv4) or "::" (IPv6) allows the server to accept connections from any network interface on the system.
 * @note
 * Binding to "127.0.0.1" (IPv4) or "::1" (IPv6) limits the server to only accept connections from the localhost (loopback interface).
 */
MYS_PUBLIC int mys_tcp_server(const char *bind_addr, int bind_port);

/**
 * @brief Creates and binds a TCP server socket (IPv4 or IPv6) with a given socket option.
 *
 * @param bind_addr The address to bind to (IPv4 or IPv6).
 * @param bind_port The port to bind the server to.
 * @param sock_opt  The socket option to be set using `setsockopt()`.
 * 
 * @return int Returns the socket file descriptor on success, or -1 on error.
 *
 * @note
 * Binding to "0.0.0.0" (IPv4) or "::" (IPv6) allows the server to accept connections from any network interface on the system.
 * @note
 * Binding to "127.0.0.1" (IPv4) or "::1" (IPv6) limits the server to only accept connections from the localhost (loopback interface).
 */
MYS_PUBLIC int mys_tcp_server2(const char *bind_addr, int bind_port, int sock_opt);

/**
 * @brief Creates a TCP client socket and connects to the server (IPv4 or IPv6).
 *
 * This function creates a TCP socket and connects to the specified `server_addr`
 * and `server_port`.
 *
 * @param server_addr The server IP address (IPv4 or IPv6) to connect to.
 * @param server_port The server port to connect to.
 * 
 * @return int Returns the socket file descriptor on success, or -1 on error.
 */
MYS_PUBLIC int mys_tcp_client(const char *server_addr, int server_port);

/**
 * @brief Creates and binds a UDP server socket (IPv4 or IPv6).
 *
 * @param bind_addr The address to bind to (IPv4 or IPv6).
 * @param bind_port The port to bind the server to.
 * 
 * @return int Returns the socket file descriptor on success, or -1 on error.
 *
 * @note
 * Binding to "0.0.0.0" (IPv4) or "::" (IPv6) allows the server to accept datagrams from any network interface on the system.
 * @note
 * Binding to "127.0.0.1" (IPv4) or "::1" (IPv6) limits the server to only accept datagrams from the localhost (loopback interface).
 */
MYS_PUBLIC int mys_udp_server(const char *bind_addr, int bind_port);

/**
 * @brief Creates a UDP client socket and sends data to the server (IPv4 or IPv6).
 *
 * This function creates a UDP socket and prepares it for communication with a server
 * specified by `server_addr` and `server_port`.
 *
 * @param server_addr The server IP address (IPv4 or IPv6) to send data to.
 * @param server_port The server port to send data to.
 * 
 * @return int Returns the socket file descriptor on success, or -1 on error.
 */
MYS_PUBLIC int mys_udp_client(const char *server_addr, int server_port);
