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

  Support for NTP Extension Field: Extended Information Version 1
  */

#include "config.h"

#include "ntp_extended.h"
#include "reference.h"
#include "util.h"

#define EXTENDED_INFORMATION_TAI_OFFSET_PRESENT 0x01
#define EXTENDED_INFORMATION_INTERLEAVE_MODE_PRESENT 0x02

#define EXTENDED_INFORMATION_INTERLEAVE_MODE 0x0100

/* ================================================== */

int
NEX_GenerateExtension(uint8_t* ext, size_t ext_len, int interleaved, int tai_offset)
{  
  uint16_t* base = (uint16_t*) ext;

  /* Validate sufficient space */
  if (ext_len < NTP_MIN_EXTENSION_LENGTH)
    return 0;
        
  /* Build extension data structure */
  base[0] = htons(NTP_EXTENSION_EXTENDED_INFORMATION_ID);
  base[1] = htons(NTP_MIN_EXTENSION_LENGTH);   // size of extension - only 8 bytes used, though
  base[2] = htons(EXTENDED_INFORMATION_INTERLEAVE_MODE_PRESENT | (tai_offset ? EXTENDED_INFORMATION_TAI_OFFSET_PRESENT : 0));
  base[3] = htons( (interleaved ? EXTENDED_INFORMATION_INTERLEAVE_MODE : 0) | tai_offset);
  memset(&base[4], 0, NTP_MIN_EXTENSION_LENGTH - 8);

  return NTP_MIN_EXTENSION_LENGTH;
}

/* ================================================== */

int8_t
NEX_GetTAI(uint8_t* ext, size_t len)
{
  uint16_t flags, value;

  /* Validate length */
  if (len < 4)
    return 0;

  uint16_t* base = (uint16_t*) ext;
  flags = htons(*base);
  value = htons(*(base + 1));

  if (flags & EXTENDED_INFORMATION_TAI_OFFSET_PRESENT)
  {
    return (int8_t) (value & 0xFF);
  }

  return 0;  
}
