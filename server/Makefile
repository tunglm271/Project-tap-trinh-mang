CC = gcc
SRC = server.c data/data.c data/rooms.c data/log_session.c data/log_result.c
TARGET = server

all: $(TARGET)

$(TARGET): $(SRC) $(wildcard *.h)
	$(CC)  $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean