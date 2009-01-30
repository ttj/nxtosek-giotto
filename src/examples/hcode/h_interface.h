/*

 Copyright (c) 2001 The Regents of the University of California.
 All rights reserved.
 Permission is hereby granted, without written agreement and without
 license or royalty fees, to use, copy, modify, and distribute this
 software and its documentation for any purpose, provided that the above
 copyright notice and the following two paragraphs appear in all copies
 of this software.

 IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
 THE UNIVERSITY OF CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 SUCH DAMAGE.

 THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
 PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 CALIFORNIA HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 ENHANCEMENTS, OR MODIFICATIONS.

*/

/*

  Author: Christoph Kirsch, cm@eecs.berkeley.edu

*/

#ifndef _H_INTERFACE_
#define _H_INTERFACE_

#include "h_table.h"

#include "f_spec.h"

#include "os_interface.h"

void send_sync_signal(unsigned logical_time);
void wait_for_sync_signal();
void h_interface_init_master();
void h_interface_init_slave();
void send_to_host(host_id_type host_id, port_id_type port_id);
void send_to_all_hosts(port_id_type port_id);
void h_interface_init();

#include "f_table.h"

extern host_type host_table[];
extern port_type port_table[];

extern os_thread_type timer_thread;

#include <signal.h>

typedef struct sigaction os_signal_handler;

#include <errno.h>

#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

/* -------------------------------------------------------------------
 *
 * Type os_socket_type: a socket.
 *
 * ---------------------------------------------------------------- */

typedef int os_socket_type;

/* -------------------------------------------------------------------
 *
 * Type os_socket_address_type: a socket address.
 *
 * ---------------------------------------------------------------- */

typedef struct sockaddr_in os_socket_address_type;

/* -------------------------------------------------------------------
 *
 * Socket interface
 *
 * ---------------------------------------------------------------- */

/* -------------------------------------------------------------------
 *
 * UDP
 *
 * ---------------------------------------------------------------- */

/* -------------------------------------------------------------------
 *
 * Function os_udp_server_init: initialize a server UDP socket.
 *
 * Arguments: server_socket, a pointer to an uninitialized socket;
 * port, the port of the server.
 *
 * Returns: 0 if successful, -1 if unsuccessful.
 *
 * ---------------------------------------------------------------- */

int os_udp_server_init(os_socket_type *server_socket,
		       unsigned server_port);

/* -------------------------------------------------------------------
 *
 * Function os_udp_client_init: initialize a client UDP socket.
 *
 * Arguments: client_socket, a pointer to an uninitialized socket;
 * address, initialized for future send socket calls; ip, the IP
 * address of the server; port, the port of the server.
 *
 * Returns: 0 if successful, -1 if unsuccessful.
 *
 * ---------------------------------------------------------------- */

int os_udp_client_init(os_socket_type *client_socket,
		       os_socket_address_type *server_address,
		       char *server_ip,
		       unsigned server_port);

/* -------------------------------------------------------------------
 *
 * Function os_udp_send: send data over a UDP socket.
 *
 * Arguments: sender_socket, a pointer to an initialized UDP socket;
 * data, the data to send; size, the number of bytes of the data to
 * send.
 *
 * Returns: 0 if successful, -1 if unsuccessful.
 *
 * ---------------------------------------------------------------- */

int os_udp_send(os_socket_type *sender_socket,
		os_socket_address_type *receiver_address,
		int receiver_address_size,
		char *message_buffer,
		unsigned message_size);

/* -------------------------------------------------------------------
 *
 * Function os_udp_receive: receive data over a UDP socket.
 *
 * Arguments: receiver_socket, a pointer to an initialized UDP socket;
 * data, a buffer into which the received data gets put; size, the
 * maximum number of bytes to receive.
 *
 * Returns: 0 if successful, -1 if unsuccessful.
 *
 * ---------------------------------------------------------------- */

int os_udp_receive(os_socket_type *receiver_socket,
		   char *message_buffer,
		   unsigned message_size);

int os_udp_address_create(os_socket_address_type *address,
			  char *ip,
			  unsigned port);

#endif
