/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Will Miles  2017
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  Header for Network Time Protocol Extended Information Extension Field

  */

#ifndef GOT_NTP_EXTENDED_H
#define GOT_NTP_EXTENDED_H

#include "ntp.h"

#define NTP_EXTENSION_EXTENDED_INFORMATION         0x0009
#define NTP_EXTENSION_MAC_OPTIONAL                 0x2000
#define NTP_EXTENSION_EXTENDED_INFORMATION_VERSION 0x0100

#define NTP_EXTENSION_EXTENDED_INFORMATION_ID (NTP_EXTENSION_EXTENDED_INFORMATION | NTP_EXTENSION_MAC_OPTIONAL | NTP_EXTENSION_EXTENDED_INFORMATION_VERSION)
#define NTP_EXTENSION_EXTENDED_INFORMATION_ID_MAC (NTP_EXTENSION_EXTENDED_INFORMATION | NTP_EXTENSION_EXTENDED_INFORMATION_VERSION)

/* Function to add the Extended Information Version 1 extension field to an NTP_Packet */
/* Returns number of bytes written */
extern int NEX_GenerateExtension(uint8_t* ext, size_t ext_len, int interleaved, int tai);

/* Parses the TAI member out of an Extended Information Version 1, if present */
/* Otherwise returns 0 */
extern int8_t NEX_GetTAI(uint8_t* field, size_t len);

#endif
