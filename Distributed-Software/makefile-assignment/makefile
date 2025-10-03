# macros
CC = g++
FLAGS = -g
EXE_NAME = main
LINK_FLAGS = -lpthread
RM = /bin/rm -f

# list of object files
OBJS = main.o addOne.o addTwo.o

# default rule for compiling .cpp to .o
.cpp.o:
	${CC} ${FLAGS} -c $<

# final executable
${EXE_NAME}: ${OBJS}
	${CC} ${FLAGS} ${LINK_FLAGS} -o $@ ${OBJS}

# clean rule
clean:
	${RM} *.o ${EXE_NAME}
