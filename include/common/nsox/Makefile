include Makefile.inc
OBJ_DIR := objects
CPP_SOURCES :=  $(wildcard *.cpp)
CPP_OBJECTS :=  $(CPP_SOURCES:%.cpp=$(OBJ_DIR)/%.o)


all:$(OBJ_DIR) $(CPP_OBJECTS)
	$(CXX32) $(CXXFLAG_R) -o nsox $(CPP_OBJECTS)

$(OBJ_DIR):
	-mkdir $(OBJ_DIR)

$(CPP_OBJECTS): $(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAG) ${INCLUDE} -c  $< -o $@

clean: 
	-rm -f $(OBJ_DIR)/*.o
	-rm -f $(OBJ_DIR)/*.ro
	-rm -f nsox

