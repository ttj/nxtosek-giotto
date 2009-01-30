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

#ifndef _E_SPEC_
#define _E_SPEC_

#include "spec.h"

#define OPCODE_nop       0
#define OPCODE_future    1
#define OPCODE_call      2
#define OPCODE_schedule  3
#define OPCODE_if        4
#define OPCODE_jump      5
#define OPCODE_return    6

typedef struct {
  int opcode;
  int arg1;
  int arg2;
  int arg3;
} instruction_type;

#define NOP()                                  {OPCODE_nop,       -1,        -1,           -1}
#define FUTURE(trigger,address,parameter)      {OPCODE_future,    trigger,   address,      parameter}
#define CALL(driver)                           {OPCODE_call,      driver,    -1,           -1}
#define SCHEDULE(task,annotation,parameter)    {OPCODE_schedule,  task,      annotation,   parameter}
#define IF(condition,then_address,else_address){OPCODE_if,        condition, then_address, else_address}
#define TERMINATE(task)                        {OPCODE_terminate, task,      -1,           -1}
#define JUMP(address)                          {OPCODE_jump,      address,   -1,           -1}
#define RETURN()                               {OPCODE_return,    -1,        -1,           -1}

#endif
