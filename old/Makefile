#This is the RadMud Makefile.
#Change this to reflect whichever compiler you want to use.
CXX      = g++

#Warnings flags.
W_FLAGS  = -Wall -Wunused -Werror -Wshadow -Wformat-security -Wpointer-arith -Wcast-align -Wredundant-decls -Wno-error=unused-function -Wno-unused-function

#Miscellaneous compiler options.
CXXFLAGS = -g3 $(W_FLAGS) $(EXPORT_SYMBOLS) -pedantic -std=c++11 #-w

# Other library.
LIBS     = -ldl -lz -lpthread

# Name of the executable.
TARGET   = RadClient

## MAKE RULES #################################################################
all:
	@echo "Building $(TARGET)...";
	@$(CXX) $(CXXFLAGS) -o $(TARGET) main.cpp utils.cpp $(LIBS)
	@echo "Done.";

## CLEAN RULES ################################################################
clean:
	@echo "Cleaning.";
	@-rm -rf $(TARGET) $(OBJ) $(DEPENDS)