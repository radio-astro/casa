#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>
#include <casa/BasicSL/String.h>
    
namespace casa { //# NAMESPACE CASA - BEGIN

static String fortranModule,fortranErr;
static jmp_buf fortran_jmp;
// reportFortranError
// // This is the common error handler.
static void reportFortranError ( const String &message = String("") )
{
   String err("FORTRAN module");
   if( fortranModule.length() )
   {
      err+=" [";
      err+=fortranModule;
      err+="]";
   }
   err+=": ";
   err+=message;
// It would be nice to throw an exception at this point, but apparently
// exceptions don't work after FORTRAN (that is, if C++ calls FORTRAN, 
// and FORTRAN does a STOP, which calls this procedure, then exceptions thrown
// here are not caught). So instead of a nice and clean: 
//    throw( AipsError(err) );
// we have to do the ugly:
   fortranErr = err;
   longjmp(fortran_jmp,1);
}

// Error handler for the replaced stop statements in the FORTRAN PIM code

extern "C" {
void a2stop_( char *message,int len ) // STOP statement
{
   fprintf(stderr,"Caught STOP in FORTRAN [%s]\n",fortranModule.chars());
   String err("STOP");
   if( len )
   {
      char *msg = new char[len+1];
      strncpy(msg,message,len);
      msg[len+1]=0;
      err+=" (";
      err+=msg;
      err+=" )";
      fprintf(stderr,"STOP '%s'\n",msg);
   }
   reportFortranError(err);
}

}

} //# NAMESPACE CASA - END

