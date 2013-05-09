//# qtviewer.cc:  main program for standalone Qt viewer
//# Copyright (C) 2005,2009,2010
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <ios>
#include <iostream>
#include <casa/aips.h>
#include <casa/Inputs/Input.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <display/Display/StandAloneDisplayApp.h>
// (Configures pgplot for stand-alone Display Library apps).

#include <casa/Logging/StreamLogSink.h>
#include <casa/Logging/LogSink.h>

#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <display/QtViewer/QtViewer.qo.h>

#include <display/Utilities/Lowlevel.h>

#include <unistd.h>
#include <sys/stat.h>

/*
#include <graphics/X11/X_enter.h>
#include   <QApplication>
#include <graphics/X11/X_exit.h>
*/

#include <casadbus/utilities/Diagnostic.h>
#include <display/DisplayErrors.h>

#include <casa/namespace.h>

static pid_t manager_root_pid = 0;
static bool sigterm_received = false;
static void preprocess_args( int argc, const char *argv[], int &numargs, char **&args,
                             char *&dbus_name, bool &do_dbus, bool &inital_run, bool &server_startup,
                             bool &without_gui, bool &persistent, bool &casapy_start,
                             char *&logfile_path );
static void start_manager_root( const char *origname, int numargs, char **args,
                                const char *dbusname, bool without_gui, pid_t root_pid );
static void launch_server( const char *origname, int numargs, char **args,
                           const char *dbusname, bool without_gui,
                           bool persistent, bool casapy_start );
static char *find_xvfb( const char *paths );
static pid_t launch_xvfb( const char *name, pid_t pid, char *&display, char *&authority );

static void exiting_server( int /*sig*/ ) {
	exit(0);
}
static void signal_manager_root( int sig ) {
	if ( manager_root_pid && ! sigterm_received ) {
		killpg( manager_root_pid, sig );
		sigterm_received = true;
	}
	signal( sig, signal_manager_root );
	exit(0);
}


class ViewerApp : public QApplication {
public:
	ViewerApp( int &argc, char **argv, bool gui_enabled ) : QApplication(argc, argv, gui_enabled), viewer_(0) { }
	bool notify( QObject *receiver, QEvent *e );
	void subscribe( QtViewer *v ) {
		viewer_ = v;
	}
private:
	QtViewer *viewer_;
};

bool ViewerApp::notify( QObject *receiver, QEvent *e ) {
	// cap qt event recursion limit at 500 events deep...
	static unsigned long recursion_count = 0;
	if ( recursion_count > 500 ) {
		qWarning( ) << "qt event recursion limit reached...";
		return false;
	}

	try {
		recursion_count++;

		// notify QtViewer when application is activated/deactivated, e.g. when OSX switches to
		// "mission control", allowing the viewer to recognize that the mouse cursor has left...
		if ( e->type() == QEvent::ApplicationActivate || e->type() == QEvent::ApplicationDeactivate )
			if ( viewer_ != 0 ) viewer_->activate( e->type() == QEvent::ApplicationActivate ? true : false );

		bool result = QApplication::notify(receiver,e);
		if ( recursion_count != 0 ) recursion_count--;
		return result;
	} catch ( AipsError e ) {
		qWarning( ) << "unhandled exception:" << e.getMesg().c_str();
		if ( recursion_count != 0 ) recursion_count--;
		return false;
	} catch ( viewer::internal_error e ) {
		qWarning( ) << "internal exception:" << e.what( );
		if ( recursion_count != 0 ) recursion_count--;
		return false;
	} catch ( std::exception e ) {
		qWarning( ) << "std exception:" << e.what( );
		if ( recursion_count != 0 ) recursion_count--;
		return false;
	} catch ( ... ) {
		qWarning("unhandled exception... ");
		qDebug() << "from" << receiver->objectName() << "from event type" << e->type();
		qFatal("exiting...");
		exit(1);
	}
	return true;
}

