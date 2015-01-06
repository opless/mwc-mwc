/*
 * sh/exec2.c
 * Bourne shell.
 * System part of execution.
 */

#include "sh.h"
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * Wait for the given process to complete.
 */
waitc(pid)
int pid;
{
	register int f;
	unsigned s;
	register int w, n;
	void	(*ssig)();

	if (pid < 0)
		f = -pid;
	else
		f = pid;
	if ((ssig=signal(SIGINT, SIG_IGN)) != SIG_IGN)
		signal(SIGINT, sigintr);
	for (;;) {
		if ((w=wait(&s)) >= 0) {
			if (n = (s&0177)) {
				if (n==SIGINT)
					sigintr(n);
				else if (n==SIGPIPE && spipe
					 && (w >= spipe && w <= f))
					;
				else {
					if (w != f)
						prints("%d: ", w);
					if (n==SIGSYS && (s&0200)==0)
					    prints("exec failed");
					else if (n>0 && n<=NSIG)
						prints("%s", signame[n]);
					else
						prints("status %d", n);
					if (s & 0200)
						prints(" -- core dumped");
					prints("\n");
				}
				s = 200 + n;
			} else
				s >>= 8;
			if (w == f) {
				slret = s;
				break;
			}
		} else if (errno==EINTR) {
			if (pid > 0)
				continue;
			else {
				slret = 0;
				break;
			}
		} else if (errno==ECHILD) {
			if (pid == 0)	
				slret = 0;
			else
				slret = ECHILD;
			break;
		} else {
			panic(6);
			NOTREACHED;
		}
	}
	signal(SIGINT, ssig);
	return (slret);
}

/*
 * Make an imperfect copy of ourself.
 */
clone()
{
	register int f;
	register SES *sp;

	if ((f=fork()) < 0) {
		prints("Try again\n");
		reset(RSYSER);
		NOTREACHED;
	} else if (f == 0) {
		nllflag = 1;
		sflag = iflag = cflag = no1flag = lgnflag = 0;
		slret = spipe = 0;
		sp = sesp;
		sp->s_con->c_next = NULL;

		/*
		 * Originally this function suppressed function definitions
		 * in child processes. This is wrong. Note that we have to
		 * detach the temporary-file stuff from the child functions,
		 * however, so that functions with here-documents work (as
		 * long as the top-level shell is alive, that is).
		 */
#if 1
		subshell_shell_fns ();
#else
		sh_fnp = NULL;
#endif

		while (sp) {
			if (sp->s_type == SFILE)
				fclose(sp->s_ifp);
			sp = sp->s_next;
		}
		dflttrp(IFORK);
	}
	return (f);
}

/*
 * Open a pipe, panic on failure.
 * Otherwise return as a clone.
 */
pipeline(pv)
int *pv;
{
	if (pipe(pv) < 0) {
		prints("Pipe failed\n");
		reset(RSYSER);
		NOTREACHED;
	}
	return (clone());
}

/*
 * Try to execute a file in several ways.
 *	A return is always an error.
 *	Used by exec in inline.
 */
flexec()
{
	ffind(NULL);
	while (ffind(vpath, *nargv, 1)) {
		execve(strt, nargv, nenvp);
		if (errno==ENOEXEC) {
			scmdp = duplstr(strt, 0);
			nargc += 1;
			nargv -= 1;
			nargv = vdupl(nargv);
			nenvp = vdupl(nenvp);
			while (sesp) {
				freebuf (sesp->s_bpp);
				sesp = sesp->s_next;
			}
			longjmp (restart, 1);
		}
		if (errno==E2BIG) {
			e2big (nargv [0]);
			return -1;
		}
	}
	ecantfind (nargv [0]);
	return -1;
}

ALLOC_COUNT (undo)

/*
 * Process a redirection vector.
 * Abort and return -1 at the first failure, return 0 for success.
 *
 * NB: In ordr to support redirection of builtins, an extra argument to this
 * function has been addded. If undo is NULL, things remain as before, but if
 * non-NULL it is taken to be the head of a list of undo items. As the list of
 * redirections is processed, undo entries will be added to the head of the
 * list, so that the caller will see the list in the appropriate order for
 * undoing the redirections.
 */
