
all:
	g++ gl-demo.cpp -o test.out -lglx -lX11 -lEGL -lGL -lopencv_core -lopencv_highgui -lgomp -L/usr/lib/xorg/modules/extensions
	
clean:
	rm -f test.out
	rm -f img.png