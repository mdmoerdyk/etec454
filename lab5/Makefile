#
# Makefile for GENERIC 9S12 MetroWerks project.
# Todd Morton, 01/12/09 
# 
# Enter the project name, C sources (CSRCS), assembly sources(SRCS),  
#
CSRCS = MultiTap.c OSProj/ucos_ii.c 9S12DP256Start.c UcosKey.c Clock.c TimeSet.c TextEntry.c LayeredLcd.c 
SRCS = s12dp256vecs.a12 OSProj/os_cpu_a.a12 SCIBIO9S12Dx.a12 BasicIO.a12
# 
# Use one of the following linker command files for a standard configuration 
# or create a project linker command file specific for your project and enter 
# it below on the 'PRM = ' line. 
#
PRM = 9S12DP256sm.prm   
#
# Use one of the following Metrowerks paths to set the compiler version to use.
#  
MWPROG = "C:/Program Files/Freescale/Codewarrior for HCS12/Prog/"
#
# In general you should not have to change anything from here on. 
# 
# The include and library paths are all defined in the project.ini file.
# 
.SUFFIXES : .o12 .a12 .c 
# 
LIBS = ANSIS.LIB
COBJS = $(CSRCS:.c=.o12) 
OBJS = $(SRCS:.a12=.o12) 
LSTS = $(SRCS:.a12=.lst) $(CSRCS:.c=.lst)

all: $(PROJ).abs

$(PROJ).abs: $(OBJS) $(COBJS)
	$(MWPROG)Linker.exe $(PRM) -B -add\($(OBJS)$(COBJS)$(LIBS)\) -o$(PROJ).abs

.a12.o12: 
	$(MWPROG)ahc12.exe -Ms -L=$*.lst -Objn$*.o12 $(ASLIBS) $<

.c.o12: 
	$(MWPROG)chc12.exe -Tuc1ue1 -Cc -OnCstVar -OnB=a -Onf -Wpd -Ms -Lasm=$*.lst -Objn=$*.o12 -prod=project.ini $<
        
clean:
	rm -f $(PROJ).abs $(OBJS) $(COBJS) $(PROJ).sx $(LSTS) $(PROJ).map EDOUT *.dbg
