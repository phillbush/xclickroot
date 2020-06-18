#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>

static void usage(void);

/* X Variables */
static Display *dpy;
static Window rootwin;
static int screen;

/* xclickroot: execute a command by clicking on the root window */
int
main(int argc, char *argv[])
{
	unsigned button;
	XEvent ev;

	button = Button3;
	if ((*++argv)[0] == '-') {
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

	if (--argc == 0)
		usage();

	/* open connection to server and set X variables */
	if ((dpy = XOpenDisplay(NULL)) == NULL)
		errx(1, "cannot open display");
	screen = DefaultScreen(dpy);
	rootwin = DefaultRootWindow(dpy);

	signal(SIGCHLD, SIG_IGN);

	XGrabButton(dpy, button, 0, rootwin, False, ButtonPressMask,
	            GrabModeSync, GrabModeSync, None, None);

	while (!XWindowEvent(dpy, rootwin, ButtonPressMask, &ev)) {
		switch (ev.type) {
		case ButtonPress:
			if (ev.xbutton.button == button && ev.xbutton.subwindow == None) {
				if (fork() == 0) {
					execvp(*argv, argv);
					err(127, NULL);
				}
			}
			XAllowEvents(dpy, ReplayPointer, CurrentTime);
			break;
		}
	}

	XCloseDisplay(dpy);

	return 0;
}

/* show usage */
static void
usage(void)
{
	(void)fprintf(stderr, "usage: xclickroot [-12345lmr] command [args...]\n");
	exit(1);
}
