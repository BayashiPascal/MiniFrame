# Build mode
# 0: development (max safety, no optimisation)
# 1: release (min safety, optimisation)
# 2: fast and furious (no safety, optimisation)
BUILD_MODE?=1

all: main
	
# Makefile definitions
MAKEFILE_INC=../../../PBMake/Makefile.inc
include $(MAKEFILE_INC)

# Path to the model implementation
MF_MODEL_PATH=$(ROOT_DIR)/MiniFrame/Examples/Oware

# Rules to make the executable
main: \
		createLinkToModelHeader \
		main.o \
		miniframe-model.o \
		$(miniframe_EXE_DEP) \
		$(miniframe_DEP)
	$(COMPILER) `echo "$(miniframe_EXE_DEP) main.o" | tr ' ' '\n' | sort -u` miniframe-model.o $(LINK_ARG) $(miniframe_LINK_ARG) -o main; rm $(miniframe_DIR)/miniframe-model.h; rm $(miniframe_DIR)/miniframe-inline-model.c
	
main.o: \
		main.c \
		$(miniframe_INC_H_EXE) \
		$(miniframe_EXE_DEP)
	$(COMPILER) $(BUILD_ARG) $(miniframe_BUILD_ARG) `echo "$(miniframe_INC_DIR)" | tr ' ' '\n' | sort -u` -c main.c
	
createLinkToModelHeader:
		ln -s -f $(MF_MODEL_PATH)/miniframe-model.h $(miniframe_DIR)/miniframe-model.h; ln -s -f $(MF_MODEL_PATH)/miniframe-inline-model.c $(miniframe_DIR)/miniframe-inline-model.c

miniframe-model.o: miniframe-model.h miniframe-model.c Makefile
	$(COMPILER) $(BUILD_ARG) -c miniframe-model.c

