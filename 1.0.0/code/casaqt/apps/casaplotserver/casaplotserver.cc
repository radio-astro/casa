#include <signal.h>
#include <math.h>
#include <QApplication>
#include <casaqt/QtPlotServer/QtPlotServer.qo.h>
#include <casaqt/QtPlotServer/QtPlotSvrPanel.qo.h>

static void preprocess_args( int argc, const char *argv[], int &numargs, char **&args,
			     char *&dbus_name, bool &casapy_start );

int main( int argc, const char *argv[] ) {

    // don't let ^C [from casapy] kill the plot server...

    char *dbus_name = 0;
    bool casapy_start = false;

    char **args;
    int numargs;
    preprocess_args( argc, argv, numargs, args, dbus_name, casapy_start );

    if ( casapy_start ) {
	signal(SIGINT,SIG_IGN);
    }

    QApplication qapp(numargs, args); 

    casa::QtPlotServer plot_server(dbus_name);

    return qapp.exec();

}

// processes argv into args stripping out the the viewer setup flags... numargs has the number
// of args, and the last arg (not included in numargs count) is null (for execvp)
static void preprocess_args( int argc, const char *argv[], int &numargs, char **&args,
			     char *&dbus_name, bool &casapy_start ) {

    casapy_start = false;
    dbus_name = 0;

    // pre-process the command line arguments (for now), it looks like
    // the current scheme is to hard-wire a limited set of command line
    // parameters... should be reworked in the future.
    args = (char**) malloc(sizeof(char*)*(argc+2));
    numargs = 0;

    args[numargs++] = strdup(argv[0]);
    for ( int x = 0; x < argc; ++x ) {
	if ( ! strncmp(argv[x],"--dbusname",10) ) {
	    if ( argv[x][10] == '=' ) {
		char *name = strdup( &argv[x][11] );
		if ( strlen(name) <= 0 ) {
		    free( name );
		} else {
		    dbus_name = name;
		}
	    } else if ( x + 1 < argc ) {
		dbus_name = strdup(argv[++x]);
	    }
	} else if ( ! strcmp(argv[x],"--casapy") ) {
	    casapy_start = true;
	} else {
	    args[numargs++] = strdup(argv[x]);
	}
    }

    args[numargs] = 0;
}
