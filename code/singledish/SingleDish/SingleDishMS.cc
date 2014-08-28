#include <iostream>

#include <libsakura/sakura.h>
//#include <libsakura/config.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Utilities/Assert.h>

#include <ms/MeasurementSets/MSSelectionTools.h>

#include <casa_sakura/SakuraUtils.h>
#include <singledish/SingleDish/SingleDishMS.h>

namespace casa {

SingleDishMS::SingleDishMS()
  : msname_(""), ms_(0), mssel_(0)
{
}


SingleDishMS::SingleDishMS(string const& ms_name)
  : msname_(ms_name), ms_(0), mssel_(0)
{
  // Make a MeasurementSet object for the disk-base MeasurementSet
  String lems(ms_name);
  ms_ = new MeasurementSet(lems, TableLock(TableLock::AutoNoReadLocking), 
			    Table::Update);
  AlwaysAssert(ms_, AipsError);
}


SingleDishMS::SingleDishMS(MeasurementSet &ms)
  : msname_(""), ms_(0), mssel_(0)
{
  msname_ = static_cast<string>(ms.tableName());
  ms_ = new MeasurementSet(ms);
  AlwaysAssert(ms_, AipsError);
}

SingleDishMS::SingleDishMS(SingleDishMS const &other)
  : msname_(""), ms_(0)
{
  ms_ = new MeasurementSet(*other.ms_);
  if(other.mssel_) {
    mssel_ = new MeasurementSet(*other.mssel_);
  }
}

SingleDishMS &SingleDishMS::operator=(SingleDishMS const &other)
{
  msname_ = "";
  if (ms_ && this != &other) {
    *ms_ = *(other.ms_);
  }
  if (mssel_ && this != &other && other.mssel_) {
    *mssel_ = *(other.mssel_);
  }
  return *this;
}

SingleDishMS::~SingleDishMS()
{
  if (ms_) {
    ms_->relinquishAutoLocks();
    ms_->unlock();
    delete ms_;
  }
  ms_ = 0;
  if (mssel_) {
    mssel_->relinquishAutoLocks();
    mssel_->unlock();
    delete mssel_;
  }
  mssel_ = 0;
  msname_ = "";
}

void SingleDishMS::check_MS()
{
  AlwaysAssert(ms_, AipsError);
}

bool SingleDishMS::close()
{
  LogIO os(LogOrigin("SingleDishMS", "close()", WHERE));
  os << "Closing MeasurementSet and detaching from SingleDishMS"
     << LogIO::POST;

  ms_->unlock();
  if(mssel_) {
    mssel_->unlock();
    delete mssel_;
    mssel_ = 0;
  }
  if(ms_) delete ms_; ms_ = 0;
  msname_ = "";

  return True;
}

void SingleDishMS::scale(double const factor)
{
  check_MS();
  cout << "Got scaling factor = " << factor << endl;
}

void SingleDishMS::set_selection(Record const &selection)
{
  check_MS();
  reset_selection();

  //Parse selection
  String timeExpr(""), antennaExpr(""), fieldExpr(""),
    spwExpr(""), uvDistExpr(""), taQLExpr(""), polnExpr(""),
    scanExpr(""), arrayExpr(""), stateExpr(""), obsExpr("");
  timeExpr = get_field_as_casa_string(selection,"time");
  antennaExpr = get_field_as_casa_string(selection,"baseline");
  fieldExpr = get_field_as_casa_string(selection,"field");
  spwExpr = get_field_as_casa_string(selection,"spw");
  uvDistExpr = get_field_as_casa_string(selection,"uvdist");
  taQLExpr = get_field_as_casa_string(selection,"taql");
  polnExpr = get_field_as_casa_string(selection,"polarization");
  scanExpr = get_field_as_casa_string(selection,"scan");
  arrayExpr = get_field_as_casa_string(selection,"array");
  stateExpr = get_field_as_casa_string(selection,"state");
  obsExpr = get_field_as_casa_string(selection,"observation");
  //Now the actual selection.
  mssel_ = new MeasurementSet(*ms_);
  if (!mssSetData(*ms_,*mssel_,"",timeExpr,antennaExpr,fieldExpr,
		  spwExpr,uvDistExpr,taQLExpr,polnExpr,scanExpr,
		  arrayExpr,stateExpr,obsExpr)) { // no valid selection
    reset_selection();
    cout << "Reset selection" << endl;
  }
  if (mssel_!=0)
    cout << "Selected nrows = " << mssel_->nrow() << " from " << ms_->nrow() << "rows" << endl;
}

String SingleDishMS::get_field_as_casa_string(Record const &in_data, string const &field_name){
  Int ifield;
  ifield = in_data.fieldNumber(String(field_name));
  if (ifield>-1) return in_data.asString(ifield);
  return "";
}

void SingleDishMS::reset_selection()
{
  if (mssel_) {
    delete mssel_;
    mssel_=0;
  };
}

}  // End of casa namespace.
