#define NO_INITIALIZE_STATICS 1
#include <imagemetadata_cmpt.h>
#undef NO_INITIALIZE_STATICS
#include <stdcasa/StdCasa/CasacSupport.h>

#include <casa/namespace.h>
#include <memory>

namespace casa 
{
	class LogIO;
	template<class T> class ImageMetaDataRW;
	template<class T> class ImageInterface;
}
