# 
# Licensed under the Apache License, Version 2.0 (the "License"); 
# you may not use this file except in compliance with the License.  
# You may obtain a copy of the License at 
#
#       http://www.apache.org/licenses/LICENSE-2.0 
#
# Unless required by applicable law or agreed to in writing, software 
# distributed under the License is distributed on an "AS IS" BASIS, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
#
# See the License for the specific language governing permissions and 
# limitations under the License.
#

###############  Changeable  Parameters  ##############

include ../Makefile.common

ifeq ($(MAIN_EXE_NAME),)
    MAIN_EXE_NAME     =   halley
endif

####    Installation Directories    ####
BIN_INSTALL_DIR         = /usr/local/bin

###############  DO NOT MODIFY BELOW THIS   ##############

# 
# Sources
#
HALLEYTEST_SRCS     = halley.cpp

# 
# Corresponding obj files
#
HALLEYTEST_OBJS     = $(foreach obj, $(patsubst %.cpp,%.o,$(HALLEYTEST_SRCS)), $(OUTPUT_DIR)/$(obj))

MAIN_OBJS       = $(HALLEYTEST_OBJS)

MAIN_OUTPUT     = $(OUTPUT_DIR)/$(MAIN_EXE_NAME)

# 
# Libraries to include
#
LIBS    = -lpthread -luuid


###################     Begin Targets       ######################

LINK_STATICALLY = yes

all: base test
	@echo BIN_INSTALL_DIR = $(BIN_INSTALL_DIR)

.PHONY: clean cleanall distclean test

ifeq ($(LINK_STATICALLY),yes)

test: base $(MAIN_OBJS)
	@echo Building Test with Static Libraries...
	@$(GPP) $(CXXFLAGS) $(MAIN_OBJS) $(OUTPUT_DIR)/libhalley.a -o $(MAIN_OUTPUT) $(LIBS)

else

test: base $(MAIN_OBJS)
	@echo Building Test with Shared Libraries...
	$(GPP) $(CXXFLAGS) $(MAIN_OBJS) -o $(MAIN_OUTPUT) $(LIBS)

endif

install: 
	@echo "Nothing for install.  Run the test executable from here itself."

base:
	@mkdir -p "$(OUTPUT_DIR)"

clean:
	@rm -f $(MAIN_OBJS)

cleanall: clean
	@rm -f "$(MAIN_OUTPUT)"

distclean: cleanall
	@rm -f Makefile

help:
	@echo   "Usage: make <options> <targets>"
	@echo   "   Options:"
	@echo   "       BUILD_MODE=[debug | release]        -   Default: release"
	@echo   "       OUTPUT_DIR=<output_dir>             -   Directory to place all outputs. Default: bld"
	@echo   "       MAIN_EXE_NAME=<name>                -   Name of output file.  Default: $(MAIN_EXE_NAME)"
	@echo   "   Targets:"
	@echo   "       test:       Builds test executable (default)"
	@echo   "       base:       Core/Base checks (building output dirs etc)"
	@echo   "       clean:      Cleans all object files"
	@echo   "       cleanall:   Cleans all object files and executables"
	@echo   "       dep:        Build source/project dependencies"
	@echo   "       help:       Prints help information about targets and options"

dep:
	makedepend -Y -p"$(OUTPUT_DIR)/" -I../src   -- $(HALLEYTEST_SRCS)

# DO NOT DELETE

../bld/debug/halley.o: ../src/eds/server.h ../src/thread/thread.h
../bld/debug/halley.o: ../src/utils/listeners.h ../src/thread/mutex.h
../bld/debug/halley.o: ../src/thread/task.h ../src/thread/mutex.h
../bld/debug/halley.o: ../src/eds/fwd.h ../src/eds/http/httpfwd.h
../bld/debug/halley.o: ../src/eds/fwd.h ../src/eds/connection.h
../bld/debug/halley.o: ../src/net/sockbuff.h ../src/eds/job.h
../bld/debug/halley.o: ../src/eds/stage.h ../src/eds/event.h
../bld/debug/halley.o: ../src/eds/fileiohelper.h ../src/thread/task.h
../bld/debug/halley.o: ../src/eds/http/request.h ../src/eds/http/message.h
../bld/debug/halley.o: ../src/eds/http/headers.h ../src/eds/http/utils.h
../bld/debug/halley.o: ../src/eds/http/httpfwd.h ../src/eds/http/response.h
../bld/debug/halley.o: ../src/eds/http/readerstage.h
../bld/debug/halley.o: ../src/eds/http/handlerstage.h
../bld/debug/halley.o: ../src/eds/http/urlrouter.h
../bld/debug/halley.o: ../src/eds/http/httpmodule.h
../bld/debug/halley.o: ../src/eds/http/filemodule.h
../bld/debug/halley.o: ../src/eds/http/bayeux/bayeuxmodule.h
../bld/debug/halley.o: ../src/eds/http/httpmodule.h ../src/json/json.h
../bld/debug/halley.o: ../src/utils/refcount.h ../src/utils/membuff.h
../bld/debug/halley.o: ../src/eds/http/bayeux/channel.h
../bld/debug/halley.o: ../src/eds/http/httpfwd.h ../src/json/json.h
../bld/debug/halley.o: ../src/eds/http/contentmodule.h
../bld/debug/halley.o: ../src/eds/http/transfermodule.h
../bld/debug/halley.o: ../src/eds/http/writermodule.h
../bld/debug/halley.o: ../src/net/connhandler.h ../src/net/connfactory.h
../bld/debug/halley.o: ../src/net/server.h
