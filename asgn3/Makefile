CC	=	clang

CFLAGS	=	-gdwarf-4	-Wall	-Wextra	-Werror	-pedantic -pthread

TARGET	=	basic

OBJECT	=	queue

all:$(TARGET)	$(OBJECT)
	
$(OBJECT).o:	$(OBJECT).c $(OBJECT).h
	$(CC)	$(CFLAGS)	-c	$(OBJECT).c

$(TARGET).o:	$(TARGET).c $(OBJECT).h
	$(CC)	$(CFLAGS)	-c	$(TARGET).c

$(TARGET):	$(TARGET).o	$(OBJECT).o
	$(CC):	$(CFLAGS)	-o	$(TARGET)	$(TARGET).o	$(OBJECT).o

clean:	
	rm	-f	$(TARGET).o	$(OBJECT).o
	
