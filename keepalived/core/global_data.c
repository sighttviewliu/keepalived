/*
 * Soft:        Keepalived is a failover program for the LVS project
 *              <www.linuxvirtualserver.org>. It monitor & manipulate
 *              a loadbalanced server pool using multi-layer checks.
 *
 * Part:        Dynamic data structure definition.
 *
 * Version:     $Id: global_data.c,v 1.1.2 2003/09/08 01:18:41 acassen Exp $
 *
 * Author:      Alexandre Cassen, <acassen@linux-vs.org>
 *
 *              This program is distributed in the hope that it will be useful,
 *              but WITHOUT ANY WARRANTY; without even the implied warranty of
 *              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *              See the GNU General Public License for more details.
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Copyright (C) 2001, 2002, 2003 Alexandre Cassen, <acassen@linux-vs.org>
 */

#include <syslog.h>
#include <unistd.h>
#include <pwd.h>
#include "global_data.h"
#include "memory.h"
#include "list.h"
#include "utils.h"

/* External vars */
extern conf_data *data;

/* Default settings */
static void
set_default_lvs_id(conf_data* conf_data)
{
	char *new_id = NULL;
	int len = 0;

	new_id = get_local_name();
	if (!new_id || !new_id[0]) return;

	len = strlen(new_id);
	conf_data->lvs_id = MALLOC(len + 1);
	if (!conf_data->lvs_id) return;

	memcpy(conf_data->lvs_id, new_id, len);
}

static void
set_default_email_from(conf_data* conf_data)
{
	struct passwd *pwd = NULL;
	char* hostname = NULL;
	int len = 0;

	hostname = get_local_name();
	if (!hostname || !hostname[0]) return;

	pwd = getpwuid(getuid());
	if (!pwd) return;

	len = strlen(hostname) + strlen(pwd->pw_name) + 2;
	conf_data->email_from = MALLOC(len);
	if (!conf_data->email_from) return;
       
	snprintf(conf_data->email_from, len, "%s@%s", pwd->pw_name, hostname);
}

static void
set_default_smtp_server(conf_data* conf_data)
{
	conf_data->smtp_server = htonl(DEFAULT_SMTP_SERVER);
}

static void
set_default_smtp_connection_timeout(conf_data* conf_data)
{
	conf_data->smtp_connection_to = DEFAULT_SMTP_CONNECTION_TIMEOUT;
}

static void
set_default_values(conf_data* conf_data)
{
	/* No global data so don't default */
	if (!conf_data) return;
	set_default_lvs_id(conf_data);
	set_default_smtp_server(conf_data);
	set_default_smtp_connection_timeout(conf_data);
	set_default_email_from(conf_data);
}

/* email facility functions */
static void
free_email(void *data)
{
	FREE(data);
}
static void
dump_email(void *data)
{
	char *addr = data;
	syslog(LOG_INFO, " Email notification = %s", addr);
}

void
alloc_email(char *addr)
{
	int size = strlen(addr);
	char *new;

	new = (char *) MALLOC(size + 1);
	memcpy(new, addr, size);

	list_add(data->email, new);
}

/* data facility functions */
conf_data *
alloc_global_data(void)
{
	conf_data *new;

	new = (conf_data *) MALLOC(sizeof (conf_data));
	new->email = alloc_list(free_email, dump_email);

	set_default_values(new);
	return new;
}

void
free_global_data(conf_data *data)
{
	free_list(data->email);
	FREE_PTR(data->lvs_id);
	FREE_PTR(data->email_from);
	FREE(data);
}

void
dump_global_data(conf_data *data)
{
	if (!data)
		return;

	if (data->lvs_id ||
	    data->smtp_server ||
	    data->smtp_connection_to || data->email_from) {
		syslog(LOG_INFO, "------< Global definitions >------");
	}
	if (data->lvs_id)
		syslog(LOG_INFO, " LVS ID = %s", data->lvs_id);
	if (data->smtp_server)
		syslog(LOG_INFO, " Smtp server = %s",
		       inet_ntop2(data->smtp_server));
	if (data->smtp_connection_to)
		syslog(LOG_INFO, " Smtp server connection timeout = %d",
		       data->smtp_connection_to);
	if (data->email_from) {
		syslog(LOG_INFO, " Email notification from = %s",
		       data->email_from);
		dump_list(data->email);
	}
}