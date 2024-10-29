FROM gcc:latest

WORKDIR /usr/src/app

COPY . .

RUN make

EXPOSE 1444

CMD ["./server"]
