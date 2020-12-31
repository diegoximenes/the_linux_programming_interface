all: wait.c post.c wait_post.c
	g++ wait.c wait_post.c -o wait -pthread -Wall
	g++ post.c wait_post.c -o post -pthread -Wall
