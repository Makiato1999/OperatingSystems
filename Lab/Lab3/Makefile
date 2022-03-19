# the compiler to use
CC = clang

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall -Wpedantic -Wextra -Werror
  
#files to link:
LFLAGS = -lpthread
  
# the name to use for both the target source file, and the output file:
TARGET1 = spin-lock
TARGET2 = atomic-lock
TARGET3 = lock-really

all: $(TARGET1) $(TARGET2) $(TARGET3)

$(TARGET1): $(TARGET1).c
	$(CC) $(CFLAGS) -o $(TARGET1) $(TARGET1).c $(LFLAGS)

$(TARGET2): $(TARGET2).c
	$(CC) $(CFLAGS) -o $(TARGET2) $(TARGET2).c $(LFLAGS)

$(TARGET3): $(TARGET3).c
	$(CC) $(CFLAGS) -o $(TARGET3) $(TARGET3).c $(LFLAGS)

# use 'make clean'
clean:
	rm -rf *o $(TARGET1)
	rm -rf *o $(TARGET2)
	rm -rf *o $(TARGET3)