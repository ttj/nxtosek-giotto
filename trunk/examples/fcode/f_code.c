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

#include "f_code.h"
#include <string.h>

void c_connect_sensor_to_return_key(c_bool *sensor) {
	//*sensor = os_key_event();
	*sensor = ecrobot_get_light_sensor(NXT_PORT_S1);

	os_print_message("sensor value (ret)");
	os_print_hex((int)*sensor);
}

void c_connect_sensor_to_random_generator(c_int *sensor) {
  unsigned i, text_message_length;

#if defined(OSEK) || defined(NXTOSEK)
  //*sensor = (*sensor + 1) % 10;
  *sensor = ecrobot_get_light_sensor(NXT_PORT_S1);
#else
  *sensor = 1 + (int)(MAXDISPLAY*(double)rand()/(RAND_MAX+1.0));
#endif

//  sprintf(text_message, "\t\tSensor reading:  ");

//  text_message_length = strlen(text_message);

//  for(i = text_message_length; i < *sensor + text_message_length; i++) {
//    text_message[i] = '*';
//	}

//  text_message[i] = 0;

//  os_print_message(text_message);
	os_print_message("sensor value (rand)");
	os_print_hex((int)*sensor);
}

void c_connect_actuator_to_display(c_string *string) {
	//  sprintf(text_message, "\t\tActuator update: %s", *string);

	//  os_print_message(text_message);
	os_print_message("actuator value");
}

void c_empty_string(c_string *string) {
	**string = 0;
}

void copy_c_string(c_string *string_source, c_string *string_dest) {
	char *s = (char *) *string_source, *d = (char *) string_dest;

	while (*s) {
		*d++ = *s++;
	}

	*d = 0;
}

void c_one(c_int *integer) {
	*integer=1;
}

void copy_c_int(c_int *integer_source, c_int *integer_dest) {
	*integer_dest=*integer_source;
}

void copy_c_bool(c_bool *bool_source, c_bool *bool_dest) {
	*bool_dest=*bool_source;
}

void c_control_task(c_int *average, c_string *display) {
	unsigned i;
	int dummy = 7;
	char *s=*display;

//	os_print_message("\t\tControl task begins");

	for(i=0;i<*average;i++) {
		*s++ = '-';
	}

	*s = 0;

	// To trigger deadline violation uncomment for loop
	//for(i=0;i<70000000;i++) {
	//	dummy = (dummy * dummy) % 100;
	//}

//	os_print_message("\t\tControl task ends");
}

void c_navigation_task(c_int *in, c_int *state, c_int *out) {
	unsigned i;
	int dummy = 7;

//	os_print_message("\t\tNavigation task begins");

	*out = (*in+*state)/2;

	// To trigger deadline violation uncomment for loop
	for(i=0;i<170000;i++) {
		dummy = (dummy * dummy) % 100;
	}

//	os_print_message("\t\tNavigation task ends");
}






void c_guard_task(c_bool intrusion, c_bool portIntrusion) {
	//check if an intruder is coming in
	os_print_message("guarding" );
	return;
}

void c_search_task(c_bool found, c_bool stateFound, c_bool portFound) {
	//find the intruder
	os_print_message("searching");
	return;
}

void c_get_light_sensor(c_int* val) {
	val=ecrobot_get_light_sensor(NXT_PORT_S1); //todo: make dynamic
	os_print_hex(val);
}

void c_get_sonar_sensor(c_int* val) {
	val=ecrobot_get_sonar_sensor(NXT_PORT_S4); //todo: make dynamic
	os_print_hex(val);
}


void c_set_motor_sonar_speed(c_int* speed) {
	nxt_motor_set_speed(NXT_PORT_A, speed, 1);
}



void c_zero(c_int *integer) {
	*integer=0;
}

unsigned c_false() {
	return 0;
}

unsigned c_true() {
	return 1;
}

void c_string_to_string(c_string *string_source, c_string *string_dest) {
	copy_c_string(string_source, string_dest);
}

unsigned c_key_pressed(c_bool *pressed) {
	return *pressed;
}

void c_switch_mode(c_int *integer) {
	os_print_message("\t\tSwitch mode!");

	c_one(integer);
}

void c_switch_undeclared_mode() {
	os_print_message("\t\tSwitch to dynamic mode!");
}

void c_int_to_int(c_int *integer_source, c_int *integer_dest) {
	copy_c_int(integer_source, integer_dest);
}

void c_bool_to_bool(c_int *bool_source, c_int *bool_dest) {
	copy_c_bool(bool_source, bool_dest);
}

unsigned c_ready_to_search(c_bool *intrusion) {
	return *intrusion;
}

unsigned c_ready_to_guard(c_bool *found) {
	return *found;
}

void giotto_timer_enable_code(e_machine_type e_machine_func, int relative_time) {
	return;
}

int giotto_timer_save_code(void) {
	return get_logical_time();
}

unsigned giotto_timer_trigger_code(int initial_time, int relative_time) {
	return (get_logical_time() == (initial_time + relative_time) % get_logical_time_overflow());
}
