AM_DEFAULT_SOURCE_EXT = .cc

lib_LTLIBRARIES = libcasa_tables.la
libcasa_tables_la_SOURCES = \
        LogTables/LogFilterExpr.cc \
        LogTables/LogFilterTaql.cc \
        LogTables/LoggerHolder.cc \
        LogTables/NewFile.cc \
        LogTables/TableLogSink.cc \
        Tables/BaseColDesc.cc \
        Tables/BaseColumn.cc \
        Tables/BaseTabIter.cc \
        Tables/BaseTable.cc \
        Tables/BitFlagsEngine.cc \
        Tables/ColDescSet.cc \
        Tables/ColumnCache.cc \
        Tables/ColumnDesc.cc \
        Tables/ColumnSet.cc \
        Tables/ColumnsIndex.cc \
        Tables/ColumnsIndexArray.cc \
        Tables/CompressComplex.cc \
        Tables/CompressFloat.cc \
        Tables/ConcatColumn.cc \
        Tables/ConcatRows.cc \
        Tables/ConcatTable.cc \
        Tables/DataManAccessor.cc \
        Tables/DataManager.cc \
        Tables/DataManError.cc \
        Tables/ExprConeNode.cc \
        Tables/ExprDerNode.cc \
        Tables/ExprDerNodeArray.cc \
        Tables/ExprFuncNode.cc \
        Tables/ExprFuncNodeArray.cc \
        Tables/ExprLogicNode.cc \
        Tables/ExprLogicNodeArray.cc \
        Tables/ExprMathNode.cc \
        Tables/ExprMathNodeArray.cc \
        Tables/ExprNode.cc \
        Tables/ExprNodeArray.cc \
        Tables/ExprNodeRecord.cc \
        Tables/ExprNodeRep.cc \
        Tables/ExprNodeSet.cc \
        Tables/ExprRange.cc \
        Tables/ExprUnitNode.cc \
        Tables/ExternalLockSync.cc \
        Tables/ForwardCol.cc \
        Tables/ForwardColRow.cc \
        Tables/IncrementalStMan.cc \
        Tables/IncrStManAccessor.cc \
        Tables/ISMBase.cc \
        Tables/ISMBucket.cc \
        Tables/ISMColumn.cc \
        Tables/ISMIndColumn.cc \
        Tables/ISMIndex.cc \
        Tables/MemoryStMan.cc \
        Tables/MemoryTable.cc \
        Tables/MSMBase.cc \
        Tables/MSMColumn.cc \
        Tables/MSMDirColumn.cc \
        Tables/MSMIndColumn.cc \
        Tables/NullTable.cc \
        Tables/PlainColumn.cc \
        Tables/PlainTable.cc \
        Tables/ReadAsciiTable.cc \
        Tables/RecordExpr.cc \
        Tables/RecordGram.cc \
        Tables/RefColumn.cc \
        Tables/RefRows.cc \
        Tables/RefTable.cc \
        Tables/RowCopier.cc \
        Tables/ScaRecordColData.cc \
        Tables/ScaRecordColDesc.cc \
        Tables/SetupNewTab.cc \
        Tables/SSMBase.cc \
        Tables/SSMColumn.cc \
        Tables/SSMDirColumn.cc \
        Tables/SSMIndColumn.cc \
        Tables/SSMIndex.cc \
        Tables/SSMIndStringColumn.cc \
        Tables/SSMStringHandler.cc \
        Tables/StandardStMan.cc \
        Tables/StandardStManAccessor.cc \
        Tables/StArrAipsIO.cc \
        Tables/StArrayFile.cc \
        Tables/StIndArrAIO.cc \
        Tables/StIndArray.cc \
        Tables/StManAipsIO.cc \
        Tables/StManColumn.cc \
        Tables/SubTabDesc.cc \
        Tables/Table.cc \
        Tables/TableAttr.cc \
        Tables/TableCache.cc \
        Tables/TableColumn.cc \
        Tables/TableCopy.cc \
        Tables/TableDesc.cc \
        Tables/TableError.cc \
        Tables/TableExprData.cc \
        Tables/TableExprId.cc \
        Tables/TableGram.cc \
        Tables/TableIndexProxy.cc \
        Tables/TableInfo.cc \
        Tables/TableIter.cc \
        Tables/TableIterProxy.cc \
        Tables/TableKeyword.cc \
        Tables/TableLock.cc \
        Tables/TableLockData.cc \
        Tables/TableLocker.cc \
        Tables/TableParse.cc \
        Tables/TableProxy.cc \
        Tables/TableRecord.cc \
        Tables/TableRecordRep.cc \
        Tables/TableRow.cc \
        Tables/TableRowProxy.cc \
        Tables/TableSyncData.cc \
        Tables/TabPath.cc \
        Tables/TaQLNode.cc \
        Tables/TaQLNodeDer.cc \
        Tables/TaQLNodeHandler.cc \
        Tables/TaQLNodeRep.cc \
        Tables/TaQLNodeResult.cc \
        Tables/TaQLNodeVisitor.cc \
        Tables/TaQLResult.cc \
        Tables/TaQLStyle.cc \
        Tables/TiledCellStMan.cc \
        Tables/TiledColumnStMan.cc \
        Tables/TiledDataStMan.cc \
        Tables/TiledDataStManAccessor.cc \
        Tables/TiledFileAccess.cc \
        Tables/TiledFileHelper.cc \
        Tables/TiledShapeStMan.cc \
        Tables/TiledStMan.cc \
        Tables/TiledStManAccessor.cc \
        Tables/TSMColumn.cc \
        Tables/TSMCoordColumn.cc \
        Tables/TSMCube.cc \
        Tables/TSMDataColumn.cc \
        Tables/TSMFile.cc \
        Tables/TSMIdColumn.cc \
        Tables/TSMShape.cc \
        Tables/VirtColEng.cc \
        Tables/VirtualTaQLColumn.cc 

