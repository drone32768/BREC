/* 
 * This work is a derivative of "xvcd.c" (https://github.com/tmbinc/xvcd) 
 * by tmbinc, used under CC0 1.0 Universal 
 * (http://creativecommons.org/publicdomain/zero/1.0/). 
 * 
 * Many thanks!
 *
 * This work is a derivative of  "xvcServer.c" 
 * (https://github.com/Xilinx/XilinxVirtualCable)
 * by Xlinx, used under CC0 1.0 Universal
 * were incorporated.
 *
 * The differences are factoring out the gpio to be used as a library
 * to support different boards (including test emulators).  Provide 
 * a .o with jtag_bs interface specific to the board and link together to 
 * form a server.
 *
 * Connect to this server with iMPACT using cable configure, selecting
 * "Open Cable Plugin" and entering the following line in the drop down
 *
 *      xilinx_xvc host=192.168.0.170:2542 disableversioncheck=true
 *
 * Where the IP address is that of the actual xvcSrvr you are using.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "jtag_bs.h"

int verbose;

static int sread(int fd, void *target, int len)
{
	unsigned char *t = (unsigned char*)target;
	while (len)
	{
		int r = read(fd, t, len);
		if (r <= 0)
			return r;
		t += r;
		len -= r;
	}
	return 1;
}

//
// handle_data(fd) handles JTAG shift instructions.
//   To allow multiple programs to access the JTAG chain
//   at the same time, we only allow switching between
//   different clients only when we're in run_test_idle
//   after going test_logic_reset. This ensures that one
//   client can't disrupt the other client's IR or state.
//
int handle_data(int fd)
{
        const char xvcInfo[] = "xvcServer_v1.0:2048\n";
	int i;

	do
	{
		char cmd[128];
		unsigned char buffer[131072], result[131072];
		
		if (sread(fd, cmd, 2) != 1)
			return 1;
		
		if (memcmp(cmd, "ge", 2) == 0) {
			if (sread(fd, cmd, 6) != 1)
				return 1;
			memcpy(result, xvcInfo, strlen(xvcInfo));
			if (write(fd, result, strlen(xvcInfo)) != 
                                   strlen(xvcInfo)) {
				perror("write");
				return 1;
			}
			if (verbose) {
				printf(
                        "%u : Received command: 'getinfo'\n", (int)time(NULL));
				printf("\t Replied with %s\n", xvcInfo);
			}
			break;
		} else if (memcmp(cmd, "se", 2) == 0) {
			if (sread(fd, cmd, 9) != 1)
				return 1;
			memcpy(result, cmd + 5, 4);
			if (write(fd, result, 4) != 4) {
				perror("write");
				return 1;
			}
			if (verbose) {
				printf(
                        "%u : Received command: 'settck'\n", (int)time(NULL));
			printf("\t Replied with '%.*s'\n\n", 4, cmd + 5);
			}
			break;
                } else if (memcmp(cmd, "sh", 2) == 0) {
                        if (sread(fd, cmd, 4) != 1)
                                return 1;
                        if (verbose) {
                          printf(
                           "%u : Received command: 'shift'\n", (int)time(NULL));
                        }
                } else {

                        fprintf(stderr, "invalid cmd '%s'\n", cmd);
                        return 1;
                }

		int len;
		if (sread(fd, &len, 4) != 1)
		{
			fprintf(stderr, "reading length failed\n");
			return 1;
		}
		
		int nr_bytes = (len + 7) / 8;
		if (nr_bytes * 2 > sizeof(buffer))
		{
			fprintf(stderr, "buffer size exceeded\n");
			return 1;
		}
		
		if (sread(fd, buffer, nr_bytes * 2) != 1)
		{
			fprintf(stderr, "reading data failed\n");
			return 1;
		}
		
		memset(result, 0, nr_bytes);

		if (verbose)
		{
			printf("#");
			for (i = 0; i < nr_bytes * 2; ++i)
				printf("%02x ", buffer[i]);
			printf("\n");
		}

			for (i = 0; i < len; ++i)
			{
				//
				// Do the actual cycle.
				//
				
				int tms = !!(buffer[i/8] & (1<<(i&7)));
				int tdi = !!(buffer[nr_bytes + i/8] & (1<<(i&7)));
				result[i / 8] |= jtag_bs_get_tdo() << (i&7);
				jtag_bs_set_tms(tms);
				jtag_bs_set_tdi(tdi);
				jtag_bs_set_tck(1);
				jtag_bs_set_tck(0);
			}

		if (write(fd, result, nr_bytes) != nr_bytes)
		{
			perror("write");
			return 1;
		}
		
        } while( 1  );
	return 0;
}

int xvc_main(int argc, char **argv)
{
	int i;
	int s;
	int c;
	struct sockaddr_in address;
	
	opterr = 0;
	
	while ((c = getopt(argc, argv, "v")) != -1)
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case '?':
			fprintf(stderr, "usage: %s [-v]\n", *argv);
			return 1;
		}
	
	//
	// Initialize GPIOs (mapping them into the process, 
	// re-setting alternate functions, making input/outputs).
	//
	
	//
	// Listen on port 2542.
	//
	
	s = socket(AF_INET, SOCK_STREAM, 0);
	
	if (s < 0)
	{
		perror("socket");
		return 1;
	}
	
	i = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &i, sizeof i);
	
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(2542);
	address.sin_family = AF_INET;
	
	if (bind(s, (struct sockaddr*)&address, sizeof(address)) < 0)
	{
		perror("bind");
		return 1;
	}
	
	if (listen(s, 0) < 0)
	{
		perror("listen");
		return 1;
	}
	
	fd_set conn;
	int maxfd = 0;
	
	FD_ZERO(&conn);
	FD_SET(s, &conn);
	
	maxfd = s;
	
	while (1)
	{
		fd_set read = conn, except = conn;
		int fd;
		
		//
		// Look for work to do.
		//
		
		if (select(maxfd + 1, &read, 0, &except, 0) < 0)
		{
			perror("select");
			break;
		}
		
		for (fd = 0; fd <= maxfd; ++fd)
		{
			if (FD_ISSET(fd, &read))
			{
				//
				// Readable listen socket? Accept connection.
				//
				
				if (fd == s)
				{
					int newfd;
					socklen_t nsize = sizeof(address);
					
					newfd = accept(s, (struct sockaddr*)&address, &nsize);
					// if (verbose)
				        printf("connection accepted - fd %d\n", newfd);
					if (newfd < 0)
					{
						perror("accept");
					} else
					{
						if (newfd > maxfd)
						{
							maxfd = newfd;
						}
					}

/*
            	                        printf("setting TCP_NODELAY to 1\n");
                                 	int flag = 1;
                                 	int optResult = setsockopt(newfd,
            			  	  	  	  	  	 IPPROTO_TCP,
            			  	  	  	  	  	 TCP_NODELAY,
            			  	  	  	  	  	 (char *)&flag,
            			  	  	  	  	  	 sizeof(int));
                                 	if (optResult < 0) perror("TCP_NODELAY error");
*/
                                        FD_SET(newfd, &conn);
	                                jtag_bs_open();
				}
				//
				// Otherwise, do work.
				//
				else if (handle_data(fd))
				{
	                                jtag_bs_close();

					//
					// Close connection when required.
					//
					
					// if (verbose)
				printf("connection closed - fd %d\n", fd);
					close(fd);
					FD_CLR(fd, &conn);
				}
			}
			//
			// Abort connection?
			//
			else if (FD_ISSET(fd, &except))
			{
                                jtag_bs_close();

				// if (verbose)
				printf("connection aborted - fd %d\n", fd);
				close(fd);
				FD_CLR(fd, &conn);
				if (fd == s)
					break;
			}
		}
	}
	
	//
	// Un-map IOs.
	//
	
        jtag_bs_close();
	
	return 0;
}
