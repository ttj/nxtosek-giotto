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

#include "e_machine.h"

trigger_binding_type e_schedule[MAXSCHEDULE]; /* e machine trigger array */
unsigned n_enabled_triggers;                  /* number of enabled triggers */

annotation_binding_type e_ready[MAXTASK];

int arg1, arg2, arg3;

int pc;

unsigned mask;     /* Auxiliary mask */
unsigned ready;    /* tasks that are in the ready queue */
unsigned schedule; /* tasks to be scheduled */

unsigned i, j;

int priority, highest_priority;
int highest_priority_task;


#ifdef OSEK
double average_e_execution_time;
double average_d_execution_time;
double average_s_execution_time;

unsigned int e_execution_time,
  d_execution_time,
  s_execution_time,
  last_system_time,
  current_system_time;
#endif


/* -------------------------------------------------------------------
 *
 * Function e_machine: check every enabled trigger. For every activated
 * trigger interpret e code. If a task is scheduled post its semaphore.
 *
 * Arguments: trigger_number, not used.
 *
 * ---------------------------------------------------------------- */

#ifdef OSEK
TASK(e_machine_and_drivers)
#elif defined(PTHREADS)
void e_machine(int trigger_number)
#elif defined(NXTOSEK)
//todo
#endif
{
#ifdef OSEK
  getTickCountLow(&last_system_time);

  e_execution_time = 0;
  d_execution_time = 0;
  s_execution_time = 0;
#endif

  set_logical_time();

  ready = schedule;

  sprintf(text_message, "E machine time: %dms", get_logical_time());

  os_print_message(text_message);

  for(i = 0, mask = 1; i < MAXTASK; i++) {
    if (!is_task_finished(i))
      ready = ready | mask;

    mask = mask << 1;
  }

  while (1) {
    // Resetting i can be avoided if already evaluated triggers cannot
    // become active later in the same reaction, see ordered halt
    // points for Esterel compilation.
    i = 0;

    pc = -1;

    while ((pc == -1) && (i < n_enabled_triggers)) {
      if (e_schedule[i].trigger->is_active(e_schedule[i].state, e_schedule[i].parameter)) {

	pc = e_schedule[i].address;

	for(j = i; j < n_enabled_triggers-1; j++) {
	  e_schedule[j].trigger = e_schedule[j+1].trigger;
	  e_schedule[j].address = e_schedule[j+1].address;
	  e_schedule[j].state = e_schedule[j+1].state;
	  e_schedule[j].parameter = e_schedule[j+1].parameter;
	}

	n_enabled_triggers--;
      } else
	i++;
    }

    if (pc == -1)
      break;

    while (pc != -1) {
      arg1 = program[pc].arg1;
      arg2 = program[pc].arg2;
      arg3 = program[pc].arg3;

      switch(program[pc].opcode) {
      case OPCODE_nop:          /* nop() */
	pc++;

	break;
      case OPCODE_future:       /* future(Trigger,Address,Parameter) */
	j = n_enabled_triggers;

	e_schedule[j].trigger = &(trigger_table[arg1]);
	e_schedule[j].address = arg2;
	e_schedule[j].state = e_schedule[j].trigger->save();
	e_schedule[j].parameter = arg3;

#ifdef OSEK
	e_schedule[j].trigger->enable(e_machine_and_drivers, arg3);
#elif defined(PTHREADS)
	e_schedule[j].trigger->enable(e_machine, arg3);
#elif defined(NXTOSEK)
	//todo: add to schedule table
#endif

	n_enabled_triggers++;

	pc++;

	break;
      case OPCODE_call:         /* call(Driver) */
	if (driver_table[arg1].protected & ready) {
	  sprintf(text_message, "call(%d) exception (deadline violation)", arg1);

	  os_print_warning(text_message);
	} else {
#ifdef OSEK
	  getTickCountLow(&current_system_time);

	  e_execution_time = e_execution_time + current_system_time - last_system_time;
#endif

	  driver_table[arg1].call();

#ifdef OSEK
	  getTickCountLow(&last_system_time);

	  d_execution_time = d_execution_time + last_system_time - current_system_time;
#endif
	}

	pc++;

	break;
      case OPCODE_schedule:     /* schedule(Task,Annotation,Parameter) */
	mask = 1 << arg1;

	if (ready & mask) {
	  sprintf(text_message, "schedule(%d) too early (deadline violation)", arg1);

	  os_print_warning(text_message);
	} else {
	  schedule = schedule | mask;

	  e_ready[arg1].annotation = &(annotation_table[arg2]);
	  e_ready[arg1].state = e_ready[arg1].annotation->save();
	  e_ready[arg1].parameter = arg3;

	  // FIXME: 0 argument allows to plugin scheduling algorithm
	  e_ready[arg1].annotation->release(0, arg3);
	}

	pc++;

	break;
      case OPCODE_if:           /* if(Condition,Then_Address,Else_Address) */
	if (condition_table[arg1].protected & ready) {
	  sprintf(text_message, "if(%d, %d, %d) exception (deadline violation)", arg1, arg2, arg3);

	  os_print_warning(text_message);
	} else if (condition_table[arg1].is_true())
	  pc = arg2;
	else
	  pc = arg3;

	break;
      case OPCODE_jump:         /* jump(Address) */
	if(arg1 < 0) {
	  pc++;

	  os_print_message("undefined jump -> continue");
	} else
	  pc = arg1;

	break;
      case OPCODE_return:       /* return */
	pc = -1;

	break;
      default:
	os_print_error("Unknown opcode");

      } /* switch */
    } /* interpreter while */
  } /* fixed-point while */

#ifdef OSEK
  getTickCountLow(&current_system_time);

  e_execution_time = e_execution_time + current_system_time - last_system_time;
#endif

  highest_priority = -1;
  highest_priority_task = -1;

  for(i = 0, mask = 1; i < MAXTASK; i++) {
    if (schedule & mask) {
      if (e_ready[i].annotation->is_active(e_ready[i].state, e_ready[i].parameter)) {
	priority = e_ready[i].annotation->get_priority(e_ready[i].parameter);

	if ((highest_priority < 0) || (highest_priority > priority)) {
	  highest_priority = priority;

	  highest_priority_task = i;
	}
      }
    }

    mask = mask << 1;
  }

  if (highest_priority_task != -1) {
    schedule = schedule & ~(1 << highest_priority_task);

    schedule_task(highest_priority_task, highest_priority);
  }

#ifdef OSEK
  getTickCountLow(&last_system_time);

  s_execution_time = s_execution_time + last_system_time - current_system_time;

  if (d_execution_time > 0) {
    // Measure only if some drivers were called
    average_e_execution_time = 0.99*average_e_execution_time
      + 0.01*((double)e_execution_time);
    average_d_execution_time = 0.99*average_d_execution_time
      + 0.01*((double)d_execution_time);
    average_s_execution_time = 0.99*average_s_execution_time
      + 0.01*((double)s_execution_time);
  }

  if (get_logical_time() % 1000 == 0) {
    // Print only every second
    sprintf(text_message,
	    "E: %6.2f, D: %6.2f, S: %6.2f, E/E+D: %6.2f\%",
	    average_e_execution_time,
	    average_d_execution_time,
	    average_s_execution_time,
	    average_e_execution_time * 100.0 / (average_e_execution_time + average_d_execution_time));

    os_print_warning(text_message);
  }

  TerminateTask();

  os_print_error("e_machine_and_drivers: TerminateTask error");
#endif
}

