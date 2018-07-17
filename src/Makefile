#----------------------------------
# example Makefile used to generate
# binary .so file
#----------------------------------

XX			= g++
LIBS		= -L.  -lthrift -lboost_thread -lboost_system
#FLAGS		= -std=c++0x -Wall -Wnon-virtual-dtor -DNDEBUG -O2 -ggdb -shared -fPIC -MMD -D_USE_CONFIG_FILE
FLAGS		= -std=c++0x -Wall -Wnon-virtual-dtor -DNDEBUG -O2 -ggdb -shared -fPIC -MMD -DHAVE_NETINET_IN_H
CONN_DIR    = ./connection
BASE_DIR    = ./base
URILS_DIR   = ./utils
COMMON_DIR  = ./common
HANDLER_DIR     = ./handler
THRIFT_DIR      = ./thrift
#FLAGS		= -std=c++0x -Wall -DNDEBUG -O2 -shared -fPIC -MMD

SOURCE		= ${wildcard *.cpp \
			  ${CONN_DIR}/*.cpp \
		      ${BASE_DIR}/*.cpp \
			  ${URILS_DIR}/*.cpp \
			  ${COMMON_DIR}/*.cpp \
			  ${HANDLER_DIR}/*.cpp \
			  ${THRIFT_DIR}/*.cpp}
			 

INCLUDES         = -I. \
		           -I${CONN_DIR} \
			       -I${BASE_DIR} \
			       -I${URILS_DIR} \
			       -I${COMMON_DIR} \
			       -I${HANDLER_DIR} \
				   -I${THRIFT_DIR}
	
OBJS		= $(patsubst %.cpp, %.o, $(SOURCE))
# 删除.d文件 
OBJSD       = $(patsubst %.cpp, %.d, $(SOURCE))
TARGET		= ../bin/async-proxy.so

all: $(TARGET)

$(TARGET): $(OBJS)
	$(XX) $(FLAGS) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.cpp Makefile
	$(XX) $(FLAGS) $(INCLUDES) -c $< -o $@
	#@-mv -f $*.d .dep.$@

clean:
	rm -rf  $(OBJS) $(OBJSD) $(TARGET)

c: clean

-include /dev/null $(wildcard .dep.*)
