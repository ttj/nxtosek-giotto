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

#ifndef _E_MACHINE_
#define _E_MACHINE_

#include <stdio.h>
#include <math.h>

#include "e_interface.h"

#include "f_spec.h"
#include "f_table.h"

#include "s_spec.h"
#include "s_table.h"

#include "e_spec.h"
#include "e_code.h"

#include "h_spec.h"
#include "h_table.h"

#include "n_spec.h"
#include "n_table.h"

typedef struct {
  trigger_type *trigger;

  unsigned address;

  int state;
  int parameter;
} trigger_binding_type;

typedef struct {
  annotation_type *annotation;

  int state;
  int parameter;
} annotation_binding_type;

extern task_type task_table[MAXTASK];                  /* f_table.c */
extern trigger_type trigger_table[MAXTRIGGER];         /* f_table.c */
extern driver_type driver_table[MAXDRIVER];            /* f_table.c */
extern condition_type condition_table[MAXCONDITION];   /* f_table.c */

extern annotation_type annotation_table[MAXCONDITION]; /* s_table.c */

extern instruction_type program[MAXPROGRAM];           /* e_code.c */

extern host_type host_table[MAXHOST];                  /* h_table.c */
extern network_type network_table[MAXNETWORK];         /* n_table.c */

#endif
