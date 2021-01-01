writer_reader: writer.c reader.c writer_reader.c
	gcc writer.c writer_reader.c -o writer -Wall -lrt -pthread
	gcc reader.c writer_reader.c -o reader -Wall -lrt -pthread
