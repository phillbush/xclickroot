#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>

static void
usage(void)
{
	(void)fprintf(stderr, "usage: xclickroot [-12345lmr] command [args...]\n");
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	struct sigaction sa;
	unsigned button;
	XEvent ev;
	Display *dpy;
	Window rootwin;

	argv++;
	argc--;
	button = Button3;
	if (*argv && (*argv)[0] == '-') {
		switch ((*argv)[1]) {
		case '1': case 'l': button = Button1; break;
		case '2': case 'm': button = Button2; break;
		case '3': case 'r': button = Button3; break;
		case '4':           button = Button4; break;
		case '5':           button = Button5; break;
		default:
			usage();
			break;
		}

		if ((*argv)[2] != '\0')
			usage();

		argv++;
		argc--;
	}

	if (argc == 0)
		usage();

	/* don't leave zombies around */
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART;
	if (sigemptyset(&sa.sa_mask) == -1)
		err(EXIT_FAILURE, "sigemptyset");
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
		err(EXIT_FAILURE, "sigaction");

	/* open connection to server and set X variables */
	if ((dpy = XOpenDisplay(NULL)) == NULL)
		errx(EXIT_FAILURE, "cannot open display");

#if __OpenBSD__
	/*
	 * rpath: xlib needs it at runtime (for e.g. reading the error db)
	 * proc exec: running the program
	 */
	if (pledge("stdio rpath proc exec", NULL) == -1)
		err(EXIT_FAILURE, "pledge");
#endif

	rootwin = DefaultRootWindow(dpy);
	XGrabButton(dpy, button, AnyModifier, rootwin, False, ButtonPressMask,
	            GrabModeSync, GrabModeSync, None, None);
	while (!XWindowEvent(dpy, rootwin, ButtonPressMask, &ev)) {
		if (ev.type == ButtonPress) {
			if (ev.xbutton.button != button || ev.xbutton.subwindow != None) {
				XAllowEvents(dpy, ReplayPointer, CurrentTime);
				continue;
			}
			XUngrabPointer(dpy, ev.xbutton.time);
			XAllowEvents(dpy, ReplayPointer, CurrentTime);
			XFlush(dpy);
			switch (fork()) {
			case -1:
				warn("can't fork");
				break;
			case 0:
				execvp(*argv, argv);
				warn("can't exec %s", *argv);
				_exit(127);
			}
		}
	}
	XCloseDisplay(dpy);
	return 0;
}
