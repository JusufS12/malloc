BIN=alloc
BIN_PATH=bin/$(BIN)
SRC=alloc.c

$(BIN_PATH): $(SRC)
	gcc -o $(BIN_PATH) $(SRC)

clean: $(SRC)
	rm $(BIN_PATH)
	gcc -o $(BIN_PATH) $(SRC)