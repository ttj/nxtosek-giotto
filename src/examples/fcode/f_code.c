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

static int count_guard = 0;
static int count_search = 0;

#if !defined(NXTOSEK)
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
#endif // !defined(NXTOSEK)

void c_empty_string(c_string *string) {
	**string = 0;
}

void c_one(c_int *integer) {
	*integer=1;
}

#if !defined(NXTOSEK)
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
//	for(i=0;i<170000;i++) {
//		dummy = (dummy * dummy) % 100;
//	}

//	os_print_message("\t\tNavigation task ends");
}

#endif // !defined(NXTOSEK)

void c_guard_task(c_bool *intrusion, c_bool *portIntrusion, c_int *light, c_int *portLight, c_bool *stateIntrusion) {
	count_guard++;

	//simple detection to check if an intruder is coming in
	if (*light < 0x1F0) {
		*intrusion = c_false();
		os_print_message("guarding" );
	}
	else {
		os_print_message("intrusion detected!");
		*intrusion = c_true();
		os_print_hex((int)*light);
		os_print_hex((int)*portLight);
	}

	c_bool_to_bool(intrusion, portIntrusion);

	os_print_hex(count_guard);
	os_print_hex((int)*intrusion);
	os_print_hex((int)*portIntrusion);

	return;
}

void c_search_task(c_bool *found, c_bool *stateFound, c_bool *portFound, c_int *sonar, c_int *portSonar) {
	count_search++;

	//find the intruder
	os_print_message("searching");
	os_print_hex(count_search);
	os_print_hex(*sonar);
//	os_print_hex((int)*found);
//	os_print_hex((int)*stateFound);
//	os_print_hex((int)*portFound);

	//sonar produces values 0x00 to 0xFF (0-255cm) which is distance from
	//object reflecting sound back in cm +/- 3cm error
	//given a square guard/search area of 2.5ft a side (hence sqrt(12.5)ft diagonal),
	//the range of distances is: 76.2cm to 107.8cm
	//adding the tolerance of +/- 3cm, we have the following logic:
	//    if *sonar < 73cm (0x49), then definitely we have an object found
	//    elif *sonar > 111cm (0x6F), then we definitely do not have an object
	//    elif *sonar >= 73cm && *sonar <= 111cm, then possibly we have an object found
	//    else error
	//
	//note that the < 73cm allows us to check in a RADIUS of 73cm around the search point,
	//which gives us coverage over the quarter of a circle traced by this radius within
	//the 73cm a side square
	//
	//so, we need a way to determine when the sonar value is between 73 and 111 if we
	//have found the object: this could be accomplished with multiple sensors

	//definitely found
	if (*sonar < 0x49) {
		*portFound = c_true();
	}
	//definitely not found
	else if (*sonar > 0x6F) {
		*portFound = c_false();
	}
	//possibly found, for now let's say it is
	else if (*sonar >= 0x49 && *sonar <= 0x6F) {
		*portFound = c_true(); //todo: needs to be fixed
	}
	//some error, assume not found
	else {
		*portFound = c_false();
	}

	//not done: repeatedly rotate 90 degrees cw then ccw to search area
	if (*portFound == c_false()) {
		if ((count_search % 2) == 0) { //todo: change to degrees counting (there is an encoder available)
			nxt_motor_set_speed(NXT_PORT_A, 100, 1); //todo: change to proper giotto actuator driver
		}
		else {
			nxt_motor_set_speed(NXT_PORT_A, -100, 1);
		}
	}

	//todo: once we have an angle calculation from the encoder, we can use this with the
	//      distance-to-target given by the sonar, so that we can approximate the area
	//      the intruder is in
	//equations:
	//x=distance*cos(angle)
	//y=distance*sin(angle)
	return;
}

void c_get_light_sensor(c_int* val) {
	*val=ecrobot_get_light_sensor(NXT_PORT_S1); //todo: make dynamic
	os_print_message("get_light_sensor");
	os_print_hex((int)*val);
}

void c_get_sonar_sensor(c_int* val) {
	*val=ecrobot_get_sonar_sensor(NXT_PORT_S4); //todo: make dynamic
	os_print_message("get_sonar_sensor");
	os_print_hex((int)*val);
}


void c_set_motor_sonar_speed(c_int* speed) {
	nxt_motor_set_speed(NXT_PORT_A, *speed, 1);
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

void copy_c_string(c_string *string_source, c_string *string_dest) {
	char *s = (char *) *string_source, *d = (char *) string_dest;

	while (*s) {
		*d++ = *s++;
	}

	*d = 0;
}

void copy_c_int(c_int *integer_source, c_int *integer_dest) {
	*integer_dest=*integer_source;
}

void c_int_to_int(c_int *integer_source, c_int *integer_dest) {
	copy_c_int(integer_source, integer_dest);
}

void copy_c_bool(c_bool *bool_source, c_bool *bool_dest) {
	*bool_dest=*bool_source;
}

void c_bool_to_bool(c_bool *bool_source, c_bool *bool_dest) {
	copy_c_bool(bool_source, bool_dest);
}

void c_bool_to_bool_and_int_to_int(c_bool *bool_source, c_bool *bool_dest, c_int *int_source, c_int *int_dest) {
	c_bool_to_bool(bool_source, bool_dest);
	c_int_to_int(int_source, int_dest);
}

unsigned c_ready_to_search(c_bool *intrusion) {
	os_print_message("rdy_search");
	os_print_hex(*intrusion);
	if (*intrusion == c_true()) {
		return c_true();
	}
	else {
		return c_false();
	}
}

unsigned c_ready_to_guard(c_bool *found) {
	os_print_message("rdy_guard");
	os_print_hex(*found);
	if (*found == c_true()) {
		return c_true();
	}
	else {
		return c_false();
	}
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
