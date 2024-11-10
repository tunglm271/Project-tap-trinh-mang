# Biến
CC = gcc
CFLAGS = -Wall
LDFLAGS = `pkg-config --libs --cflags gtk+-3.0`
CLIENT = client
OUTPUT = $(CLIENT)/myapp
PROG = gui

# Mục tiêu mặc định
all: $(OUTPUT)
	./$(OUTPUT)

# Quy tắc biên dịch và liên kết tệp gui.c để tạo myapp trong thư mục client
$(OUTPUT): $(CLIENT)/$(PROG).c
	$(CC) $(CLIENT)/$(PROG).c -o $(OUTPUT) $(LDFLAGS)

# Quy tắc làm sạch
clean:
	rm -f $(OUTPUT)
