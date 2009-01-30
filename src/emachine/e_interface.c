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

#include "e_interface.h"

#ifdef PTHREADS
e_timer_type timer;                                     /* e machine periodic timer */

os_thread_type timer_thread;                            /* e machine timer thread */

unsigned unprotected_logical_time = 0;                  /* unprotected logical time counter */

unsigned first_protect_logical_time_value;
unsigned second_protect_logical_time_value;
#endif

unsigned global_logical_time = 0;                       /* global logical time counter */
unsigned timer_logical_time = 0;                        /* logical time counter of timer */
unsigned logical_time_overflow = MSEC_PER_UNIT*1000;    /* forced logical time counter overflow */

#ifdef DYNAMIC
int mpu = MSEC_PER_UNIT;

struct mode_entry {
  char mode_name[MAXNAME];
  int fix_up;
} symbol_table[32], aux_table[32];

os_thread_type loader_thread;

void loader_code(void *args);
#endif

#ifdef DISTRIBUTED
host_id_type my_host_id;
#endif

#if defined(OSEK) || defined(NXTOSEK)
TASK(timer_code) {
	// FIXME: unprotected increase of timer_logical_time

	timer_logical_time = (timer_logical_time + MSEC_PER_UNIT) % logical_time_overflow;

	e_machine_go();
}

void set_logical_time() {
	// FIXME: unprotected access of timer_logical_time

	global_logical_time = timer_logical_time;
}

unsigned get_logical_time() {
	return global_logical_time;
}

unsigned get_logical_time_overflow() {
	return logical_time_overflow;
}

void e_machine_go() {
	ChainTask(e_machine_and_drivers);
	return;
}

void e_interface_init() {
	timer_logical_time = logical_time_overflow - MSEC_PER_UNIT;
	global_logical_time = timer_logical_time;

	switch (SetRelAlarm(TimerAlarm, 1000, MSEC_PER_UNIT))
	{
		//No error
		case E_OK: {
			os_print_message("eit: ok");
			break;
		}
		//Alarm <AlarmID> is already in use
		case E_OS_STATE: {
			os_print_error("eit: state");
			break;
		}
		//Alarm <AlarmID> is invalid
		case E_OS_ID: {
			os_print_error("eit: id");
			break;
		}
		//Value of <increment> outside of the admissible limits (lower
		//than zero or greater than maxallowedvalue)
		//	or
		//Value of <cycle> unequal to 0 and outside of the admissible
		//counter limits (less than mincycle or greater than
		//maxallowedvalue)
		case E_OS_VALUE: {
			os_print_error("eit: val");
			break;
		}
		default: {
			os_print_warning("eit: other");
			break;
		}
	}

//	if (SetRelAlarm(TimerAlarm, 1000, MSEC_PER_UNIT) != E_OK)
//	{
//		os_print_error("eit: SetRelAlarm error");
//	}
//#if defined(DEBUG_OSEK)
//	else {
//		os_print_message("eit: rel TimerAlarm");
//	}
//#endif
}

#elif defined(PTHREADS)

/* -------------------------------------------------------------------
 *
 * Function timer_code: periodic trigger thread of control.  Wait for
 * the time specified in (trig_sec, trig_nsec) and then activate the
 * embedded machine code by posting to its semaphore.
 *
 * ---------------------------------------------------------------- */

void timer_code(void *args) {
  os_time_type req_desc, rem_desc;
  e_timer_type *timer_timer;
  unsigned timer_protect_logical_time_value;

  timer_timer = (e_timer_type *) args;

  req_desc.tv_sec = timer_timer->timer_sec;
  req_desc.tv_nsec = timer_timer->timer_nsec;

#ifdef DISTRIBUTED
  if (my_host_id != 0)
    os_wait();
#endif

  while (1) {
    sprintf(text_message, "Logical time:   %dms", timer_logical_time);

    os_print_message(text_message);

#ifdef DISTRIBUTED
    if (my_host_id == 0)
      send_sync_signal(timer_logical_time);
#endif

    e_machine_go();

    //req_desc.tv_sec = floor(mpu / 1000);
    //req_desc.tv_nsec = 1000000 * (mpu - (req_desc.tv_sec * 1000));

    os_nanosleep(&req_desc, &rem_desc);

    // Non-blocking write protocol for timer_logical_time

    os_semaphore_getvalue(&(timer_timer->protect_logical_time),
			  &timer_protect_logical_time_value);

    // 100 is an arbitrary upper bound
    if (timer_protect_logical_time_value > 100)
      os_semaphore_init(&(timer_timer->protect_logical_time));

    os_semaphore_post(&(timer_timer->protect_logical_time));

    timer_logical_time = (timer_logical_time + MSEC_PER_UNIT) % logical_time_overflow;

    os_semaphore_post(&(timer_timer->protect_logical_time));
  }
}

