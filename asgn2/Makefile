CC	=	clang

CFLAGS	=	-g	-Wall	-Wextra	-Werror	-pedantic -pthread

TARGET	=	scaffold

all:$(TARGET)
	
$(TARGET):	$(TARGET).c 
	$(CC)	$(CFLAGS)	-o	$(TARGET)	$(TARGET).c	asgn2_helper_funcs.a 

clean:	
	rm	-f	$(TARGET)	$(TARGET).o
	

