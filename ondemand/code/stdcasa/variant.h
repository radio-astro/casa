#ifndef __casac_variant_h__
#define __casac_variant_h__

#include <string>
#include <vector>
#include <complex>

namespace casac {

class record;

class variant {

    public:

	enum TYPE { RECORD, BOOL, INT, LONG, DOUBLE, COMPLEX, STRING, BOOLVEC, INTVEC, LONGVEC, DOUBLEVEC, COMPLEXVEC, STRINGVEC };

	static TYPE compatible_type( TYPE one, TYPE two );

	class error {
	    public:
		error( std::string msg ) : message_(msg) { }
		const std::string &message( ) const { return message_; }
	    private:
		std::string message_;
	};

	class overflow : public error {
	    public:
		overflow( std::string lbl ) : error(lbl + ": overflow error") { }
	};

        variant *clone() const { return new variant(*this); }
        int compare(const variant*) const;

	variant( );
	variant(const variant &);

	variant(bool arg) : typev(BOOL), shape_(1,1) { val.b = arg;  }
	variant(int arg) : typev(INT), shape_(1,1) { val.i = arg; }
	variant(long long arg) : typev(LONG), shape_(1,1) { val.l = arg; }
	variant(double arg) : typev(DOUBLE), shape_(1,1) { val.d = arg; }
	variant(std::complex<double> arg) : typev(COMPLEX) { val.c = new std::complex<double>(arg); }
	variant(const char *arg) : typev(STRING), shape_(1,1)
			{ val.s = new std::string(arg); }
	variant(const std::string &arg) : typev(STRING), shape_(1,1)
			{ val.s = new std::string(arg); }
//
	variant(const std::vector<bool> &arg) : typev(BOOLVEC), shape_(1,arg.size())
			{ val.bv = new std::vector<bool>(arg); }
	variant(const std::vector<bool> &arg, const std::vector<int> &theshape) : typev(BOOLVEC), shape_(theshape)
			{ val.bv = new std::vector<bool>(arg); }
	variant(std::vector<bool> *arg) : typev(BOOLVEC), shape_(1,arg->size())
                        { val.bv = arg; }
	variant(std::vector<bool> *arg, std::vector<int> &theshape) : typev(BOOLVEC), shape_(theshape)
                        { val.bv = arg; }
//
	variant(const std::vector<int> &arg) : typev(INTVEC), shape_(1,arg.size())
			{ val.iv = new std::vector<int>(arg); }
	variant(const std::vector<int> &arg, const std::vector<int> &theshape) : typev(INTVEC), shape_(theshape)
			{ val.iv = new std::vector<int>(arg); }
	variant(std::vector<int> *arg) : typev(INTVEC), shape_(1, arg->size())
                        { val.iv = arg; }
	variant(std::vector<int> *arg, std::vector<int> &theshape) : typev(INTVEC), shape_(theshape)
			{ val.iv = arg; }
//
	variant(const std::vector<long long> &arg) : typev(LONGVEC), shape_(1,arg.size())
			{ val.lv = new std::vector<long long>(arg); }
	variant(const std::vector<long long> &arg, const std::vector<int> &theshape) : typev(LONGVEC), shape_(theshape)
			{ val.lv = new std::vector<long long>(arg); }
	variant(std::vector<long long> *arg) : typev(LONGVEC), shape_(1, arg->size())
                        { val.lv = arg; }
	variant(std::vector<long long> *arg, std::vector<int> &theshape) : typev(LONGVEC), shape_(theshape)
			{ val.lv = arg; }
//
	variant(const std::vector<double> &arg) : typev(DOUBLEVEC), shape_(1,arg.size())
			{ val.dv = new std::vector<double>(arg); }
	variant(const std::vector<double> &arg, const std::vector<int> &theshape) : typev(DOUBLEVEC), shape_(theshape)
			{ val.dv = new std::vector<double>(arg); }
	variant(std::vector<double> *arg) : typev(DOUBLEVEC), shape_(1,arg->size())
			{ val.dv = arg; }
	variant(std::vector<double> *arg, std::vector<int> &theshape) : typev(DOUBLEVEC), shape_(theshape)
			{ val.dv = arg; }

	variant(const std::vector<std::complex<double> > &arg) : typev(COMPLEXVEC), shape_(1, arg.size())
			{ val.cv = new std::vector<std::complex<double> >(arg); }
	variant(const std::vector<std::complex<double> > &arg, const std::vector<int> &theshape) : typev(COMPLEXVEC), shape_(theshape)
			{ val.cv = new std::vector<std::complex<double> >(arg); }
	variant(std::vector<std::complex<double> > *arg) : typev(COMPLEXVEC), shape_(1,arg->size())
                        { val.cv = arg; }
	variant(std::vector<std::complex<double> > *arg, std::vector<int> &theshape) : typev(COMPLEXVEC), shape_(theshape)
			{ val.cv = arg; }
//
	variant(const std::vector<std::string> &arg, const std::vector<int> &theshape) : typev(STRINGVEC), shape_(theshape)
			{ val.sv = new std::vector<std::string>(arg); }
	variant(const std::vector<std::string> &arg) : typev(STRINGVEC), shape_(1,arg.size())
			{ val.sv = new std::vector<std::string>(arg); }
	variant(std::vector<std::string> *arg) : typev(STRINGVEC), shape_(1, arg->size())
			{ val.sv = arg; }
	variant(std::vector<std::string> *arg, std::vector<int> &theshape) : typev(STRINGVEC), shape_(theshape)
			{ val.sv = arg; }
//
	variant(const record &arg);
	variant(record *arg);

