# the compiler to use
CC = clang

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall -Wpedantic -Wextra -Werror
  
#files to link:
LFLAGS = -lpthread
  
# the name to use for both the target source file, and the output file:
TARGET1 = a3q1
  
all: $(TARGET1)
  
$(TARGET1): $(TARGET1).c
	$(CC) $(CFLAGS) -o $(TARGET1) $(TARGET1).c $(LFLAGS)

# use 'make clean'
clean:
	rm -rf *o $(TARGET1)