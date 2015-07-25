/*
 * Copyright (c) 2009 Bartosz Jakoktochce <grypsy@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This module has been tested under OpenBSD 4.4/4.5
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/exec.h>
#include <sys/conf.h>
#include <sys/lkm.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/protosw.h>
#include <net/route.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>

#include <sys/proc.h>

/* 
 * Our prototypes 
 */

extern int	  lkmexists __P((struct lkm_table *));
extern char	 *inet_ntoa __P((struct in_addr));

static void new_input(struct proc *, void *, register_t *);
static sy_call_t	*old_my_input;

/*
 * Declare and initialise our module structure
 */

MOD_MISC("myinfo")

/*
 * Our handler function, used for load and unload.
 */

int
mymod_handler(lkmtp, cmd)
	struct lkm_table *lkmtp;
	int cmd;
{
	int s;

	switch(cmd) {

	case LKM_E_LOAD:
		
		/*
		 * Provide some sanity checking, making sure the module
		 * will not be loaded more than once.
		 */

		if (lkmexists(lkmtp))		
			return(EEXIST);
		
		/*
		 * We are changing the pointer to the
		 * function execve() to our own wrapper function.
		 */ 

		old_my_input = sysent[SYS_execve].sy_call; 
		sysent[SYS_execve].sy_call = (sy_call_t *)new_input;
		printf("AZUKE module: Initialized\n");

		break;

	case LKM_E_UNLOAD:

		/*
		 * Restore the structure back to normal when we 
		 * are unloaded. 
		 */

		sysent[SYS_execve].sy_call = (sy_call_t *)old_my_input;
		printf("AZUKE module: Terminated\n");		

		break;
	}

	return(0);
}

/*
 * Our external entry point, nothing to do but use DISPATCH.
 */

int
myinfo(lkmtp, cmd, ver)
	struct lkm_table *lkmtp;
	int cmd;
	int ver;
{
	DISPATCH(lkmtp, cmd, ver, mymod_handler, mymod_handler, lkm_nofunc);
}


/*
 * This is our new execve() function wrapper. At the end it executes the old execve()
 */

static void
new_input(struct proc *p, void *a, register_t *b)
{
	struct ucred *cred = p->p_ucred;	// user credentials

	uid_t user = cred->cr_uid; 		// getting UID of actual user

	/*
	 * Getting login of user executing the command. Note that the login may differ if user executed "su" command.
	 */

	char *login = p->p_session->s_login;

	/*
	 * Getting the executed command
	 */	
	 
	struct sys_execve_args {
	    char *path; 
	} *uap = a;
	
	char *pth = uap->path; 

	/*
	 * Write essensial informations to logfile in format <user> (<uid>) <command>
	 * and call the old execve() function passing arguments
	 */

	printf("AZUKE module: %s (uid:%d)  executed: %s\n", login, user, pth);
	(*old_my_input) (p,a,b);		
	return;
}
