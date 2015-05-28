This project is a port of [Henzinger](http://mtc.epfl.ch/~tah/), Horowitz, Matic, [Kirsch](http://cs.uni-salzburg.at/~ck/), Sanvido, and Ghosal's Giotto E-machine implementation in C for OSEK and pthreads to [nxtOSEK](http://lejos-osek.sourceforge.net), an OSEK-compliant operating system for [Lego Mindstorms NXT](http://mindstorms.lego.com).  The only E-machine implementations for Giotto on Lego we could find appeared to be for the older Mindstorms RCX, so we decided to make this port.

You can [read more about Giotto at its home at Berkeley](http://embedded.eecs.berkeley.edu/giotto/).  The source code this port is based upon [is available on the Berkeley site as well](http://embedded.eecs.berkeley.edu/giotto/download.html), under the heading "E Machine implementation for C (pthreads and OSEK)".

To use this with the nxtOSEK operating system, place the code in a new directory in the nxtosek/samples directory, and compile using the normal "make all" which will generate the binary image called "emac.rxe". Note: if you do not place the code in a directory like nxtosek/samples/projectName/trunk/src, you will have to modify some paths in the makefile.

Please note [the copyright that Berkeley has on Giotto](http://embedded.eecs.berkeley.edu/giotto/copyright.html):

Copyright © 1990-2004 The Regents of the University of California. All rights reserved.

Permission is hereby granted, without written agreement and without license or royalty fees, to use, copy, modify, and distribute this software and its documentation for any purpose, provided that the above copyright notice and the following two paragraphs appear in all copies of this software.

IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.