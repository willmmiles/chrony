/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Will Miles, 2017-
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

  Routines implementing SOCK reference clock control socket.

  */


#include "config.h"

#include "sysincl.h"

#include "conf.h"
#include "refclock.h"
#include "logging.h"
#include "util.h"
#include "sched.h"

#include "refclock_sock_control.h"


/* Socket control message */
#define SOCK_MAGIC 0x5343544c

struct sock_message {
  /* Protocol identifier (0x5343544c) */
  int magic;
  /* Reference clook specification */
  char path[256];
  int poll;
  int filter_length;
  int pps_forced;
  int pps_rate;
  int min_samples;
  int max_samples;
  int sel_options;
  int max_lock_age;
  int stratum;
  int tai;
  uint32_t ref_id;
  uint32_t lock_ref_id;
  double offset;
  double delay;
  double precision;
  double max_dispersion;
  double pulse_width;
};

static int control_sockfd = -1;

static void
control_socket_read(int sockfd, int event, void *anything)
{
  struct sock_message message;
  RefclockParameters params;
  int s;
  union {
    struct sockaddr sa;
    char buf[512];
  } addr;
  socklen_t addr_len = sizeof(addr);

  s = recvfrom(sockfd, &message, sizeof (message), 0, &addr.sa, &addr_len);

  if (s < 0) {
    LOG(LOGS_ERR, "Could not read reference control message : %s",
        strerror(errno));
    return;
  }

  if (s != sizeof (message)) {
    LOG(LOGS_WARN, "Unexpected length of reference control message : %d != %ld",
        s, (long)sizeof (message));
    s = -1;
  }

  if (message.magic != SOCK_MAGIC) {
    LOG(LOGS_WARN, "Unexpected magic number in reference control message: %x != %x",
        message.magic, SOCK_MAGIC);
    s = -1;
  }

  if ((s > 0) && (message.path[0] != 0)) {
    /* Convert to a reference clock parameters struct, and add */
    params.driver_name = strdup("SOCK");
#ifdef QNX
    /* No strndup - ensure null terminated */
    message.path[255] = 0;
    params.driver_parameter = strdup(message.path);
#else
    params.driver_parameter = strndup(message.path,sizeof(message.path));
#endif
    params.driver_poll = 0;
    params.poll = message.poll;
    params.filter_length = message.filter_length;
    params.pps_forced = message.pps_forced;
    params.pps_rate = message.pps_rate;
    params.min_samples = message.min_samples;
    params.max_samples = message.max_samples;
    params.sel_options = message.sel_options;
    params.max_lock_age = message.max_lock_age;
    params.stratum = message.stratum;
    params.tai = message.tai;
    params.ref_id = message.ref_id;
    params.lock_ref_id = message.lock_ref_id;
    params.offset = message.offset;
    params.delay = message.delay;
    params.precision = message.precision;
    params.max_dispersion = message.max_dispersion;
    params.pulse_width = message.pulse_width;

    if (RCL_AddRefclock(&params, 1) != RCL_Success) {
      LOG(LOGS_WARN, "Failed to add client reference clock");
      s = -1;
    };
  } else {
    /* Empty path; try a removal */
    /* TODO: validate that the removed refclock is SOCK */
    if (RCL_RemoveRefclock(message.ref_id) != RCL_Success) {
      LOG(LOGS_WARN, "Failed to remove client reference clock");
      s = -1;
    }
  }

  /* Send a reply to the caller */
  /* This will be the size of the struct, or -1 on failure */
  sendto(sockfd, &s, sizeof(s), MSG_DONTWAIT, &addr.sa, addr_len);
}

void
RSC_Initialise(void)
{
  char *path = CNF_GetRefControlSocket();
  struct sockaddr_un s;

  if (path) {
    s.sun_family = AF_UNIX;
    if (snprintf(s.sun_path, sizeof (s.sun_path), "%s", path) >= sizeof (s.sun_path))
      LOG_FATAL("reference clock control socket path %s is too long", path);

    control_sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (control_sockfd < 0)
      LOG_FATAL("reference clock control socket() failed");

    UTI_FdSetCloexec(control_sockfd);

    unlink(path);
    if (bind(control_sockfd, (struct sockaddr *)&s, sizeof (s)) < 0)
      LOG_FATAL("reference clock control bind() failed");

    SCH_AddFileHandler(control_sockfd, SCH_FILE_INPUT, control_socket_read, NULL);
  }
}

void
RSC_Finalise(void)
{
  if (control_sockfd >= 0) {
    SCH_RemoveFileHandler(control_sockfd);
    close(control_sockfd);
  }
}
