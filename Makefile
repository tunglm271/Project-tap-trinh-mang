# Biến
CC = gcc
CFLAGS = -Wall
LDFLAGS = `pkg-config --libs --cflags gtk+-3.0 gstreamer-1.0`
CLIENT = client
OUTPUT = $(CLIENT)/app
PROG = app
SOURCES = $(CLIENT)/$(PROG).c $(CLIENT)/app_function.c 

# Mục tiêu mặc định
all: $(OUTPUT)
	./$(OUTPUT)

# Quy tắc biên dịch và liên kết tệp để tạo myapp trong thư mục client
$(OUTPUT): $(SOURCES)
	$(CC) $(SOURCES) -o $(OUTPUT) $(LDFLAGS)

# Quy tắc làm sạch
clean:
	rm -f $(OUTPUT)
