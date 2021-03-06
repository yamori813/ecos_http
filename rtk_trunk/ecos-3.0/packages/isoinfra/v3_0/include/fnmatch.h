#ifndef CYGONCE_ISO_FNMATCH_H
#define CYGONCE_ISO_FNMATCH_H
/*========================================================================
//
//      fnmatch.h
//
//      fnmatch()
//
//========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2007 Free Software Foundation, Inc.                        
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     Peter Korsgaard <peter.korsgaard@barco.com>
// Contributors:
// Date:          2007-01-24
// Purpose:       This file provides the fnmatch() function
//                required by POSIX 1003.2-1992, section B.6.
// Description:   The real contents of this file get set from the
//                configuration (set by the implementation)
// Usage:         #include <fnmatch.h>
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/isoinfra.h>          /* Configuration header */

/* INCLUDES */

#ifdef CYGBLD_ISO_FNMATCH_HEADER
# include CYGBLD_ISO_FNMATCH_HEADER
#endif

#endif /* CYGONCE_ISO_FNMATCH_H multiple inclusion protection */

/* EOF fnmatch.h */
