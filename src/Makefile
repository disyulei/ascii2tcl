CC       =  g++
CFLAGS   =  -funroll-all-loops -O2 -m64 -fPIC -Wno-deprecated 
#CFLAGS  =  -g -Wno-deprecated 
BOOST_I  =-I$(BOOST_DIR)
BLIB_I   =-I./bLib

MKDIR = if [ ! -d $(@D) ]; then mkdir $(@D); fi
BPATH = ../build/
OPATH = ../obj/
SRCS	= $(wildcard *.cpp)
OBJS = $(SRCS:%.cpp=$(OPATH)%.o)

EXE	= $(BPATH)ascii2tcl

.SUFFIXES: .cc .cpp .o .x

$(EXE): $(OBJS) tags
	@$(MKDIR)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(OPATH)%.o:%.cpp
	@$(MKDIR)
	$(CC) $(CFLAGS) -c -o $@ $< $(BOOST_I) $(BLIB_I)

tags: $(SRCS)
	ctags -R *

clean:
	rm -f *.o *~ $(EXE) $(OPATH)*
