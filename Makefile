
server:server.c
	gcc $^ -o $@
clean:
	rm -rf server
