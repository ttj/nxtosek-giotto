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

#include "s_interface.h"

#ifdef OSEK

void schedule_task(unsigned task_id, int priority) {
  if (ActivateTask((int) task_table[task_id].schedule) != E_OK)
    os_print_error("schedule_task: ActivateTask error");
}

unsigned is_task_finished(unsigned task_id) {
  // FIXME: enable runtime exceptions

  return 1;
}

void s_interface_init() {
}

#else

os_thread_type task_threads[MAXTASK];       /* task thread array */
os_semaphore_type task_semaphores[MAXTASK]; /* task semaphore array */

unsigned task_ids[MAXTASK];

unsigned task_semaphore_value;

void task_code(void *args) {
  while(1) {
    os_semaphore_wait(&(task_semaphores[*((unsigned *) args)]));

    task_table[*((unsigned *) args)].schedule();

    os_semaphore_wait(&(task_semaphores[*((unsigned *) args)]));

    e_machine_go();
  }
}

void schedule_task(unsigned task_id, int priority) {
  /* post task semaphore twice in order to have semaphore value of 1
     during the task execution */

  // FIXME: Set priority

  os_semaphore_post(&(task_semaphores[task_id]));
  os_semaphore_post(&(task_semaphores[task_id]));
}

unsigned is_task_finished(unsigned task_id) {
  os_semaphore_getvalue(&(task_semaphores[task_id]), &task_semaphore_value);

  /* an unscheduled task has a semaphore value of 0 */
  return (task_semaphore_value == 0);
}

void s_interface_init() {
  unsigned i;

  for (i = 0; i < MAXTASK; i++) {
    task_ids[i] = i;

    os_semaphore_init(&(task_semaphores[i]));

    os_thread_create(&(task_threads[i]), &task_code, &(task_ids[i]));
  }
}
#endif