# Dependencies for libcasa_tables
AM_CPPFLAGS = -DCASA_USECASAPATH -DCASACORE_NEEDS_RETHROW \
        -I.. \
        -I$(top_srcdir)/casa

libcasa_tables_la_LIBADD = \
        $(top_builddir)/casa/casa/libcasa_casa.la

libcasa_tables_la_DEPENDENCIES = \
        $(top_builddir)/casa/casa/libcasa_casa.la

SUBDIRS = Tables .



check_PROGRAMS = \
        LogTables/test/dLogging \
        LogTables/test/dLogging2 \
        LogTables/test/tLoggerHolder \
        LogTables/test/tLogging \
        Tables/test/ascii2Table \
        Tables/test/dRetypedArrayEngine \
        Tables/test/dVirtColEng \
        Tables/test/dVSCEngine \
        Tables/test/nISMBucket \
        Tables/test/tArrayColumnSlices \
        Tables/test/tBitFlagsEngine \
        Tables/test/tColumnsIndex \
        Tables/test/tColumnsIndexArray \
        Tables/test/tCompressComplex \
        Tables/test/tCompressFloat \
        Tables/test/tConcatRows \
        Tables/test/tConcatTable \
        Tables/test/tConcatTable2 \
        Tables/test/tExprNode \
        Tables/test/tExprNodeSet \
        Tables/test/tExprUnitNode \
        Tables/test/tForwardCol \
        Tables/test/tForwardColRow \
        Tables/test/tIncrementalStMan \
        Tables/test/tMappedArrayEngine \
        Tables/test/tMemoryStMan \
        Tables/test/tMemoryTable \
        Tables/test/tReadAsciiTable \
        Tables/test/tReadAsciiTable2 \
        Tables/test/tRecordExpr \
        Tables/test/tRecordGram \
        Tables/test/tRecordGramTable \
        Tables/test/tRefRows \
        Tables/test/tRowCopier \
        Tables/test/tScalarRecordColumn \
        Tables/test/tScaledArrayEngine \
        Tables/test/tScaledComplexData \
        Tables/test/tSSMStringHandler \
        Tables/test/tStandardStMan \
        Tables/test/tStArrayFile \
        Tables/test/tStMan \
        Tables/test/tStMan1 \
        Tables/test/tTable \
        Tables/test/tTable_1 \
        Tables/test/tTable_2 \
        Tables/test/tTable_3 \
        Tables/test/tTable_4 \
        Tables/test/tTableCopy \
        Tables/test/tTableDesc \
        Tables/test/tTableDescHyper \
        Tables/test/tTableExprData \
        Tables/test/tTableGram \
        Tables/test/tTableInfo \
        Tables/test/tTableIter \
        Tables/test/tTableKeywords \
        Tables/test/tTableLock \
        Tables/test/tTableLockSync \
        Tables/test/tTableLockSync_2 \
        Tables/test/tTableRecord \
        Tables/test/tTableRow \
        Tables/test/tTableVector \
        Tables/test/tTaQLNode \
        Tables/test/tTiledCellStM_1 \
        Tables/test/tTiledCellStMan \
        Tables/test/tTiledColumnStMan \
        Tables/test/tTiledDataStM_1 \
        Tables/test/tTiledDataStMan \
        Tables/test/tTiledFileAccess \
        Tables/test/tTiledShapeStM_1 \
        Tables/test/tTiledShapeStMan \
        Tables/test/tTiledStMan \
        Tables/test/tTSMShape \
        Tables/test/tVirtColEng \
        Tables/test/tVirtualTaQLColumn \
        Tables/test/tVSCEngine 

TESTS = $(check_PROGRAMS)
LDADD = libcasa_tables.la
