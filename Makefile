CC ?= g++

CFLAGS = -Wall -Werror 

OBJFILES = bitio.o main.o lz78.o

EXECUTABLE = lz78

$(EXECUTABLE): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(OBJFILES)

lz78.o: 	lz78.h bitio.h

main.o: 	lz78.h

bitio.o: 	bitio.h

clean: 		
			rm $(OBJFILES) $(EXECUTABLE)
