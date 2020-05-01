# This file is to be included in the top-level Makefile.
# This file is owned by the integration lead programmer, but *tentative* changes
# may be made by others.

SCHED_LIB_OBJECTS = \
	$(SCHED_BASE_DIR)/src/AggregateQBox.o \
	$(SCHED_BASE_DIR)/src/BitShit.o \
	$(SCHED_BASE_DIR)/src/BoxExecutionQueue.o \
	$(SCHED_BASE_DIR)/src/BSortQBox.o \
	$(SCHED_BASE_DIR)/src/Catalog.o \
	$(SCHED_BASE_DIR)/src/check_ip.o  \
	$(SCHED_BASE_DIR)/src/DropQBox.o \
	$(SCHED_BASE_DIR)/src/ExperimentQBox.o \
	$(SCHED_BASE_DIR)/src/FilterQBox.o \
	$(SCHED_BASE_DIR)/src/HelloWorldQBox.o \
	$(SCHED_BASE_DIR)/src/global.o \
	$(SCHED_BASE_DIR)/src/JoinQBox.o \
	$(SCHED_BASE_DIR)/src/LRReadRelationQBox.o \
	$(SCHED_BASE_DIR)/src/LRUpdateRelationQBox.o \
	$(SCHED_BASE_DIR)/src/MapQBox.o \
	$(SCHED_BASE_DIR)/src/Measurement.o \
	$(SCHED_BASE_DIR)/src/OldPredicate.o \
	$(SCHED_BASE_DIR)/src/PriorityGrid.o \
	$(SCHED_BASE_DIR)/src/QBox.o \
	$(SCHED_BASE_DIR)/src/QoSMonitor.o \
	$(SCHED_BASE_DIR)/src/QueueMon.o \
	$(SCHED_BASE_DIR)/src/ReadRelationQBox.o \
	$(SCHED_BASE_DIR)/src/ResampleQBox.o \
	$(SCHED_BASE_DIR)/src/RestreamQBox.o \
	$(SCHED_BASE_DIR)/src/Scheduler.o \
	$(SCHED_BASE_DIR)/src/sem_util.o \
	$(SCHED_BASE_DIR)/src/Shared.o \
	$(SCHED_BASE_DIR)/src/shm_util.o \
	$(SCHED_BASE_DIR)/src/SMInterface.o \
	$(SCHED_BASE_DIR)/src/StreamThread.o \
	$(SCHED_BASE_DIR)/src/SysMon.o \
	$(SCHED_BASE_DIR)/src/TupleDescription.o \
	$(SCHED_BASE_DIR)/src/tupleGenerator.o \
	$(SCHED_BASE_DIR)/src/UnionQBox.o \
	$(SCHED_BASE_DIR)/src/UpdateRelationQBox.o \
	$(SCHED_BASE_DIR)/src/WorkerThread.o

ALL_CXX_OBJECTS += \
	$(SCHED_LIB_OBJECTS)

$(SCHED_LIB_SO): \
	$(SCHED_LIB_OBJECTS)
	$(CXX) -fPIC -shared -o $@ \
	$(SCHED_LIB_OBJECTS)

clean_sched_lib:
	rm -f $(SCHED_LIB_OBJECTS)
	rm -f $(SCHED_LIB_SO)
