gcc socklib.c *server*.c -o server -lpthread
chmod u+x server
./server $@