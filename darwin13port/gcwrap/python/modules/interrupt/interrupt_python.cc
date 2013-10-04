#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#if defined(__APPLE__)
typedef sig_t signalhandler_type;
#else
typedef sighandler_t signalhandler_type;
#endif

extern "C" {
    void interrupt_init_priv( bool forward );
    void interrupt_reset_priv( );
    bool interrupt_cont_priv( );
    void interrupt_on_priv( );
    void interrupt_off_priv( );
};

static PyObject *InterruptError;

static signalhandler_type original_sigint_handler = 0;
static bool interrupt_has_occurred = false;
static bool interrupt_initialized = false;
static bool interrupt_on = false;
static bool interrupt_forward_sigint = false;

void interrupt_sigint_handler( int sig ) {

    interrupt_has_occurred = true;

    if ( interrupt_forward_sigint &&
	 original_sigint_handler != 0 &&
	 original_sigint_handler != SIG_IGN &&
	 original_sigint_handler != SIG_DFL &&
	 original_sigint_handler != SIG_ERR )
	(*original_sigint_handler)(sig);

    // reset the signal handler, for next time
    original_sigint_handler = signal(SIGINT,interrupt_sigint_handler);
}

void interrupt_init_priv( bool forward ) {
    if ( interrupt_initialized == false ) {
	interrupt_initialized = true;
	interrupt_on = true;
	original_sigint_handler = signal(SIGINT,interrupt_sigint_handler);
	interrupt_forward_sigint = forward;
    } else if ( interrupt_on ) {
	signalhandler_type handler = signal(SIGINT,interrupt_sigint_handler);
	if ( handler != interrupt_sigint_handler )
	    original_sigint_handler = handler;
    }
    interrupt_has_occurred = false;
}

static PyObject *interrupt_init_(PyObject *self, PyObject *args) {
    int forward = 1;

    PyArg_ParseTuple( args, "|i", &forward );
    interrupt_init_priv(forward != 0 ? true : false);
    Py_INCREF(Py_None);
    return Py_None;
}

void interrupt_reset_priv( ) {
    // reset() is a noop unless interrupt handling has been
    // initialized. This allows reset() & cont() to be
    // called whether SIGINT handling has been overriden or not.
    if ( interrupt_initialized == true ) {
	interrupt_init_priv( interrupt_forward_sigint );
    }
}

static PyObject *interrupt_reset_(PyObject *self, PyObject *args) {
    interrupt_reset_priv( );
    Py_INCREF(Py_None);
    return Py_None;
}

bool interrupt_cont_priv( ) {
    return interrupt_initialized ? (interrupt_has_occurred ? false : true) : true;
}

static PyObject *interrupt_cont_(PyObject *self, PyObject *args) {
    if ( interrupt_cont_priv( ) ) {
	Py_INCREF(Py_True);
	return Py_True;
    } else {
	Py_INCREF(Py_False);
	return Py_False;
    }
}

void interrupt_off_priv( ) {
    if ( interrupt_initialized == true && interrupt_on == true ) {
	signal(SIGINT,original_sigint_handler);
	interrupt_on = false;
	original_sigint_handler = 0;
	interrupt_has_occurred = false;
    }
}

static PyObject *interrupt_off_(PyObject *self, PyObject *args) {
    interrupt_off_priv( );
    Py_INCREF(Py_None);
    return Py_None;
}

void interrupt_on_priv( ) {
          
    if ( interrupt_initialized == true && interrupt_on == false ) {
        original_sigint_handler = signal(SIGINT,interrupt_sigint_handler);
	interrupt_on = true;
	interrupt_has_occurred = false;
    }
}

static PyObject *interrupt_on_(PyObject *self, PyObject *args) {
    interrupt_on_priv( );
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *interrupt_sleep_(PyObject *self, PyObject *args) {
    sleep(30);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef InterruptMethods[] = {
  { "init", interrupt_init_, METH_VARARGS,
    "initialize interrupt handling" },
  { "reset", interrupt_reset_, METH_NOARGS,
    "reset interrupt flag" },
  { "cont", interrupt_cont_, METH_NOARGS,
    "returns false when interrupt has occurred" },
  { "off", interrupt_off_, METH_NOARGS,
    "turn interrupt processing off" },
  { "on", interrupt_on_, METH_NOARGS,
    "turn interrupt processing on" },
  { "sleep", interrupt_sleep_, METH_NOARGS,
    "for debugging sleep" },
  { NULL, NULL, 0, NULL }
};

PyMODINIT_FUNC initinterrupt( ) {
    PyObject *m;
    m = Py_InitModule("interrupt", InterruptMethods);
    InterruptError = PyErr_NewException("interrupt.error", NULL, NULL);
    Py_INCREF(InterruptError);
    PyModule_AddObject(m, "error", InterruptError);
}
