# del command is for windows
# in linux, use rm instead
OBJ = main.o
INC = -I "./"

Raytracer: $(OBJ)
	g++ $(OBJ) -o Raytracer.exe
	del -f $(OBJ)
	
main.o:
	g++ -c main.cpp $(INC)
	
clean:
	del -f $(OBJ) Raytracer