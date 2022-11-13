# Project Name
TARGET = Looper

USE_FATFS = 1

# Sources
CPP_SOURCES = Looper.cpp ../../lib/daisy_dpt.cpp ../../lib/dev/DAC7554.cpp

# Library Locations
LIBDAISY_DIR = ../../libDaisy/
DAISYSP_DIR = ../../DaisySP/

# LDFLAGS = -lc -lrdimon --specs=rdimon.specs

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
