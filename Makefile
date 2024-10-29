CC = gcc
CFLAGS = -Wall -Wextra -Isrc -Isrc/lib/cJSON -Isrc/cxc -lsqlite3
SRC_DIR = src
LIB_DIR = src/lib/cJSON
SRCS = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(LIB_DIR)/*.c) $(wildcard $(SRC_DIR)/*/*.c)

SERVER = server
CXC = cxc

IMAGE_NAME = c-webserver

.PHONY: all clean cxc

all: clean cxc $(SERVER)

$(SERVER): $(SRCS)
	@$(CC) $(CFLAGS) -o $@ $^

cxc:
	@$(CC) $(CFLAGS) -o $(CXC) src_cxc/main.c
	@./$(CXC)

clean:
	@rm -f $(SERVER)
	@rm -f $(CXC)

docker-build:
	docker build -t $(IMAGE_NAME) .

docker-run:
	docker run -p 1444:1444 --name $(IMAGE_NAME) $(IMAGE_NAME)

docker-stop:
	docker stop $(IMAGE_NAME) || true
	docker rm $(IMAGE_NAME) || true

docker-clean: stop
	docker rmi $(IMAGE_NAME) || true

docker-dev:
	docker stop $(IMAGE_NAME) || true
	docker rm $(IMAGE_NAME) || true
	docker run -p 1444:1444 -v $(PWD):/usr/src/app --name $(IMAGE_NAME) $(IMAGE_NAME)

docker-rebuild: docker-stop docker-build docker-run