int main( int argc, const char *argv[] ) {

	casa::dbus::diagnostic.argv( argc, argv );

	bool server_startup = false;
	bool without_gui = false;
	bool persistent = false;
	bool casapy_start = false;
	char *dbus_name = 0;
	bool with_dbus = false;
	char *logfile_path = 0;
	bool initial_run = false;


	char **args;
	int numargs;

	signal( SIGTERM, exiting_server );

	// On Mac OS X by default, Qt swaps the Control and Meta (Command) keys (i.e., whenever
	// Control is pressed, Qt sends Meta, and whenever Meta is pressed Control is sent).
	// When this attribute is true, Qt will not do the flip. QKeySequence::StandardShortcuts
	// will also flip accordingly (i.e., QKeySequence::Copy will be Command+C on the keyboard
	// regardless of the value set, though what is output for
	// QKeySequence::toString(QKeySequence::PortableText) will be different).
	//
	// This avoids this swapping, making the behavior more consistent when running on an X11
	// system displaying on OSX or just when using it with OSX.
	//
	// This option makes *NO*DIFFERENCE* for the context menu on the "Point Button Tool".
	// Thu Aug 30 10:32:15 EDT 2012 <drs>
	//
//     QCoreApplication::setAttribute(Qt::AA_MacDontSwapCtrlAndMeta);

	preprocess_args( argc, argv, numargs, args, dbus_name, with_dbus,
	                 initial_run, server_startup, without_gui, persistent,
	                 casapy_start, logfile_path );

	//
	// setup casa logging's global sink, if the user supplied a path...
	//
	if ( logfile_path ) {
		FILE *file = fopen(logfile_path,"a");
		if ( file ) {
			fclose(file);
			casa::LogSinkInterface *sink = new casa::StreamLogSink(new ofstream(logfile_path,std::ios_base::app));
			casa::LogSink::globalSink(sink);
		}
	}

	if ( (server_startup || without_gui) && initial_run ) {
		launch_server( argv[0], numargs, args, dbus_name, without_gui,
		               persistent, casapy_start );
		exit(0);
	}

	INITIALIZE_PGPLOT
	try {

		ViewerApp qapp(numargs, args, true);

		// if it's a server, stick around even if all windows are closed...
		if ( server_startup ) {
			qapp.setQuitOnLastWindowClosed(false);
		}

		String	   filename    = "",
		           displaytype = "",
		           datatype    = "",
		           arg2        = "",
		           arg3        = "";


		Int narg;

#ifndef AIPS_DARWIN
		narg = qapp.argc();
		if(narg>1) filename = qapp.argv()[1];
		if(narg>2) arg2     = qapp.argv()[2];
		if(narg>3) arg3     = qapp.argv()[3];
#else
		narg = numargs;
		if(narg>1) filename = args[1];
		if(narg>2) arg2     = args[2];
		if(narg>3) arg3     = args[3];
#endif

		// Workaround for python task's "empty parameter" disability....
		if(filename==".") filename="";

		if ( filename != "" && filename[0] == '-' && filename[1] == '-' ) {
			struct stat statbuf;
			if ( stat( filename.c_str( ), &statbuf ) == -1 ) {
				filename = "";
			}
		}

		// Pass along the remaining arguments to QtViewer...
		// instead of littering the ctor arguments...
		std::list<std::string> stdargs;
		for ( int arg_index=0; args[arg_index]; ++arg_index )
			stdargs.push_back(args[arg_index]);

		QtViewer* v = new QtViewer( stdargs, server_startup || with_dbus, dbus_name );
		qapp.subscribe(v);

		if ( ! server_startup ) {

			// define the panel
			QtDisplayPanelGui* dpg;

			dpg = v->createDPG( );

			QtDisplayData* qdd = 0;

			// Data files are now typed automatically (see v_->filetype(filename),
			// below; e.g.: "image" or "ms").  arg2 need be used only to specify a
			// displaytype, and then only when it is not the default displaytype
			// for the datatype (e.g.  viewer "my.im", "contour" ).
			//
			// The user can enter an lel expression in place of filename, but such
			// an expression _cannot_ be automatically typed.  In this case the user
			// must have "lel" in arg2 (or in arg3: the only case where arg3 is vaguely
			// useful is something like:
			//
			//   casaviewer "'my.im'-'other.im'"  contour  lel
			//
			// arg3 is not even offered in the viewer casapy task).
			//
			// The logic below allows displaytypes or datatypes to be entered in
			// either order, and for old datatypes to be used other than "lel" (these
			// are simply ignored).  This allows old (deprecated) parameter usage in
			// scripts (such as viewer("my.ms", "ms")) to continue to be understood.
			//
			// However, the canonical 'allowed' parameter set (per user documentation)
			// is now just:
			//
			//   viewer [filename [displaytype]]
			//

			if(filename!="") {

				Bool tryDDcreate = True;

				if(arg3=="lel" || arg2=="lel") {

					// (this means that first ('filename') parameter is supposed to
					// contain a valid lel (image expression) string; this is advanced
					// (and undocumented) parameter usage).

					datatype = "lel";
					displaytype = (arg3=="lel")? arg2 : arg3;
					v->dataDisplaysAs(datatype, displaytype);
				} else {

					datatype = v->filetype(filename);


					if(datatype=="restore") {

						// filename is a restore file.

						tryDDcreate = False;

						dpg->restorePanelState(filename);
					}

					else {

						if(datatype=="nonexistent") {
							cerr << "***Can't find  " << filename << "***" << endl;
							tryDDcreate = False;
						}

						if(datatype=="unknown") {
							cerr << "***Unknown file type for  " << filename << "***" << endl;
							tryDDcreate = False;
						}

						// filename names a normal data file.  If user has passed a valid
						// displaytype in either arg2 or arg3, use it; otherwise, the
						// default displaytype for datatype will be inserted.

						displaytype = arg2;
						if(!v->dataDisplaysAs(datatype, displaytype)) {
							displaytype = arg3;
							v->dataDisplaysAs(datatype, displaytype);
						}
					}
				}


				if(tryDDcreate) {

					qdd = dpg->createDD(filename, datatype, displaytype);


					if(qdd==0)  cerr << dpg->errMsg() << endl;
				}
			}

			dpg->show();

			if( dpg->isEmptyDD() ) dpg->showDataManager();

		}

		Int stat = qapp.exec();

		//delete dpg;		// Used to lead to crash (double-deletion
		// of MWCTools); should work now.

		delete v;

		// cerr<<"Normal exit -- status: "<<stat<<endl;	//#diag

		return stat;
	}

	catch (const casa::AipsError& err) {
		cerr<<"**"<<err.getMesg()<<endl;
	} catch (...) {
		cerr<<"**non-AipsError exception**"<<endl;
	}

}

