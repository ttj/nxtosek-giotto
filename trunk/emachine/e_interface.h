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

#ifndef _E_INTERFACE_
#define _E_INTERFACE_

#include <math.h> /* Do not remove, required for 'floor' function etc. */

#include "os_interface.h"

extern char text_message[];

#ifdef DISTRIBUTED
#include "h_interface.h"
#endif

#include "e_code.h"

#ifndef OSEK
typedef struct {
  unsigned timer_sec, timer_nsec;

  os_semaphore_type emachine;
  os_semaphore_type protect_logical_time;
} e_timer_type;
#endif

void set_logical_time();

unsigned get_logical_time();
unsigned get_logical_time_overflow();

void e_machine_wait();
void e_machine_go();

#ifdef DISTRIBUTED
void e_interface_init(host_id_type host_id);
#else
void e_interface_init();
#endif

extern instruction_type program[MAXPROGRAM]; /* e_code.c */ 

#endif












