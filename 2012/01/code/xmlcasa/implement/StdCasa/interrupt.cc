#include <dlfcn.h>
#include <Python.h>
#include <xmlcasa/interrupt.h>

static bool interrupt_initialized = false;
static void (*interrupt_init_p)(bool) = 0;
static void (*interrupt_reset_p)( ) = 0;
static bool (*interrupt_cont_p)( ) = 0;
static void (*interrupt_off_p)( ) = 0;
static void (*interrupt_on_p)( ) = 0;


sigjmp_buf casa::jmp_buf;
bool casa::longjmp_ok = false;

static void _casa_signal_handler_(int sig) {
    if ( casa::longjmp_ok ) {
	siglongjmp( casa::jmp_buf, 1 );
    }
}

void (*casa::signal_handler)(int) = &_casa_signal_handler_;

bool casa::interrupt::init( bool forward ) {
    if ( interrupt_initialized == false ) {

	Py_Initialize( );
	PyObject *mod = PyImport_ImportModule("interrupt");
	if ( ! mod ) return false;
	char *modname = PyModule_GetFilename(mod);
	if ( ! modname ) return false;
	void *dlhandle = dlopen(modname,RTLD_NOW);
	if ( ! dlhandle ) return false;
	interrupt_init_p = (void (*)(bool)) dlsym( dlhandle, "interrupt_init_priv" );
	if ( ! interrupt_init_p ) return false;
	interrupt_reset_p = (void (*)()) dlsym( dlhandle, "interrupt_reset_priv" );
	if ( ! interrupt_reset_p ) return false;
	interrupt_cont_p = (bool (*)()) dlsym( dlhandle, "interrupt_cont_priv" );
	if ( ! interrupt_cont_p ) return false;
	interrupt_off_p = (void (*)()) dlsym( dlhandle, "interrupt_off_priv" );
	if ( ! interrupt_off_p ) return false;
	interrupt_on_p = (void (*)()) dlsym( dlhandle, "interrupt_on_priv" );
	if ( ! interrupt_on_p ) return false;

	//
	// dlopen calls should be balanced with dlclose calls...
	// we should be able to call dlclose here because python
	// should still be holding a dl handle to the interrupt module.
	//
 	dlclose(dlhandle);

	(*interrupt_init_p)(forward);
	interrupt_initialized = true;
    }

    return true;
}


void casa::interrupt::reset( ) {
    if ( interrupt_reset_p ) (*interrupt_reset_p)( );
}

bool casa::interrupt::cont( ) {
    if ( interrupt_cont_p )
	return (*interrupt_cont_p)( );
    return true;
}

void casa::interrupt::off( ) {
    if ( interrupt_off_p ) (*interrupt_off_p)( );
}

void casa::interrupt::on( ) {
    if ( interrupt_on_p ) (*interrupt_on_p)( );
}

