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
          Slobodan Matic, matic@eecs.berkeley.edu

*/

#ifndef _OS_INTERFACE_
#define _OS_INTERFACE_

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>

#ifdef OSEK
#include "oil.h"
#include "bsp.h"
#else
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#ifndef __MINGW32__
#include <sys/ioctl.h>
#endif

unsigned os_key_event();

void os_print_message(char *message);
void os_print_warning(char *message);
void os_print_error(char *message);

#ifndef OSEK
typedef struct timespec os_time_type;

void os_nanosleep(const os_time_type *req, os_time_type *rem);

/* -------------------------------------------------------------------
 *
 * Semaphore interface
 *
 * ---------------------------------------------------------------- */

typedef sem_t os_semaphore_type;

/* -------------------------------------------------------------------
 *
 * Function os_semaphore_init: initialize a semaphore with 0 value.
 *
 * Arguments: semaphore, a pointer to an unitialized semaphore;
 *
 * ---------------------------------------------------------------- */

void os_semaphore_init(os_semaphore_type *semaphore);

/* -------------------------------------------------------------------
 *
 * Function os_semaphore_wait: wait on the semaphore (block until its value
 * is greater than 0).  Decrement the semaphore's value by 1.
 *
 * Arguments: semaphore, a pointer to an initialized semaphore.
 *
 * ---------------------------------------------------------------- */

void os_semaphore_wait(os_semaphore_type *semaphore);

/* -------------------------------------------------------------------
 *
 * Function os_semaphore_post: increment the value of a semaphore by 1.
 *
 * Arguments: semaphore, a pointer to an initialized semaphore.
 *
 * ---------------------------------------------------------------- */

void os_semaphore_post(os_semaphore_type *semaphore);

void os_semaphore_getvalue(os_semaphore_type *semaphore, unsigned *semaphore_value);

/* -------------------------------------------------------------------
 *
 * Thread interface
 *
 * ---------------------------------------------------------------- */

typedef pthread_t os_thread_type;

/* -------------------------------------------------------------------
 *
 * Function os_thread_create: create a thread.
 *
 * Arguments:
 * thread
 *
 * ---------------------------------------------------------------- */

void os_thread_create(os_thread_type *thread, void (*thread_code)(), void *args);

/* -------------------------------------------------------------------
 *
 * Function os_thread_set_priority: set the priority of a thread.
 *
 * Arguments:
 * thread
 * priority, the new priority of the thread.
 *
 * Threads with higher priorities preempt threads with lower priorities.
 * This follows POSIX, but not VxWorks.
 *
 * ---------------------------------------------------------------- */

void os_thread_set_priority(os_thread_type *thread, unsigned priority);

/* -------------------------------------------------------------------
 *
 * Function os_pipe_create: create a pipe. permissions are set to 0777.
 *
 * Arguments: 
 * pipe_name
 *
 * ---------------------------------------------------------------- */

void os_pipe_create(char *pipe_name);

#endif /* ifndef OSEK */

#endif
