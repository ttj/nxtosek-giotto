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

#include "h_interface.h"

#define MASTER_ID 0

#define SEND_SIZE 30

char send_buffer[SEND_SIZE];
unsigned send_size = SEND_SIZE;

os_socket_type message_socket;
os_socket_address_type host_message_addresses[MAXHOST];

#define MESSAGE_SIZE SEND_SIZE

char message_buffer[MESSAGE_SIZE];
unsigned message_size = MESSAGE_SIZE;

os_thread_type receiver_thread;

os_socket_type sync_socket;
unsigned sync_port = 49152;
os_socket_address_type host_sync_addresses[MAXHOST];

#define SYNC_SIZE 10

char sync_buffer[SYNC_SIZE];
unsigned sync_size = SYNC_SIZE;

os_thread_type sync_thread;

host_id_type my_host_id;

void send_sync_signal(unsigned logical_time) {
  int i;

  sprintf(sync_buffer, "%u", logical_time);

  for (i = 0; i < MAXHOST; i++)
    if (i != MASTER_ID)
      os_udp_send(&sync_socket, &host_sync_addresses[i], sizeof(host_sync_addresses[i]), sync_buffer, sync_size);
}

void wait_for_sync_message() {
  os_udp_receive(&sync_socket, sync_buffer, sync_size);

  printf("Sync buffer:    %sms\n", sync_buffer);
}

void send_to_host(host_id_type host_id, port_id_type port_id) {
  sprintf(send_buffer, "%u,%d", port_id, *((int *) port_table[port_id].port));

  os_udp_send(&message_socket, &host_message_addresses[host_id], sizeof(host_message_addresses[host_id]), send_buffer, SEND_SIZE);
}

void send_to_all_hosts(port_id_type port_id) {
  int i;

  printf("\t\tSending begins\n");

  sprintf(send_buffer, "%u", port_id);

  for (i = 0; i < port_table[port_id].size; i++)
    send_buffer[i+sizeof(port_id_type)] = ((char *) port_table[port_id].port)[i];

  //sprintf(send_buffer, "%u,%d", port_id, *((int *) port_table[port_id].port));
 
  for (i = 0; i < MAXHOST; i++)
    if (i != my_host_id)
      os_udp_send(&message_socket, &host_message_addresses[i], sizeof(host_message_addresses[i]), send_buffer, SEND_SIZE);

  printf("\t\tSending ends\n");
}

void receiver_code(void *args) {
  int i;

  port_id_type port_id;
  int port;

  for (;;) {
    os_udp_receive(&message_socket, message_buffer, message_size);

    printf("Message buffer: %s\n", message_buffer);

    //sscanf(message_buffer, "%u,%d", &port_id, &port);

    sscanf(message_buffer, "%u", &port_id);

    for (i = 0; i < port_table[port_id].size; i++)
      ((char *) port_table[port_id].port)[i] = message_buffer[i+sizeof(port_id_type)];

    //*((int *) port_table[port_id].port) = port;
  }
}

void sync_code(void *args) {
  for (;;) {
    wait_for_sync_message();

    os_thread_kill(timer_thread, SIGUSR1);
  }
}

void h_interface_init_master() {
  int i;

  os_udp_server_init(&sync_socket, sync_port);

  for (i = 0; i < MAXHOST; i++)
    if (i != MASTER_ID)
      os_wait_for_client(&sync_socket, &host_sync_addresses[i], sizeof(host_sync_addresses[i]), sync_buffer, sync_size);
}

void h_interface_init_slave() {
  os_udp_client_init(&sync_socket, &host_sync_addresses[MASTER_ID], host_table[MASTER_ID].ip, sync_port);

  os_udp_send(&sync_socket, &host_sync_addresses[MASTER_ID], sizeof(host_sync_addresses[MASTER_ID]), sync_buffer, sync_size);

  os_signal_handler_create();

  os_thread_create(&sync_thread, &sync_code, 0);
}

void h_interface_init(host_id_type host_id) {
  int i;

  my_host_id = host_id;

#ifdef DEBUG
  for (i = 0; i < MAXPORT; i++)
    if (port_table[i].size > MESSAGE_SIZE) {
      printf("Type of port %s is larger than message buffer, port size: %d, message buffer size: %d\n",
	     port_table[i].name,
	     port_table[i].size,
	     MESSAGE_SIZE);

      exit(1);
    } else if (port_table[i].size > SEND_SIZE) {
      printf("Type of port %s is larger than send buffer, port size: %d, send buffer size: %d\n",
	     port_table[i].name,
	     port_table[i].size,
	     SEND_SIZE);

      exit(1);
    }
#endif

  if (MAXHOST > 0) {
    if (my_host_id == 0)
      h_interface_init_master();
    else
      h_interface_init_slave();

    for (i = 0; i < MAXHOST; i++)
      if (i != my_host_id)
	os_udp_address_create(&host_message_addresses[i], host_table[i].ip, host_table[i].port);

    os_udp_server_init(&message_socket, host_table[my_host_id].port);

    os_thread_create(&receiver_thread, &receiver_code, 0);
  }
}

/* -------------------------------------------------------------------
 *
 * Signal interface
 *
 * ---------------------------------------------------------------- */

void signal_handler(int signo) {
}