redirect(iovp, undo)
char **iovp;
REDIR_UNDO ** undo;
{
	register char **iopp;
	register char *io;
	register int op;
	int u1, u2;
	REDIR_UNDO * undo_node;

	for (iopp = iovp ; (io = * iopp ++) != NULL; ) {
		if (class (* io, MDIGI))
			u1 = * io ++ - '0';
		else
			u1 = * io == '<' ? 0 : 1;

		/*
		 * NIGEL: When redirecting stdin/stdout/stderr, we have to
		 * worry about the cached state of the FILE structure.
		 */

		switch (u1) {
		case 0:
			clearerr (stdin);
			break;

		case 1:
			clearerr (stdout);
			break;

		case 2:
			clearerr (stderr);
			break;

		default:
			/* DO NOTHING */
		}

		if (undo) {
			/*
			 * Create and link in the undo node now. Stash away a
			 * spare copy of the original fd as well. Note that
			 * this may cause semantic changes in attempts to
			 * redirect from fds that would otherwise be closed,
			 * but that error was never diagnosed before anyway...
			 */

			ALLOC_ALLOC (undo);
			undo_node = (REDIR_UNDO *) salloc (sizeof (REDIR_UNDO));
			undo_node->ru_next = * undo;
			undo_node->ru_oldfd = u1;
			if ((undo_node->ru_newfd = dup (u1)) == -1) {
				if (errno != EBADF)
					eredirundo ();
			} else {
				fcntl (undo_node->ru_newfd, F_SETFD,
				       fcntl (undo_node->ru_newfd, F_GETFD,
					      0) | FD_CLOEXEC);
			}
			* undo = undo_node;
		} else
			undo_node = NULL;

		for (op = 0 ; ; io += 1) {
			if (*io == '>')
				op += 1;
			else if (* io == '<')
				op -= 1;
			else
				break;
		}
		if (* io ++ == '&') {
			if (op != 1 && op != -1) {
				panic (7);
				NOTREACHED;
			}
			u2 = * io ++;
			if (u2 == '-') {
				close (u1);
				continue;
			} else if (! class (u2, MDIGI)) {
				eredir ();
				return -1;
			} else {
				u2 -= '0';
				dup2 (u2, u1);
				continue;
			}
		}
		for (io -= 1 ; * io == ' ' || * io == '\t' ; io += 1)
			/* DO NOTHING */;

		switch (op) {
		case -2:	/* Unquoted here */
				/* Fall through */
		case -1:	/* Input file, quoted here */
			if ((u2 = open (io, 0)) < 0) {
				ecantopen (io);
				return -1;
			}
			if (op == -2 && (u2 = evalhere (u2)) < 0)
				return -1;
			dup2 (u2, u1);
			close (u2);
			continue;

		case 2:		/* Append to output */
			if ((u2 = open (io, 1)) >= 0) {
				lseek (u2, 0L, SEEK_END);
				dup2 (u2, u1);
				close (u2);
				continue;
			}
			/* FALL THROUGH */

		case 1:		/* Output file */
			u2 = creat (io, S_IRUSR | S_IWUSR | S_IRGRP |
					S_IWGRP | S_IROTH | S_IWOTH);
			if (u2 < 0) {
				ecantmake (io);
				return -1;
			}
			dup2 (u2, u1);
			close (u2);
			continue;

		default:
			panic (8);
			NOTREACHED;
		}
	}
	return 0;
}

/*
 * Undo a redirection sequence, reclaiming all the space for the undo nodes.
 */
redirundo (undo)
REDIR_UNDO ** undo;
{
	REDIR_UNDO    *	undo_node;

	while ((undo_node = * undo) != NULL) {
		* undo = undo_node->ru_next;
		if (undo_node->ru_newfd == -1)
			close (undo_node->ru_oldfd);
		else {
			dup2 (undo_node->ru_newfd, undo_node->ru_oldfd);
			close (undo_node->ru_newfd);
		}
		ALLOC_FREE (undo);
		sfree (undo_node);
	}
	return 0;
}


#ifdef NAMEPIPE
/*
 * Create a named pipe.
 */
npipe(np)
register NODE *np;
{
	register char *tmp;
	register int f;
	char *tvec[2];

	tmp = shtmp();
	mknod(tmp, S_IFPIP, 0);
	if ((f = clone()) == 0) {
		if (np->n_type==NRPIPE)
			strt[0] = '<';
		else
			strt[0] = '>';
		strt[1] = '\0';
		tvec[0] = strcat(strt, tmp);
		tvec[1] = NULL;
		redirect(tvec, NULL);
		command(np->n_auxp);
		exit(slret);
		NOTREACHED;
	}
	remember_temp (tmp);
	strcpy(strt, tmp);
	return;
}
#endif

/*
 * Search a path, perhaps repeatedly, for a file with access mode.
 * Is called with paths == NULL to reset pointers.
 * UGLY, but it works.
 */
ffind(paths, file, mode)
char *paths, *file;
{
	register char c, *cp1, *cp2;
	static char *fp, *ff, *fcp;

	if (paths==NULL) {
		fp = ff = fcp = NULL;
		return (0);
	}
	if (ff!=file) {
		fp = fcp = paths;
		ff = file;
		if (index(ff, '/')!=NULL)
			fcp = "";
	}
	for (c=':'; c==':'; ) {
		cp1 = fcp;
		cp2 = strt;
		while (*cp1 && *cp1!=':')
			*cp2++ = *cp1++;
		if (cp2 != strt)
			*cp2++ = '/';
		c = *cp1++;
		fcp = cp1;
		cp1 = ff;
		while (*cp2++ = *cp1++);
		if (access(strt, mode)>=0)
			return (1);
	}
	return (0);
}

/*
 * execute a non standard shell.
 */
exshell(vp) VAR *vp;
{
	char *vshell;
	register char *p;

	vshell = vp->v_strp;
	while (*vshell && *vshell++ != '=');
	/* Construct -name argv[0] */
	if ((p = rindex(vshell, '/')) != NULL)
		p += 1;
	else
		p = vshell;
	strcpy(strt, "-");
	strcat(strt, p);
	/* Construct argv */
	nargv = makargl();
	nargv = addargl(nargv, "sh");
	nargv = addargl(nargv, duplstr(strt, 0));
	nargc = 2;
	/* Construct envp */
	nenvp = makargl();
	nenvp = envlvar(nenvp);
	/* Try exec */
	execve(vshell, nargv+1, nenvp);

	if (errno==ENOEXEC) {
		fakearg(1, nargc, nargv, nenvp);
		sargc = 0;
		sargp = nargv+2;
		sarg0 = nargv[1];
		nllflag = 0;
		return session(SFILE, vshell);
	}
	fprintf(stderr, "No shell: %s\n", vshell);
	exit(1);
}

/*
 * Check to see if we have mail.
 */
checkmail()
{
	static long mailsize = -1;
	struct stat sbuf;

	if (*vmail == '\0')
		return;
	if (stat(vmail, &sbuf)<0) {
		mailsize = 0;
	} else {
		if (sbuf.st_size != 0
		 && sbuf.st_size > mailsize) {
			if (mailsize == -1)
				prints("You have mail.\n");
			else
				prints("You have new mail.\n");
		}
		mailsize = sbuf.st_size;
	}
}
