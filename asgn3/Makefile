CC	=	clang

CFLAGS	=	-gdwarf-4	-Wall	-Wextra	-Werror	-pedantic -pthread

TARGET	=	queue

all:$(TARGET).o
	
$(TARGET).o:	$(TARGET).c $(TARGET).h
	$(CC)	$(CFLAGS)	-c	$(TARGET).c 

clean:	
	rm	-f	$(TARGET).o
	

	