// processes argv into args stripping out the the viewer setup flags... numargs has the number
// of args, and the last arg (not included in numargs count) is null (for execvp)
static void preprocess_args( int argc, const char *argv[], int &numargs, char **&args,
                             char *&dbus_name, bool &with_dbus, bool &initial_run,
                             bool &server_startup, bool &without_gui, bool &persistent,
                             bool &casapy_start, char *&logfile_path ) {

	without_gui = false;
	persistent = false;
	casapy_start = false;
	dbus_name = 0;

	initial_run = (isdigit(argv[0][0]) ? false : true);

	for ( int x = 0; x < argc; ++x ) {
		if ( ! strncmp(argv[x],"--nogui",7) ) {
			without_gui = true;
			if ( argv[x][7] == '=' ) {
				char *name = strdup( &argv[x][8] );
				if ( strlen(name) <= 0 ) {
					free( name );
				} else {
					dbus_name = name;
				}
			}
		} else if ( ! strncmp(argv[x],"--server",8) ) {
			server_startup = true;
			if ( argv[x][8] == '=' ) {
				char *name = strdup( &argv[x][9] );
				if ( strlen(name) <= 0 ) {
					free( name );
				} else {
					dbus_name = name;
				}
			}
		} else if ( ! strncmp(argv[x],"--dbusname",10) ) {
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
		} else if ( ! strcmp(argv[x],"--persist") ) {
			persistent = true;
		} else if ( ! strcmp(argv[x],"--casapy") ) {
			casapy_start = true;
		} else if ( ! strcmp(argv[x],"--dbus") ) {
			with_dbus = true;
		}  else if ( ! strncmp(argv[x],"--rcdir",7) ) {
			if ( argv[x][7] == '=' ) {
				viewer::setrcDir(&argv[x][8]);
			} else if ( x + 1 < argc ) {
				viewer::setrcDir(argv[++x]);
			}
		} else if ( ! strncmp(argv[x],"--casalogfile",13) ) {
			if ( argv[x][13] == '=' ) {
				char *file = strdup( &argv[x][14] );
				if ( strlen(file) <= 0 ) {
					free( file );
				} else {
					logfile_path = file;
				}
			} else if ( x + 1 < argc ) {
				logfile_path = strdup(argv[++x]);
			}
		}
	}

	char *orig_name = strdup(argv[0]);
	char *name = orig_name + strlen(orig_name) - 1;
	while ( name > orig_name && *name != '/' ) --name;
	if ( *name == '/' ) ++name;

	// pre-process the command line arguments (for now), it looks like
	// the current scheme is to hard-wire a limited set of command line
	// parameters... should be reworked in the future.
	args = (char**) malloc(sizeof(char*)*(argc+2));

	numargs = 0;
	if ( name != orig_name ) {
		args[numargs++] = strdup(name);
		free(orig_name);
	} else {
		args[numargs++] = orig_name;
	}


	if ( initial_run && (without_gui || server_startup) ) {
		// --nogui imples --server and for consistency, --server also forks child
		char *arg;
		if ( dbus_name ) {
			arg = (char*) malloc( sizeof(char)*(strlen(dbus_name)+15) );
			sprintf( arg, "--server=%s", dbus_name );
		} else
			arg = strdup( "--server" );

		args[numargs++] = arg;
	}

	for ( int x = 1; x < argc; ++x ) {
		if ( ! strcmp(argv[x], "--dbusname") ) {
			++x;
		} else if ( strncmp( argv[x], "--server", 8 ) &&
		            strncmp( argv[x], "--nogui", 7 ) &&
		            strcmp( argv[x], "--persist" ) &&
		            strcmp( argv[x], "--casapy" ) &&
		            strcmp( argv[x], "--eso3d" ) &&
		            strncmp( argv[x],"--dbusname",10) )
			args[numargs++] = strdup(argv[x]);
	}
	args[numargs] = 0;

	std::list<char*> files;
	std::list<char*> others;
	std::list<char*> flags;
	struct stat statbuf;
	for ( int x = 1; x < numargs; ++x ) {
		if ( ! stat( args[x], &statbuf ) ) {
			files.push_back(args[x]);
		} else if ( args[x][0] == '-' && args[x][1] == '-' ) {
			flags.push_back(args[x]);
		} else {
			others.push_back(args[x]);
		}
	}

	int offset = 1;
	for ( std::list<char*>::iterator iter = files.begin( );
	        iter != files.end( ); ++iter ) {
		args[offset++] = *iter;
	}
	for ( std::list<char*>::iterator iter = others.begin( );
	        iter != others.end( ); ++iter ) {
		args[offset++] = *iter;
	}
	for ( std::list<char*>::iterator iter = flags.begin( );
	        iter != flags.end( ); ++iter ) {
		args[offset++] = *iter;
	}
}

