/*
 * This file is part of libtrace
 *
 * Copyright (c) 2004 The University of Waikato, Hamilton, New Zealand.
 * Authors: Daniel Lawson 
 *          Perry Lorier 
 *          
 * All rights reserved.
 *
 * This code has been developed by the University of Waikato WAND 
 * research group. For further information please see http://www.wand.net.nz/
 *
 * libtrace is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * libtrace is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libtrace; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: test-pcap-to-erf.c,v 1.3 2006/02/27 03:41:12 perry Exp $
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include "dagformat.h"
#include "libtrace.h"

void iferr(libtrace_t *trace)
{
	libtrace_err_t err = trace_get_err(trace);
	if (err.err_num==0)
		return;
	printf("Error: %s\n",err.problem);
	exit(1);
}

void iferrout(libtrace_out_t *trace)
{
	libtrace_err_t err = trace_get_err_output(trace);
	if (err.err_num==0)
		return;
	printf("Error: %s\n",err.problem);
	exit(1);
}

char *lookup_uri(const char *type) 
{
	if (!strcmp(type,"erf"))
		return "erf:traces/100_packets.erf";
	if (!strcmp(type,"pcap"))
		return "pcap:traces/100_packets.pcap";
	if (!strcmp(type,"wtf"))
		return "wtf:traces/wed.wtf";
	if (!strcmp(type,"rtclient"))
		return "rtclient:chasm";
	return "unknown";
}

char *lookup_out_uri(const char *type) {
	if (!strcmp(type,"erf"))
		return "erf:traces/100_packets.out.erf";
	if (!strcmp(type,"pcap"))
		return "pcap:traces/100_packets.out.pcap";
	if (!strcmp(type,"wtf"))
		return "wtf:traces/wed.out.wtf";
	return "unknown";
}

int main(int argc, char *argv[]) {
        int psize = 0;
	int error = 0;
	int count = 0;
	int level = 0;
	int expected = 100;
	libtrace_t *trace;
	libtrace_out_t *outtrace;
	libtrace_packet_t *packet;

	trace = trace_create(lookup_uri(argv[1]));
	iferr(trace);

	if (strcmp(argv[1],"rtclient")==0)
		expected=101;

	outtrace = trace_create_output(lookup_out_uri(argv[2]));
	iferrout(outtrace);

	level=0;
	trace_config_output(outtrace,TRACE_OPTION_OUTPUT_COMPRESS,&level);
	iferrout(outtrace);

	trace_start(trace);
	iferr(trace);
	trace_start_output(outtrace);
	iferrout(outtrace);
	
	packet=trace_create_packet();
        for (;;) {
		if ((psize = trace_read_packet(trace, packet)) <0) {
			error = 1;
			break;
		}
		if (psize == 0) {
			error = 0;
			break;
		}
		count ++;
		trace_write_packet(outtrace,packet);
		iferrout(outtrace);
		if (count>100)
			break;
        }
	trace_destroy_packet(&packet);
	if (error == 0) {
		if (count == expected) {
			printf("success: %d packets read\n",expected);
		} else {
			printf("failure: %d packets expected, %d seen\n",expected,count);
			error = 1;
		}
	} else {
		iferr(trace);
	}
        trace_destroy(trace);
	trace_destroy_output(outtrace);
        return error;
}