void set_logical_time() {
  //int i,dummy;

  // Non-blocking write protocol for timer_logical_time

  os_semaphore_getvalue(&(timer.protect_logical_time),
			&first_protect_logical_time_value);

  unprotected_logical_time = timer_logical_time;

  /* Uncomment for triggering re-read through non-blocking write protocol. */
  //for(i=0;i<3000000;i++)
  //  dummy = (dummy * dummy) % 100;

  os_semaphore_getvalue(&(timer.protect_logical_time),
			&second_protect_logical_time_value);

  if (first_protect_logical_time_value == second_protect_logical_time_value)
    global_logical_time = unprotected_logical_time;
  else
    os_print_message("Missed logical time! +++++++++++++++++++++++++++++++++++++++++++++++");
}

unsigned get_logical_time() {
  return global_logical_time;
}

unsigned get_logical_time_overflow() {
  return logical_time_overflow;
}

void e_machine_wait() {
  os_semaphore_wait(&(timer.emachine));
}

void e_machine_go() {
  os_semaphore_post(&(timer.emachine));
}

#ifdef DISTRIBUTED
void e_interface_init(host_id_type host_id) {
  my_host_id = host_id;
#else
void e_interface_init() {
#endif
  timer_logical_time = 0;
  global_logical_time = timer_logical_time;

  timer.timer_sec = floor(MSEC_PER_UNIT / 1000);
  timer.timer_nsec = 1000000 * (MSEC_PER_UNIT - (timer.timer_sec * 1000));

  os_semaphore_init(&(timer.emachine));
  os_semaphore_init(&(timer.protect_logical_time));

#ifdef DYNAMIC
  os_thread_create(&loader_thread, &loader_code, NULL);
#else
  os_thread_create(&timer_thread, &timer_code, &timer);
#endif
}
#endif

#ifdef DYNAMIC

void inc_address(unsigned new_start_add, unsigned n_instr) {
  unsigned i;

  for(i = new_start_add;i < new_start_add + n_instr;i++) {
    switch(program[i].opcode) {
      case OPCODE_future:  /* {OPCODE_future, trigger, address, parameter} */
	program[i].arg2 += new_start_add;
	break;
      case OPCODE_if:  /* {OPCODE_if, condition, then_address, else_address} */
	program[i].arg2 += new_start_add;
	program[i].arg3 += new_start_add;
	break;
      case OPCODE_jump:  /* {OPCODE_jump, address, -1, -1} */
	if(program[i].arg1 < 0) {
	  if(program[i].arg1 != -1)
	    program[i].arg1 -= new_start_add;
        }
	else
	  program[i].arg1 += new_start_add;
	break;
    }
  }
}

void fix_up(int fix_up, unsigned address) {
  unsigned current = -fix_up - 2;
  int up;

  while((up = program[current].arg1) < 0) {  /* {OPCODE_jump, address, -1, -1} */
    program[current].arg1 = address;
    if(up != -1)
      current = -up - 2;
  }
}

void connect_fix_up(int fix_up, int prev_fix_up) {
  unsigned current = -fix_up - 2;

  while(program[current].arg1 != -1)  /* {OPCODE_jump, address, -1, -1} */
    current = -program[current].arg1 - 2;
  program[current].arg1 = prev_fix_up;
}

unsigned strcomp(char *st1, char *st2) {
  while(*st1==*st2 && *st1) {
    st1++;
    st2++;
  }
  return(*st1==*st2);
}

void strcopy(char *st1, char *st2) {
  while(*st2) {
    *st1=*st2;
    st1++;
    st2++;
  }
}

int find_mode(unsigned n_mode, char *mode_name) {
  unsigned i;

  for(i=0;i<n_mode;i++)
    if(strcomp(mode_name, symbol_table[i].mode_name))
      return i;
  return -1;
}

void loader_code(void *args) {
  int fd, i, j, k, l, newmpu, value;
  unsigned n_mode = 0, mode_index, start = 1, single, offset;

  offset = 0;  /* offset = start e_code address for new mode */

  n_mode = 0;  /* n_mode = number of uploaded modes */
  start = 1;   /* start = 0 after the uploading of the first mode */

  os_pipe_create("loaderpipe");

  while (1) {

    fd=open("loaderpipe", O_RDONLY);

    read(fd, &j, sizeof(int));  /* j = number of entries in the symbol table of the new mode */

    sprintf(text_message, "Received: %d symbols", j);
    os_print_message(text_message);

    k = 0;

    for(i=0;i<j;i++) {
      read(fd, aux_table[i].mode_name, 32);
      read(fd, &aux_table[i].fix_up, sizeof(int));
      if(aux_table[i].fix_up >= 0) k++;
    }   /* k = number of modes with associated code */

    read(fd, &l, sizeof(int));  /* l = number of instructions in the new mode */

    sprintf(text_message, "Received: %d instructions", l);
    os_print_message(text_message);

    read(fd, &newmpu, sizeof(int));  /* mpu = number of msec per unit in the new mode */

    sprintf(text_message, "Received: %d msec per unit", newmpu);
    os_print_message(text_message);

    if(newmpu < mpu) mpu = newmpu;

    for(i=0;i<l;i++) {
      read(fd, &program[offset+i].opcode, sizeof(int));
      read(fd, &program[offset+i].arg1, sizeof(int));
      read(fd, &program[offset+i].arg2, sizeof(int));
      read(fd, &program[offset+i].arg3, sizeof(int));
    }

    close(fd);

    single = 1;

    for(i=0;i<k && single;i++) {
      if((mode_index = find_mode(n_mode, aux_table[i].mode_name)) != -1) {
	if(symbol_table[mode_index].fix_up >= 0) {
	  /* new mode is already uploaded (isn't new) */
	  sprintf(text_message, "duplicate e-code for mode %s", aux_table[i].mode_name);
	  os_print_message(text_message);

	  single = 0;
	}
      }
    }

    if(!single) continue;  /* -> wait for new uploading without changing offset and n_mode */

    for(i=0;i<k;i++) {
      if((mode_index = find_mode(n_mode, aux_table[i].mode_name)) == -1) {
	/* new mode is not referenced in previously uploaded modes */
	mode_index = n_mode++;
	strcopy(symbol_table[mode_index].mode_name, aux_table[i].mode_name);
      }
      else {
	/* new mode is already referenced */
	value = symbol_table[mode_index].fix_up;
	fix_up(value, offset + aux_table[i].fix_up);
      }
      symbol_table[mode_index].fix_up = offset + aux_table[i].fix_up;
    }

    inc_address(offset, l);  /* increment adresses for future, if and jump instructions in the new mode */

    for(i=k;i<j;i++) {
      if((mode_index = find_mode(n_mode, aux_table[i].mode_name)) == -1) {
	/* mode referenced in a new mode is neither referenced in previous modes nor is uploaded */
	mode_index = n_mode++;
	strcopy(symbol_table[mode_index].mode_name, aux_table[i].mode_name);
	symbol_table[mode_index].fix_up = -offset + aux_table[i].fix_up;
      }
      else {
	/* mode referenced in a new mode is already referenced or uploaded */
	value = symbol_table[mode_index].fix_up;
	if(value < 0) {
	  /* mode referenced in a new mode is already referenced (it's not uploaded) */
	  connect_fix_up(-offset + aux_table[i].fix_up, symbol_table[mode_index].fix_up);
	  symbol_table[mode_index].fix_up = -offset + aux_table[i].fix_up;
	}
	else
	  /* mode referenced in a new mode is already uploaded */
	  fix_up(-offset + aux_table[i].fix_up, value);
      }
    }
    offset += l;  /* increment pointer to free e_code space */

    if(start) {   /* start E machine after uploading of the first mode */
      os_thread_create(&timer_thread, &timer_code, &timer);

      start = 0;
    }
  }
}

#endif /* DYNAMIC */
