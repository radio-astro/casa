#ifndef ASAPACCELERATOR_H
#define ASAPACCELERATOR_H
//
// C++ Interface: CustomTableExprNode
//
// Description:
//	Various utilities for speed.
//
// Author: Kohji Nakamura <k.nakamura@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <casa/Utilities/Assert.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ScalarColumn.h>

namespace asap {

using namespace casa;

class TableExprPredicate {
public:
	virtual ~TableExprPredicate() {}
	virtual Bool match(Table const& table, const TableExprId& id) = 0;
};

class CustomTableExprNodeRep :public TableExprNodeRep {
	TableExprPredicate & pred_;
public:
	CustomTableExprNodeRep(const Table &table,
		TableExprPredicate & pred)
	:TableExprNodeRep(TableExprNodeRep::NTBool,
		TableExprNodeRep::VTScalar,
		TableExprNodeRep::OtUndef,
		table),
	pred_(pred) {}
	virtual ~CustomTableExprNodeRep() {}
	virtual Bool getBool(const TableExprId& id) {
		return pred_.match(table(), id);
	}
};

class CustomTableExprNode: public TableExprNode {
public:
	CustomTableExprNode(CustomTableExprNodeRep &nodeRep)
	: TableExprNode(&nodeRep) {
	}
	virtual ~CustomTableExprNode() {
	}
};

template<typename T, size_t N>
class SingleTypeEqPredicate: public TableExprPredicate {
	Table const & table;
	ROScalarColumn<T> *cols[N];
	T const *values;
public:
	SingleTypeEqPredicate(Table const &table_, 
		char const*const colNames[],
		T const values_[]):
			table(table_), values(values_) {
		for (size_t i = 0; i < N; i++) {
			cols[i] = new ROScalarColumn<T>(table, colNames[i]);
		}
	}
	virtual ~SingleTypeEqPredicate() {
		for (size_t i = 0; i < N; i++) {
			delete cols[i];
		}
	}
	virtual Bool match(Table const& table, const TableExprId& id) {
		DebugAssert(&table == &this->table, AipsError);
		for (size_t i = 0; i < N; i++) {
			T v;
			cols[i]->get(id.rownr(), v);
			if (v != values[i]) {
				return false;
			}
		}
		return true;
	}
};

} // asap

#endif // ASAPACCELERATOR_H
