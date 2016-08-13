common_o =  d3plot.o options.o
info_o   = lsdt-info.o
dump_o   = lsdt-dump.o

CFLAGS = -pg -g -I/usr/include/vtk -Wno-deprecated
#-lvtkDICOMParser
LDFLAGS = -pg -g -L/usr/lib/vtk -lvtkIO -lvtkexpat -lvtkFiltering  -lvtkpng -lvtkzlib -lvtkjpeg -lvtktiff -lvtkCommon -ldl

all: lsdt-dump lsdt-info

lsdt-info: $(common_o) $(info_o)
	g++ -g -o $@ $(common_o) $(info_o) $(LDFLAGS) 

lsdt-dump: $(common_o) $(dump_o)
	g++ -g -o $@ $(common_o) $(dump_o) $(LDFLAGS) 

%.o: %.cpp 
	g++ $(CFLAGS) -c -o $@ $<

clean:
	-rm -f *.o lsdt-info lsdt-dump
