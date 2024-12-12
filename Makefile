# Biến
CC = gcc
CFLAGS = -Wall
PKG_CONFIG_FLAGS = `pkg-config --libs --cflags gtk+-3.0 gstreamer-1.0`
LDFLAGS = -luuid
CLIENT = client
OUTPUT = $(CLIENT)/app
PROG = app
SOURCES = $(CLIENT)/app_function.c $(CLIENT)/utils.c

# Mục tiêu mặc định
all: $(OUTPUT)
	./$(OUTPUT)

# Quy tắc biên dịch và liên kết tệp để tạo myapp trong thư mục client
$(OUTPUT): $(SOURCES)
	$(CC) $(SOURCES) -o $(OUTPUT) $(PKG_CONFIG_FLAGS) $(LDFLAGS)

# Quy tắc làm sạch
clean:
	rm -f $(OUTPUT)
