bplist00�_WebMainResource�	
^WebResourceURL_WebResourceData_WebResourceMIMEType_WebResourceTextEncodingName_WebResourceFrameName_^https://raw.github.com/arduino/Arduino/ide-1.5.x/hardware/arduino/sam/cores/arduino/syscalls.hO\<html><head><style> .tweet .action-buffer-container i, .tweet.opened-tweet .action-buffer-container i, .tweet.opened-tweet.hover .action-buffer-container i  { background-position: -3px -3px !important; } .tweet .action-buffer-container i { background-position: -3px -21px !important; }</style></head><body><pre style="word-wrap: break-word; white-space: pre-wrap;">/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/**
  * \file syscalls.h
  *
  * Implementation of newlib syscall.
  *
  */

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
#include &lt;stdio.h&gt;
#include &lt;stdarg.h&gt;
#include &lt;sys/types.h&gt;
#include &lt;sys/stat.h&gt;

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

extern caddr_t _sbrk( int incr ) ;

extern int link( char *cOld, char *cNew ) ;

extern int _close( int file ) ;

extern int _fstat( int file, struct stat *st ) ;

extern int _isatty( int file ) ;

extern int _lseek( int file, int ptr, int dir ) ;

extern int _read(int file, char *ptr, int len) ;

extern int _write( int file, char *ptr, int len ) ;

#ifdef __cplusplus
}
#endif

</pre></body></html>Ztext/plainUUTF-8P    ( 7 I _ } � �	U	`	f                           	g