#ifdef OSEK
TASK(e_machine_init)
#elif defined(PTHREADS)
main(int argc, char *argv[])
#elif defined(NXTOSEK)
//todo
#endif
{
  host_id_type host_id;
  int id;
  unsigned start_address;

  if (sizeof(int) < 4)
    os_print_error("E machine needs at least 32 bits for data type int");

#ifdef OSEK
  id = 0;

  average_e_execution_time = 0.0;
  average_d_execution_time = 0.0;
  average_s_execution_time = 0.0;
#elif defined(PTHREADS)
  if (argc > 2)
    os_print_error("Usage: e_machine [host_id]");
  else if (argc == 2) {
    if (sscanf(argv[1], "%d", &id) < 1) {
      sprintf(text_message, "Host id: %s, invalid format", argv[1]);

      os_print_error(text_message);
    }

    if ((id < 0) ||
	(MAXHOST == 0 && id > 0) ||
	((MAXHOST > 0) && (id > MAXHOST - 1))) {
      sprintf(text_message, "Host id: %d, out of range", id);

      os_print_error(text_message);
    }
  } else
    id = 0;
#elif defined(NXTOSEK)
//todo
#endif

  host_id = id;

  os_interface_init();

  s_interface_init();

  if (MAXHOST > 0)
    start_address = host_table[host_id].start_address;
  else
    start_address = 0;

  schedule = 0;

  e_schedule[0].trigger = &(trigger_table[0]);
  e_schedule[0].address = start_address;

#ifdef OSEK
  e_schedule[0].trigger->enable(e_machine_and_drivers, 0);
#elif defined(PTHREADS)
  e_schedule[0].trigger->enable(e_machine, 0);
#elif defined(NXTOSEK)
//todo
#endif

  e_schedule[0].state = e_schedule[0].trigger->save();
  e_schedule[0].parameter = 0;

  n_enabled_triggers = 1;

#ifdef DISTRIBUTED
  if (host_id == 0) {
    h_interface_init(host_id);

    e_interface_init(host_id);
  } else if (host_id > 0) {
    e_interface_init(host_id);

    h_interface_init(host_id);
  }
#else
  e_interface_init();
#endif

#ifdef OSEK
  TerminateTask();

  os_print_error("e_machine_init: TerminateTask error");
#elif defined(PTHREADS)
  while (n_enabled_triggers > 0) {
    e_machine_wait();

    e_machine(0);
  }

  exit(0);
#elif defined(NXTOSEK)
//todo
#endif
}
