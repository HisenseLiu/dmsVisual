.PHONY: all clean depend dist
.SUFFIXES:

include ${DFDZPLAT}/include/makedefs

#---------------------------- Macro definitions ------------------------------

SRCDIR = ${DFDZPLAT}/home/lhx/dmsVisual

NO_DFDZPLAT_INCPATH = \
	-I${DFDZPLAT}/src/dbms/include\
	-I${DFDZPLAT}/src/dbms/dbs/include\
	-I${DFDZPLAT}/src/dbms/dbs/pas/include\
	-I${DFDZPLAT}/src/scada/include\
	-I${DFDZPLAT}/src/plat/include\
	-I${DFDZPLAT}/src/isrv/pas/topo/include\

DFDZPLAT_INCPATH = -I${SRCDIR} -I${INCDIR}

INCDIRPATH = ${DFDZPLAT_INCPATH} ${NO_DFDZPLAT_INCPATH}

dmsVisual = ${BINDIR}/dmsVisual

dmsVisual_OBJLIST = \
        ./obj/traceGraph.${OBJEXT}\
        ./obj/dmsSearch.${OBJEXT}\
		./obj/tinyxml2.${OBJEXT}\
        ./obj/cJSON.${OBJEXT}\
        ./obj/main.${OBJEXT}

LIB_dmsVisual_SRCLIST = \
	./traceGraph.cpp \
	./dmsSearch.cpp \
	./tinyxml2.cpp \
	./cJSON.c \
	./main.cpp

BINLOC_dmsVisual_LIB =  \
	-lpasShm \
    -lpasDb\
    -lecs \
    -lscada\
    -lplat\
    -leds\
    -lpaspscdatastruct

ifeq (${OS}, AIX)
LD=${CXX}
CXXLDFLAGS += -brtl -bernotok
endif
#------------------------ Target file dependencies ---------------------------

all:\
	${dmsVisual}

${dmsVisual}: ${dmsVisual_OBJLIST}
	${CXX} -o $@ ${CXXLDFLAGS} ${dmsVisual_OBJLIST} -L${LIBDIR} ${BINLOC_dmsVisual_LIB} -L$(QTDIR)/lib ${CXXLIB}

#------------------------ Include file dependencies --------------------------

depend:
	@(makedepend -p${OUTDIR}/ -o.${OBJEXT} -f- ${INCDIRPATH} ${LIB_dmsVisual_SRCLIST}) >.depend 2>/dev/null
	@rm -f makefile.bak

clean:
	rm -f ${dmsVisual_OBJLIST}
	rm -f ${dmsVisual}

./obj/traceGraph.${OBJEXT}: ${SRCDIR}/traceGraph.cpp
	${CXX} -c ${TEMPLATEFLAGS} ${CXXFLAGS} ${INCDIRPATH} ${FMSR_VER} ${OBJNAME} $<
	
./obj/dmsSearch.${OBJEXT}: ${SRCDIR}/dmsSearch.cpp
	${CXX} -c ${TEMPLATEFLAGS} ${CXXFLAGS} ${INCDIRPATH} ${FMSR_VER} ${OBJNAME} $<

./obj/tinyxml2.${OBJEXT}: ${SRCDIR}/tinyxml2.cpp
	${CXX} -c ${TEMPLATEFLAGS} ${CXXFLAGS} ${INCDIRPATH} ${FMSR_VER} ${OBJNAME} $<

./obj/cJSON.${OBJEXT}: ${SRCDIR}/cJSON.c
	${CXX} -c ${TEMPLATEFLAGS} ${CXXFLAGS} ${INCDIRPATH} ${FMSR_VER} ${OBJNAME} $<

./obj/main.${OBJEXT}: ${SRCDIR}/main.cpp
	${CXX} -c ${TEMPLATEFLAGS} ${CXXFLAGS} ${INCDIRPATH} ${FMSR_VER} ${OBJNAME} $<

-include .depend

# DO NOT DELETE THIS LINE -- make depend depends on it.


