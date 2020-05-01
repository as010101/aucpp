# This file is to be included in the top-level Makefile.
# This file is owned by the integration lead programmer, but *tentative* changes
# may be made by others.

PREDPARSER_LIB_OBJECTS= \
	$(PREDPARSER_BASE_DIR)/src/AggregateState.o \
	$(PREDPARSER_BASE_DIR)/src/BufferList.o \
	$(PREDPARSER_BASE_DIR)/src/GroupByBox.o \
	$(PREDPARSER_BASE_DIR)/src/GroupByHash.o \
	$(PREDPARSER_BASE_DIR)/src/GroupByState.o \
	$(PREDPARSER_BASE_DIR)/src/Hash.o \
	$(PREDPARSER_BASE_DIR)/src/HashForAggregate.o \
	$(PREDPARSER_BASE_DIR)/src/HashForBufferList.o \
	$(PREDPARSER_BASE_DIR)/src/HashForNewState.o \
	$(PREDPARSER_BASE_DIR)/src/HashWithList.o \
	$(PREDPARSER_BASE_DIR)/src/List.o \
	$(PREDPARSER_BASE_DIR)/src/NewState.o \
	$(PREDPARSER_BASE_DIR)/src/Parse.o \
	$(PREDPARSER_BASE_DIR)/src/SortList.o \
	$(PREDPARSER_BASE_DIR)/src/State.o \
	$(PREDPARSER_BASE_DIR)/src/TrashHash.o \
	$(PREDPARSER_BASE_DIR)/src/TrashState.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/ANDPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/AverageAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/CountAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FieldExt.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/Float2Int.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatAbsoluteFunction.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatAddFunction.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatAverageAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatConstant.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatDeltaAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatDivideFunction.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatEqualPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatFirstValueAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatGTEPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatGTPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatLTEPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatLTPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatLastValueAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatMaxAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatMinAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatMultiplyFunction.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatNEPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatSubtractFunction.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FloatSumAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/Int2Float.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/IntAbsoluteFunction.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/IntAddFunction.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/IntAverageAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/IntConstant.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/IntDivideFunction.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/IntEqualPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/IntGTEPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/IntGTPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/IntLTEPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/IntLTPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/IntLastValueAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/IntMaxAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/IntMinAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/IntMultiplyFunction.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/IntNEPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/IntSubtractFunction.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/IntSumAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/MaxAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/MinAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/Mitre1AF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/Mitre2AF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/MitreCOMAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/NOTPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/ORPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/StringConstant.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/StringEqualPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/StringGTEPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/StringGTPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/StringLTEPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/StringLTPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/StringNEPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/SumAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/TsConstant.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/TsEqualPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/TsGTEPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/TsGTPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/TsLTEPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/TsLTPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/TsNEPredicate.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/Fidelity1AF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/FidelityAlarmAF.o \
	$(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/Seg1AF.o \
        $(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/Seg2AF.o \
        $(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/Seg3AF.o \
        $(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/Acc1AF.o \
        $(PREDPARSER_BASE_DIR)/src/PredicatesAndExpressions/Acc2AF.o

ALL_CXX_OBJECTS += \
	$(PREDPARSER_LIB_OBJECTS)

$(PREDPARSER_LIB_SO): \
	$(PREDPARSER_LIB_OBJECTS)
	$(CC) -fPIC -shared -o $@ \
	$(PREDPARSER_LIB_OBJECTS)

clean_predparser_lib:
	rm -f $(PREDPARSER_LIB_OBJECTS)
	rm -f $(PREDPARSER_LIB_SO)