void start_manager_root( const char *origname, int /*numargs*/, char **args, const char */*dbusname*/,
                         bool without_gui, pid_t root_pid ) {

	pid_t child_xvfb = 0;
	char *display = 0;
	char *authority = 0;
	if ( without_gui ) {
		child_xvfb = launch_xvfb( args[0], root_pid, display, authority );
		sleep(2);
	}

	char *name = args[0];

	if ( display ) {
		putenv(display);
	}
	if ( authority ) {
		putenv( authority );
	}

	char *new_name = (char*) malloc( sizeof(char)*(strlen(name)+25) );
	sprintf( new_name, "%d-%s-svr", root_pid, name );
	args[0] = new_name;

	free(name);

	execvp( origname, args );
}

void launch_server( const char *origname, int numargs, char **args,
                    const char *dbusname, bool without_gui,
                    bool persistent, bool casapy_start ) {

	// ______________________________________________________________________________________________________
	//
	// root (receives sigterm & is part of casapy process group)
	//   |
	//   +-- child-root (sets new process group, forks off child to be independent, passes back pid of child, exits)
	//         |
	//         +-- manager-root (sets up xvfb if necessary, starts lowly_viewer, waits for lowly_viewer to exit or SIGTERM from root)
	//                |
	//                +-- xvfb (without_gui only)
	//                |
	//                +-- actual lowly_viewer (perhaps with DISPLAY set)
	//
	// ______________________________________________________________________________________________________

	pid_t root_pid = getpid();

	// don't let ^C [from casapy] kill the viewer...
	signal(SIGINT,SIG_IGN);
	// sent when STDIN/STDOUT is closed...
	// e.g. when running regressions...
	//      or when using expect...
	signal(SIGHUP,SIG_IGN);

	int vio[2];
	if ( pipe(vio) < 0 ) {
		perror( "child-root pipe" );
		exit(1);
	}

	pid_t child_root = fork( );
	if ( child_root == 0 ) {
		signal(SIGINT,SIG_IGN);
		close(vio[0]);

#ifdef SIGTTOU
		signal(SIGTTOU, SIG_IGN);
#endif
#ifdef SIGTTIN
		signal(SIGTTIN, SIG_IGN);
#endif
#ifdef SIGTSTP
		signal(SIGTSTP, SIG_IGN);
#endif

		if ( fork( ) != 0 ) {
			exit(0);
		}

		if ( setpgrp( ) == -1 ) {
			fprintf( stderr, "trying to do the setpgrp...%d...\n", getpid() );
			perror( "setpgrp issue" );
		}
// 	if ( setsid( ) == -1 ) {
// 	    fprintf( stderr, "trying to do the setsid...%d...\n", getpid() );
// 	    perror( "setsid issue" );
// 	}

		char buffer[50];
		sprintf( buffer,"%d", getpid() );
		write( vio[1], buffer, strlen(buffer) + 1 );
		close(vio[1]);

		start_manager_root( origname, numargs, args, dbusname, without_gui, root_pid );
		fprintf( stderr, "start_manager_root( ) should not have returned...%d...\n", getpid() );
		exit(1);
	}

	char buffer[50];
	close(vio[1]);
	read( vio[0], buffer, 50 );
	manager_root_pid = (pid_t) atoi(buffer);
	close(vio[0]);

	int child_root_status;
	waitpid( child_root, &child_root_status, 0 );
	if ( persistent || ! casapy_start ) {
		exit(0);
	}

	// catch the exit signal, and signal the manager_root
	signal(SIGTERM, signal_manager_root);

	while ( ! sigterm_received ) {
		sleep(5);
	}
}

