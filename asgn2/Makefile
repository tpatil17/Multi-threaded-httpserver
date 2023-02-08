CC	=	clang

CFLAGS	=	-gdwarf-4	-Wall	-Wextra	-Werror	-pedantic -pthread

TARGET	=	httpserver

all:$(TARGET)
	
$(TARGET):	$(TARGET).c 
	$(CC)	$(CFLAGS)	-o	$(TARGET)	$(TARGET).c	asgn2_helper_funcs.a 

clean:	
	rm	-f	$(TARGET)	$(TARGET).o
	

