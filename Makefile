all: 
	gcc hlCursors.c -o hlCursors -lXcursor -lm

clean:
	rm hlCursors