char *find_xvfb( const char *paths ) {

	if ( !paths ) return 0;

	const char *pathptr = paths;
	char buffer[PATH_MAX];		// security: too long paths could result
	// in exec'ing an unintended binary
	char *result = 0;
	const char *p = paths;
	do {
		if ( *p == ':' || *p == '\0' ) {
			if ( p == pathptr ) {
				pathptr = p+1;
				continue;
			}
			if ((p - pathptr + 6) > (int)sizeof(buffer) ) {
				pathptr = p+1;
				continue;
			}
			memcpy(buffer, pathptr, p - pathptr);
			memcpy(&buffer[p-pathptr], "/Xvfb", 6);
			if ( access( buffer, X_OK ) == 0 ) {
				result = strdup(buffer);
				break;
			}
			pathptr = p+1;
		}
	} while ( *p++ );

	return result;
}

pid_t launch_xvfb( const char *name, pid_t pid, char *&display, char *&authority ) {

	pid_t child_xvfb = 0;

#ifdef Q_WS_X11

	char *home = getenv("HOME");

	display = 0;
	authority = 0;

	char *xvfb_path = find_xvfb( getenv("PATH") );
	if ( xvfb_path == 0 ) {
		xvfb_path = find_xvfb( "/usr/X11R6/bin:/usr/X11/bin:/usr/bin" );
		if ( xvfb_path == 0 ) {
			fprintf( stderr, "casaviewer: could not start Xvfb\n" );
			exit(1);
		}
	}

#if defined(__APPLE___)
	srandomdev( );
#else
	union {
		void *foo;
		unsigned bar;
	};
	foo = &home;
	srandom(bar);
#endif

	char xauth[33];
	for ( int x=0; x < 32; x += 8 ) {
		sprintf( &xauth[x], "%08x", (int) random( ) );
	}
	xauth[32] = '\0';


	if ( ! home ) {
		fprintf( stderr, "HOME is not defined in your environment\n" );
		exit(1);
	}

	char *xvfb_name = (char*) malloc( sizeof(char)*(strlen(name)+50) );
	sprintf( xvfb_name, "%d-%s-xvfb", pid, name );

	authority = (char*) malloc( sizeof(char)*(strlen(home)+160) );
	sprintf( authority, "%s/.casa", home );
	viewer::create_dir( authority );
	sprintf( authority, "%s/.casa/xauthority", home );

	const int display_start=6;
	int display_num;
	for ( display_num=display_start; display_num < (display_start+80); ++display_num ) {

		int io[2];
		if ( pipe(io) < 0 ) {
			perror( "xvfb pipe" );
			exit(1);
		}

		child_xvfb = fork( );
		if ( ! child_xvfb ) {

			close(io[0]);
			// don't let ^C [from casapy] kill the viewer...
// 	    signal(SIGINT,SIG_IGN);

// 	    close( fileno(stdin) );
// 	    dup2( io[1], fileno(stdout) );
			dup2( io[1], fileno(stderr) );

			char screen_arg[50];
			sprintf( screen_arg, ":%d", display_num );

			execl( xvfb_path, xvfb_name, screen_arg, "-screen", "0","2048x2048x24+32",
			       "-auth", authority, (char*) 0 );
			fprintf( stderr, "exec of Xvfb should not have returned...%d...\n", getpid() );
			perror( "virtual frame buffer" );
			exit(1);
		}

		close(io[1]);
		bool failure = false;
		fd_set read_set;
		struct timeval wait_time = { 3, 0 };
		while ( 1 ) {
			FD_ZERO( &read_set );
			FD_SET( io[0], &read_set );
			int result = select( io[0] + 1, &read_set, 0, 0, &wait_time );
			if ( result > 0 ) {
				char buffer[1024];
				int nbytes = read( io[0], buffer, sizeof(buffer)-1 );
				if ( nbytes > 0 ) {
					buffer[nbytes] = '\0';
					// HANDLE: 'FreeFontPath: FPE "unix/:7100" refcount is 2, should be 1; fixing.'
					if ( strstr( buffer, "already active" ) ||
					        strstr( buffer, "server already running" ) ||
					        strstr( buffer, "SocketCreateListener() failed") ) {
						failure = true;
						int status;
						waitpid(child_xvfb, &status, 0);
						break;
					}
				} else if ( nbytes == 0 ) {
					fprintf( stderr, "OK, no bytes read while starting xvfb, timeout?...%d...\n", getpid() );
					failure = true;
					break;
				} else if ( nbytes < 0 ) {
					// some error has occurred
					fprintf( stderr, "oops something strange occurred when starting up xvfb...%d...\n", getpid() );
					perror( "xvfb launch" );
					exit(1);
				}
			} else if ( result == 0 ) {
				// timeout, things must be OK...
				break;
			} else if ( result < 0 ) {
				fprintf( stderr, "Oops, some error occurred... try again...%d...\n", getpid() );
				perror( "what was the problem... " );
				continue;
			}

		}

		if ( failure == false )
			break;

	}

	char cmd[1024];
	sprintf( cmd, "xauth -f %s add :%d . %s", authority, display_num, xauth );
	int status = system( cmd );
	if ( status != 0 ) {
		perror( "xauth setup" );
	}

	display = (char*) malloc( sizeof(char) * 50 );
	sprintf( display, "DISPLAY=localhost:%d.0", display_num );
	sprintf( authority, "XAUTHORITY=%s/.casa/xauthority", home );

#endif
	return child_xvfb;
}

