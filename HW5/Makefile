FLAGS   = -std=c99 -Wall -O3
SOURCES = matmul.c
OBJECTS = matmul.o
EXEBIN  = matmul

all: $(EXEBIN)

$(EXEBIN) : $(OBJECTS)
	gcc -o $(EXEBIN) $(OBJECTS)

$(OBJECTS) : $(SOURCES)
	gcc -c $(FLAGS) $(SOURCES)

clean :
	rm -f $(EXEBIN) $(OBJECTS)
