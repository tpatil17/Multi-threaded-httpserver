CC	=	clang

CFLAGS	=	-g	-Wall	-Wextra	-Werror	-pedantic -pthread

TARGET	=	memory

all:$(TARGET)
	
$(TARGET):	$(TARGET).c 
	$(CC)	$(CFLAGS)	-o	$(TARGET)	$(TARGET).c 

clean:	
	rm	-f	$(TARGET)	$(TARGET).o
	

