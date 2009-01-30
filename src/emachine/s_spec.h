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

#ifndef _S_SPEC_
#define _S_SPEC_

#include "spec.h"

/* -------------------------------------------------------------------
 *
 * Annotation specification
 *
 * ---------------------------------------------------------------- */

typedef void (*s_algorithm_type) ();

typedef void (*release_code_type) (s_algorithm_type, int);
typedef int (*save_release_time_code_type) (void);
typedef unsigned (*annotation_code_type) (int, int);

typedef int (*priority_code_type) (int);

typedef struct {
  char name[MAXNAME];

  release_code_type release;
  save_release_time_code_type save;
  annotation_code_type is_active;

  priority_code_type get_priority;
} annotation_type;

#endif
