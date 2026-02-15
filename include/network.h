/**
 * @file network.h
 *
 * @brief Declaration of network socket setup and connection functions
 */

#ifndef NETWORK_H
#define NETWORK_H


/* Public interface */
/**
 * @brief Return file descriptor connected to a socket server for writing
 *
 * @param hostname IP address/name of host
 * @param port     Port number to connect to on host
 *
 * @return File descriptor of a stream socket connected to a server
 *
 * @note If a connection cannot be established, the process is terminated.
 */
int nwk_socket_file_descriptor(char *hostname, char *port);

/**
 * @brief Return file descriptor of a listening socket file descriptor
 *
 * @param port Port number to listen on
 *
 * @return File descriptor of a stream socket for listening
 *
 * @note If a connection cannot be established, the process is terminated.
 */
int nwk_listener_socket_file_descriptor(char *port);


#endif  /* ! NETWORK_H */
