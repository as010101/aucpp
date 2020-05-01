# This file is to be included in the top-level Makefile.
# This file is owned by the integration lead programmer, but *tentative* changes
# may be made by others.

UTIL_LIB_OBJECTS= \
	$(UTIL_BASE_DIR)/src/BinarySem.o \
	$(UTIL_BASE_DIR)/src/CountingSem.o \
	$(UTIL_BASE_DIR)/src/CountingSemHolder.o \
	$(UTIL_BASE_DIR)/src/FdMultiPool.o \
	$(UTIL_BASE_DIR)/src/FdMultiPoolLease.o \
	$(UTIL_BASE_DIR)/src/FairMutexWithCancel.o \
	$(UTIL_BASE_DIR)/src/FifoCriticalSection.o \
	$(UTIL_BASE_DIR)/src/FileDescHolder.o \
	$(UTIL_BASE_DIR)/src/FileDescPool.o \
	$(UTIL_BASE_DIR)/src/GlobalPropsFile.o \
	$(UTIL_BASE_DIR)/src/IntegerCounter.o \
	$(UTIL_BASE_DIR)/src/IntegerCounterHolder.o \
	$(UTIL_BASE_DIR)/src/LockHolder.o \
	$(UTIL_BASE_DIR)/src/PtCondition.o \
	$(UTIL_BASE_DIR)/src/PtMutex.o \
	$(UTIL_BASE_DIR)/src/PtThreadPool.o \
	$(UTIL_BASE_DIR)/src/Runnable.o \
	$(UTIL_BASE_DIR)/src/RunnableRunner.o \
	$(UTIL_BASE_DIR)/src/stringutil.o \
	$(UTIL_BASE_DIR)/src/SerializableBitSet.o \
	$(UTIL_BASE_DIR)/src/StorageMgr_Exceptions.o \
	$(UTIL_BASE_DIR)/src/TraceLogger.o \
	$(UTIL_BASE_DIR)/src/util.o \
	$(UTIL_BASE_DIR)/src/xercesDomUtil.o \
	$(UTIL_BASE_DIR)/src/XmlTempString.o \
	$(UTIL_BASE_DIR)/src/parseutil.o \
	$(UTIL_BASE_DIR)/src/PropsFile.o

ALL_CXX_OBJECTS += $(UTIL_LIB_OBJECTS)

$(UTIL_LIB_SO): \
	$(UTIL_LIB_OBJECTS)
	$(CXX) -fPIC -shared -o $@ \
	$(UTIL_LIB_OBJECTS)

clean_util_lib:
	rm -f $(UTIL_LIB_OBJECTS)
	rm -f $(UTIL_LIB_SO)
