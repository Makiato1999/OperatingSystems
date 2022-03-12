# the compiler to use
CC = clang

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall -Wpedantic -Wextra -Werror
  
#files to link:
LFLAGS = 
  
# the name to use for both the target source file, and the output file:
TARGET1 = tmabsh
TARGET2 = touched.txt
TARGET3 = first5words.txt
TARGET4 = words
TARGET5 = temp
TARGET6 = nonvoluntaryswitches.txt
TARGET7 = test.txt
TARGET8 = rand5words.txt
TARGET9 = randsort5words.txt
TARGET10 = randsort5wordsprocsub.txt
  
all: $(TARGET1)
  
$(TARGET1): $(TARGET1).c
	$(CC) $(CFLAGS) -o $(TARGET1) $(TARGET1).c

# use 'make clean'
clean:
	rm -rf *o $(TARGET1)
	rm -rf *o $(TARGET2)
	rm -rf *o $(TARGET3)
	rm -rf *o $(TARGET4)
	rm -rf *o $(TARGET5)
	rm -rf *o $(TARGET6)
	rm -rf *o $(TARGET7)
	rm -rf *o $(TARGET8)
	rm -rf *o $(TARGET9)
	rm -rf *o $(TARGET10)