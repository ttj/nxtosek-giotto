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

#include "os_interface.h"

#define TEXT_MESSAGE_SIZE 100

char text_message[TEXT_MESSAGE_SIZE];

#if defined(OSEK)
asm void os_getTickCountLow(volatile unsigned int *timeL)
{
%reg timeL
! "r12"
   mftb  r12,268
   stwu  r12,(timeL)
}

#elif defined(NXTOSEK)

#define LINES_PER_SCREEN 8  //Lego NXT can fit 8 lines of text on its screen at a time
volatile unsigned int last_line = 0; //last line that was displayed, go from 0 to LINES_PER_SCREEN

void os_getTickCountLow(volatile unsigned int *timeL) {
	//todo: get real interface to RTC for NXTOSEK
	*timeL = 0;
}
#endif

void getTickCountLow(volatile unsigned int *timeL) {
	os_getTickCountLow(timeL);
}

#ifdef __MINGW32__
static HANDLE hstdin = 0;
#endif

//used to connect sensors to key
unsigned os_key_event() {
#ifdef __MINGW32__
  DWORD        peekRecordsRead;
  INPUT_RECORD peek;

  if (hstdin == 0)
    if ((hstdin = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE)
      os_print_error("os_key_event: GetStdHandle failed");

  if (PeekConsoleInput(hstdin, &peek, 1, &peekRecordsRead) == 0)
    os_print_error("os_key_event: PeekConsoleInput failed");

  if (FlushConsoleInputBuffer(hstdin) == 0)
    os_print_error("os_key_event: FlushConsoleInputBuffer failed");

  if (peekRecordsRead && (peek.EventType & KEY_EVENT))
    return peekRecordsRead;
  else
    return 0;
#elif defined(OSEK)
  int n_unread_chars;
  char buffer;

  n_unread_chars = SerialRead(CHAN1, &buffer, 1);

  return n_unread_chars != 1;
#elif defined(PTHREADS)
  unsigned size = 0;
  char buffer;

  if (ioctl(0, FIONREAD, &size) != 0)
    os_print_error("os_key_event: ioctl error");

  if (size)
    read(0, &buffer, 1);

  return size;
#elif defined(NXTOSEK)
//todo: look up methods to interface to NXT buttons
	return 0;
#endif
}

void os_print_message(char *message) {
#ifdef OSEK
  SerialSend(CHAN1, message, strlen(message));

  SerialSend(CHAN1, "\r\n", 1);
#elif defined(PTHREADS)
  fprintf(stdout, "%s\n", message);

  fflush(stdout);
#elif defined(NXTOSEK)
  if (last_line >= LINES_PER_SCREEN) {
      display_clear(0);
      last_line=0;
  }
  display_goto_xy(0, last_line);
  display_string(message);
  display_update();
  last_line++;

#endif
}

void os_print_warning(char *message) {
#ifdef OSEK
  SerialSend(CHAN1, message, strlen(message));

  SerialSend(CHAN1, "\r\n", 1);
#elif defined(PTHREADS)
  fprintf(stderr, "%s\n", message);

  fflush(stderr);
#elif defined(NXTOSEK)
  if (last_line >= LINES_PER_SCREEN) {
      display_clear(0);
      last_line=0;
  }
  display_goto_xy(0, last_line);
  display_string(message);
  display_update();
  last_line++;
#endif
}

void os_print_error(char *message) {
#ifdef OSEK
	SerialSend(CHAN1, message, strlen(message));

	SerialSend(CHAN1, "\r\n", 1);

	ShutdownOS(0);
#elif defined(PTHREADS)
	fprintf(stderr, "%s, errno: %d: %s\n", message, errno, strerror(errno));

	fflush(stderr);

	exit(1);
#elif defined(NXTOSEK)
	if (last_line >= LINES_PER_SCREEN) {
		display_clear(0);
		last_line=0;
	}
	display_goto_xy(0, last_line);
	display_string(message);
	display_update();
	last_line++;
//	ShutdownOS(0); //todo: reenable later
#endif
}

#ifdef OSEK

void StartupHook() {
  Serial_Driver_Install(CHAN1, SIOCPOLLED, 9600);

  /* Start generating ticks */
  RtcInit(1000, SYSTEM_COUNTER);
}

#elif defined(PTHREADS)

void os_nanosleep(const os_time_type *req, os_time_type *rem) {
  //printf("Wait: req: %d,%ld; rem: %d,%ld\n", req->tv_sec, req->tv_nsec, rem->tv_sec, rem->tv_nsec);

#ifdef __MINGW32__

  Sleep(req->tv_sec * 1000.0 + req->tv_nsec / 1000000.0);

#else

  if (nanosleep(req, rem)) {
    if (errno == EINTR) {
      if ((rem->tv_sec > req->tv_sec) ||
	  ((rem->tv_sec == req->tv_sec) && (rem->tv_nsec >= req->tv_nsec))) {
	//printf("Time reversed: req: %d,%ld; rem: %d,%ld\n", req->tv_sec, req->tv_nsec, rem->tv_sec, rem->tv_nsec);
      } else if ((rem->tv_sec * 2 < req->tv_sec - 1) ||
	       ((rem->tv_sec * 2 < req->tv_sec + 1) &&
		(rem->tv_nsec < ((req->tv_sec - rem->tv_sec * 2) * 500000000 + (req->tv_nsec >> 1))))) {
	//printf("Sync speedup: req: %d,%ld; rem: %d,%ld\n", req->tv_sec, req->tv_nsec, rem->tv_sec, rem->tv_nsec);

	return;
      }

      //printf("Sync delayed: req: %d,%ld; rem: %d,%ld\n", req->tv_sec, req->tv_nsec, rem->tv_sec, rem->tv_nsec);

      rem->tv_sec = req->tv_sec;
      rem->tv_nsec = req->tv_nsec;

      while (nanosleep(rem, rem)) {
	if (errno != EINTR)
	  os_print_error("os_nanosleep: nanosleep error");
      }

      return;
    }

    os_print_error("os_nanosleep: nanosleep error");
  }
#endif
}

#ifdef DISTRIBUTED
void os_wait() {
  os_time_type wait_time;

  wait_time.tv_sec = 1;
  wait_time.tv_nsec = 0;

  while (1) {
    if (nanosleep(&wait_time, 0)) {
      if (errno == EINTR)
	return;

      os_print_error("os_wait: nanosleep error");
    }
  }
}
#endif

void os_semaphore_init(os_semaphore_type *semaphore) {
  if (sem_init(semaphore, 0, 0))
    os_print_error("os_semaphore_init: semaphore initialization error");
}

void os_semaphore_wait(os_semaphore_type *semaphore){
  sem_wait(semaphore);
}

void os_semaphore_post(os_semaphore_type *semaphore){
  if (sem_post(semaphore))
    os_print_error("os_semaphore_post: semaphore post error");
}

void os_semaphore_getvalue(os_semaphore_type *semaphore, unsigned *semaphore_value) {
  sem_getvalue(semaphore, semaphore_value);
}

void os_thread_create(os_thread_type *thread, void (*thread_code)(), void *args) {
  if (pthread_create(thread, NULL, (void *) thread_code, args))
    os_print_error("os_thread_create: thread create error");
}

void os_thread_set_priority(os_thread_type *thread, unsigned priority) {
  int policy, err;
  struct sched_param param;

  if (pthread_getschedparam(*thread, &policy, &param))
    os_print_error("os_thread_set_priority: get task priority error");

  param.sched_priority=priority;

  if (err=pthread_setschedparam(*thread, SCHED_RR, &param)) {
    if (EPERM==err)
      os_print_error("os_thread_set_priority: set task priority error: P");
    else if (EINVAL==err)
      os_print_error("os_thread_set_priority: set task priority error: V");
    else if (ESRCH==err)
      os_print_error("os_thread_set_priority: set task priority error: S");
    else if (EFAULT==err)
      os_print_error("os_thread_set_priority: set task priority error: F");
  }
}

#ifdef DYNAMIC
void os_pipe_create(char *pipe_name) {
  if (!mkfifo(pipe_name, 0777))
    os_print_error("os_pipe_create: create pipe error");
}
#endif

#elif defined(NXTOSEK)

void ecrobot_device_initialize()
{
	nxt_motor_set_speed(NXT_PORT_A, 0, 1);
	nxt_motor_set_speed(NXT_PORT_B, 0, 1);
	nxt_motor_set_speed(NXT_PORT_C, 1, 1);
	ecrobot_set_light_sensor_active(NXT_PORT_S1);
	ecrobot_init_sonar_sensor(NXT_PORT_S2);
	ecrobot_init_sonar_sensor(NXT_PORT_S3);
	ecrobot_set_light_sensor_active(NXT_PORT_S4);
}

void ecrobot_device_terminate()
{
  nxt_motor_set_speed(NXT_PORT_A, 0, 1);
  nxt_motor_set_speed(NXT_PORT_B, 0, 1);
  nxt_motor_set_speed(NXT_PORT_C, 0, 1);
  ecrobot_set_light_sensor_inactive(NXT_PORT_S1);
  ecrobot_term_sonar_sensor(NXT_PORT_S2);
  ecrobot_term_sonar_sensor(NXT_PORT_S3);
  ecrobot_set_light_sensor_inactive(NXT_PORT_S4);
}

//LEJOS OSEK hook to be invoked from an ISR in category 2
void user_1ms_isr_type2(void)
{
	StatusType ercd = E_OK;

//	ercd = SignalCounter(SysTimerCnt); // Increment OSEK Alarm Counter
//	if(ercd != E_OK)
//	{
//		os_print_error("stc_kill");
//		ShutdownOS(ercd);
//	}

	ercd = SignalCounter(SYSTEM_COUNTER);
	if(ercd != E_OK)
	{
		os_print_error("sc_kill");
		ShutdownOS(ercd);
	}
}

#endif /* ifdef OSEK else */

void os_interface_init() {
	return;
}