int os_signal_handler_create() {
  os_signal_handler actions;

  bzero((char *) &actions, sizeof(actions));

  sigemptyset(&actions.sa_mask);

  actions.sa_flags = 0;
  actions.sa_handler = signal_handler;

  if (sigaction(SIGUSR1, &actions, NULL)) {
    printf("In os_signal_handler_create: error in call to sigaction\n");

#ifdef DEBUG
    printf("errno = %d, %s\n", errno, strerror(errno));
#endif
    
    return -1;
  }

  return 0;
}

int os_thread_kill(os_thread_type thread, int signal) {
  if (pthread_kill(thread, signal)) {
    printf("In os_thread_kill: error in call to pthread_kill\n");

#ifdef DEBUG
    printf("errno = %d, %s\n", errno, strerror(errno));
#endif
    
    return -1;
  }

  return 0;
}

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
 * ---------------------------------------------------------------- */

int os_udp_server_init(os_socket_type *server_socket,
		       unsigned server_port) {
  os_socket_address_type server_address;

  *server_socket = socket(AF_INET, SOCK_DGRAM, 0);

  if (-1 == *server_socket) {
    printf("In os_udp_server_init: error in call to socket\n");

#ifdef DEBUG
    printf("errno = %d, %s\n", errno, strerror(errno));
#endif

    return -1;
  } else {
    bzero((char *) &server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(server_port);

    if (-1 == bind(*server_socket,
		   (struct sockaddr *) &server_address,
		   sizeof(server_address))) {
      printf("In os_udp_server_init: error in call to bind\n");

#ifdef DEBUG
      printf("errno = %d, %s\n", errno, strerror(errno));
#endif

      return -1;
    }
  }

  return 0;
}

/* -------------------------------------------------------------------
 *
 * Function os_udp_client_init: initialize a client UDP socket.
 *
 * ---------------------------------------------------------------- */

int os_udp_client_init(os_socket_type *client_socket,
		       os_socket_address_type *server_address,
		       char *server_ip,
		       unsigned server_port) {
  os_socket_address_type client_address;

  os_udp_address_create(server_address, server_ip, server_port);

  if (-1 == (*client_socket = socket(AF_INET, SOCK_DGRAM, 0))) {
    printf("In os_udp_client_init: error in call to socket\n");

#ifdef DEBUG
    printf("errno = %d, %s\n", errno, strerror(errno));
#endif

    return -1;
  }

  bzero((char *) &client_address, sizeof(client_address));

  client_address.sin_family = AF_INET;
  client_address.sin_addr.s_addr = htonl(INADDR_ANY);
  client_address.sin_port = htons(0);

  if (-1 == bind(*client_socket,
		 (struct sockaddr *) &client_address,
		 sizeof(client_address))) {
    printf("In os_udp_client_init: error in call to bind\n");

#ifdef DEBUG
    printf("errno = %d, %s\n", errno, strerror(errno));
#endif
    
    return -1;
  }

  return 0;
}

/* -------------------------------------------------------------------
 *
 * Function os_udp_send: send data over a UDP socket.
 *
 * ---------------------------------------------------------------- */

int os_udp_send(os_socket_type *sender_socket,
		os_socket_address_type *receiver_address,
		int receiver_address_size,
		char *message_buffer,
		unsigned message_size) {
  if (-1 == sendto(*sender_socket,
		   (caddr_t) message_buffer,
		   message_size,
		   0,
		   (struct sockaddr *) receiver_address,
		   receiver_address_size)) {
    printf("In os_udp_send: error in call to sendto\n");

#ifdef DEBUG
    printf("errno = %d, %s\n", errno, strerror(errno));
#endif

    return -1;
  }

  return 0;
}

/* -------------------------------------------------------------------
 *
 * Function os_udp_receive: receive data over a UDP socket.
 *
 * ---------------------------------------------------------------- */

int os_udp_receive(os_socket_type *receiver_socket,
		   char *message_buffer,
		   unsigned message_size) {
  if (-1 == recvfrom(*receiver_socket,
		     message_buffer,
		     message_size,
		     0,
		     (struct sockaddr *) 0,
		     (int *) 0)) {
    printf("In os_udp_receive: error in call to recvfrom\n");

#ifdef DEBUG
    printf("errno = %d, %s\n", errno, strerror(errno));
#endif

    return -1;
  }

  return 0;
}

int os_udp_address_create(os_socket_address_type *address,
			  char *ip,
			  unsigned port) {
  bzero((char *) address, sizeof(address));

  address->sin_family = AF_INET;

  if (-1 == (address->sin_addr.s_addr = inet_addr(ip))) {
    printf("In os_udp_address_create: invalid host ip %s\n", ip);

#ifdef DEBUG
    printf("errno = %d, %s\n", errno, strerror(errno));
#endif

    return -1;
  }

  address->sin_port = htons(port);
}

int os_wait_for_client(os_socket_type *server_socket,
		       os_socket_address_type *client_address,
		       int client_address_size,
		       char *message_buffer,
		       unsigned message_size) {
  int local_client_address_size;

  local_client_address_size = client_address_size;

  if (-1 == recvfrom(*server_socket,
		     message_buffer,
		     message_size,
		     0,
		     (struct sockaddr *) client_address,
		     &local_client_address_size)) {
    printf("In os_echo: error in call to recvfrom\n");

#ifdef DEBUG
    printf("errno = %d, %s\n", errno, strerror(errno));
#endif

    return -1;
  }

  return 0;
}
