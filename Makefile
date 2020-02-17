CC					:= g++
CCFLAGS			:= -Wall
DEBUGFLAGS	:= -DDEBUG

TARGET	:= Gpio
MAINS		:= $(addsuffix .o, $(TARGET) )
OBJ			:= $(MAINS)
DEPS		:=

.PHONY: all clean

all: $(TARGET)

debug: CCFLAGS += $(DEBUGFLAGS)
debug: $(TARGET)

clean:
	rm -f $(TARGET)

$(TARGET): % : %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CCFLAGS)
