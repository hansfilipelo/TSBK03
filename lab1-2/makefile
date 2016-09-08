UNAME_S=$(shell uname -s)
SRC_FOLDER=src

EXEC=lab1-2
SOURCES = $(SRC_FOLDER)/$(EXEC).c $(wildcard ../common/*.c)
OBJS = $(SOURCES:.c=.o)

CCFLAGS = -Wall -std=c99 -DGL_GLEXT_PROTOTYPES
LDFLAGS = -lm
INCLUDES =
MGLUT =

ifeq ($(UNAME_S),Linux)
	LDFLAGS += -lGL -lX11 -lXext
	MGLUT = ../common/Linux/MicroGlut.c
	OBJS += $(MGLUT:.c=.o)
	INCLUDES +=-ILinux
endif

ifeq ($(UNAME_S),Darwin)
	LDFLAGS += -framework Cocoa -framework CoreGraphics -framework OpenGL -framework CoreFoundation
	MGLUT = ../common/mac/MicroGlut.m
	OBJS += $(MGLUT:.m=.o)
	INCLUDES += -Imac
endif

all : $(EXEC)

$(EXEC): $(OBJS)
	gcc $(CCFLAGS) -o $@ $(OBJS) $(INCLUDES) $(LDFLAGS)

%.o : %.c %.h
	gcc $(CCFLAGS) $(INCLUDES) -c -o $@ $<

%.o : %.m %.h
	gcc $(CCFLAGS) -c -o $@ $< $(INCLUDES)

clean :
	rm -f $(EXEC) $(OBJS) *~
