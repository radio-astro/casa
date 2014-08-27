#include <iostream>

#include <libsakura/sakura.h>
//#include <libsakura/config.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Utilities/Assert.h>

#include <casa_sakura/SakuraUtils.h>
#include <singledish/SingleDish/SingleDishMS.h>

namespace casa {

SingleDishMS::SingleDishMS()
  : msname_p(""), ms_p(0), mssel_p(0)
{
}


SingleDishMS::SingleDishMS(const string& ms_name)
  : msname_p(ms_name), ms_p(0), mssel_p(0)
{
  // Make a MeasurementSet object for the disk-base MeasurementSet
  String lems(ms_name);
  ms_p = new MeasurementSet(lems, TableLock(TableLock::AutoNoReadLocking), 
			    Table::Update);
  AlwaysAssert(ms_p, AipsError);
}


SingleDishMS::SingleDishMS(MeasurementSet &ms)
  : msname_p(""), ms_p(0), mssel_p(0)
{
  msname_p = static_cast<string>(ms.tableName());

  ms_p = new MeasurementSet(ms);
  AlwaysAssert(ms_p, AipsError);
}

SingleDishMS::SingleDishMS(const SingleDishMS &other)
  : msname_p(""), ms_p(0)
{
  ms_p = new MeasurementSet(*other.ms_p);
  if(other.mssel_p) {
    mssel_p = new MeasurementSet(*other.mssel_p);
  }
}

SingleDishMS &SingleDishMS::operator=(const SingleDishMS &other)
{
  msname_p = "";
  if (ms_p && this != &other) {
    *ms_p = *(other.ms_p);
  }
  if (mssel_p && this != &other && other.mssel_p) {
    *mssel_p = *(other.mssel_p);
  }
  return *this;
}

SingleDishMS::~SingleDishMS()
{
  if (ms_p) {
    ms_p->relinquishAutoLocks();
    ms_p->unlock();
    delete ms_p;
  }
  ms_p = 0;
  if (mssel_p) {
    mssel_p->relinquishAutoLocks();
    mssel_p->unlock();
    delete mssel_p;
  }
  mssel_p = 0;
  msname_p = "";
}

// string SingleDishMS::name() const
// {
//   return msname_p;
// }

bool SingleDishMS::close()
{
  LogIO os(LogOrigin("SingleDishMS", "close()", WHERE));
  os << "Closing MeasurementSet and detaching from SingleDishMS"
     << LogIO::POST;

  ms_p->unlock();
  if(mssel_p) mssel_p->unlock();
  if(mssel_p) delete mssel_p; mssel_p = 0;
  if(ms_p) delete ms_p; ms_p = 0;
  msname_p = "";

  return True;
}

}  // End of casa namespace.
