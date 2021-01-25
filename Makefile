CXX = g++
CXXFLAGS = -O2
LIBS = -ltermbox

default: snakey

snakey: snakey.cpp 
	$(CXX) $(CXXFLAGS) snakey.cpp -o snakey $(LIBS)

clean:
	$(RM) snakey