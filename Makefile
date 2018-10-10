# Build mode
# 0: development (max safety, no optimisation)
# 1: release (min safety, optimisation)
# 2: fast and furious (no safety, optimisation)
BUILD_MODE?=0

all: main
	
# Makefile definitions
MAKEFILE_INC=../PBMake/Makefile.inc
include $(MAKEFILE_INC)

# Path to the model implementation
MF_MODEL_PATH=$(ROOT_DIR)/MiniFrame/Examples/BasicExample

# Rules to make the executable
repo=miniframe
$($(repo)_EXENAME): \
		createLinkToModelHeader \
		miniframe-model.o \
		$($(repo)_EXENAME).o \
		$($(repo)_EXE_DEP) \
		$($(repo)_DEP)
	$(COMPILER) `echo "$($(repo)_EXE_DEP) $($(repo)_EXENAME).o" | tr ' ' '\n' | sort -u` $(MF_MODEL_PATH)/miniframe-model.o $(LINK_ARG) $($(repo)_LINK_ARG) -o $($(repo)_EXENAME) 
	
$($(repo)_EXENAME).o: \
		$(MF_MODEL_PATH)/miniframe-model.h \
		$($(repo)_DIR)/$($(repo)_EXENAME).c \
		$($(repo)_INC_H_EXE) \
		$($(repo)_EXE_DEP)
	$(COMPILER) $(BUILD_ARG) $($(repo)_BUILD_ARG) `echo "$($(repo)_INC_DIR)" | tr ' ' '\n' | sort -u` -c $($(repo)_DIR)/$($(repo)_EXENAME).c; rm $($(repo)_DIR)/miniframe-model.h; rm $($(repo)_DIR)/miniframe-inline-model.c

createLinkToModelHeader:
		ln -s -f $(MF_MODEL_PATH)/miniframe-model.h $($(repo)_DIR)/miniframe-model.h; ln -s -f $(MF_MODEL_PATH)/miniframe-inline-model.c $($(repo)_DIR)/miniframe-inline-model.c
		
miniframe-model.o: \
	$(MF_MODEL_PATH)/miniframe-model.h \
	$(MF_MODEL_PATH)/miniframe-model.c \
	Makefile
	$(COMPILER) $(BUILD_ARG) -c $(MF_MODEL_PATH)/miniframe-model.c