	~variant( );

	bool toBool( ) const;
	int toInt( ) const;
	long long toLong( ) const;
	double toDouble( ) const;
	std::complex<double> toComplex( ) const;
	std::string toString( bool no_brackets=false ) const;
	std::vector<bool> toBoolVec( ) const;
	std::vector<int> toIntVec( ) const;
	std::vector<long long> toLongVec( ) const;
	std::vector<double> toDoubleVec( ) const;
	std::vector<std::complex<double> > toComplexVec( ) const;
	std::vector<std::string> toStringVec( ) const;

	// Yet to be implemented

//      Modify
//      ---------------------------------------------------
	bool &asBool( );
	int &asInt( );
	long long &asLong( );
	double &asDouble( );
	std::complex<double> &asComplex( );
	std::string &asString( );
	std::vector<int> &asIntVec( int size=-1 );
	std::vector<long long> &asLongVec( int size=-1 );
	std::vector<bool> &asBoolVec( int size=-1 );
	std::vector<double> &asDoubleVec( int size=-1 );
	std::vector<std::complex<double> > &asComplexVec( int size=-1 );
	std::vector<std::string> &asStringVec( int size=-1 );
	casac::record &asRecord( );

	void as( TYPE t, int size=-1 );

//      Const
//      ---------------------------------------------------
	const bool getBool( ) const throw(error);
	const int getInt( ) const  throw(error);
	const long long  getLong( ) const  throw(error);
	const double getDouble( ) const throw(error);
	const std::complex<double> &getComplex( ) const throw(error);
	const std::string &getString( ) const throw(error);
	const std::vector<int> &getIntVec( ) const throw(error);
	const std::vector<long long> &getLongVec( ) const throw(error);
	const std::vector<bool> &getBoolVec( ) const throw(error);
	const std::vector<double> &getDoubleVec( ) const throw(error);
	const std::vector<std::complex<double> > &getComplexVec( ) const throw(error);
	const std::vector<std::string> &getStringVec( ) const throw(error);
	const record &getRecord( ) const throw(error);
        const std::vector<int> &shape() const;
        const std::vector<int> &arrayshape() const {return shape();}

//	Modify
//      ---------------------------------------------------
	bool &getBoolMod( ) throw(error);
	int &getIntMod( ) throw(error);
	double &getDoubleMod( ) throw(error);
	std::complex<double> &getComplexMod( ) throw(error);
	std::string &getStringMod( ) throw(error);
	std::vector<int> &getIntVecMod( ) throw(error);
	std::vector<bool> &getBoolVecMod( ) throw(error);
	std::vector<double> &getDoubleVecMod( ) throw(error);
	std::vector<std::complex<double> > &getComplexVecMod( ) throw(error);
	std::vector<std::string> &getStringVecMod( ) throw(error);
	record &getRecordMod( ) throw(error);
        std::vector<int> &shape();
        std::vector<int> &arrayshape() {return shape();}

	const std::string &typeString( ) const;
	TYPE type( ) const { return typev; }

	void push(bool, bool conform = true);
	void push(int, bool conform = true);
	void push(long long, bool conform = true);
	void push(double, bool conform = true);
	void push(std::vector<long long>, bool conform = true);
	void push(std::complex<double>, bool conform = true);
	void push(const std::string&, bool conform = true);

	void place(bool, unsigned int index, bool conform = true);
	void place(int, unsigned int index, bool conform = true);
	void place(long long, unsigned int index, bool conform = true);
	void place(double, unsigned int index, bool conform = true);
	void place(std::vector<long long>, unsigned int index, bool conform = true);
	void place(std::complex<double>, unsigned int index, bool conform = true);
	void place(const std::string&, unsigned int index, bool conform = true);

	int size( ) const { return typev >= BOOLVEC ? vec_size() : 1; }
	void resize( int size );
	void dump() const;

    private:

	// what size does the shape imply
	int shape_size( ) const;

	// 4294967295
	static unsigned int record_id_count;

	int vec_size( ) const;
	TYPE typev;
	union {
	  bool b;
	  std::vector<bool> *bv;
	  int i;
	  long long l;
	  std::vector<int> *iv;
	  std::vector<long long> *lv;
	  double d;
	  std::vector<double> *dv;
	  std::complex<double> *c;
	  std::vector<std::complex<double> > *cv;
	  std::string *s;
	  std::vector<std::string> *sv;
	  record *recordv;
	} val;
        std::vector<int> shape_;

	std::string create_message( const std::string s ) const;
};

variant &initialize_variant( const std::string & );
}	// casac namespace

#endif
