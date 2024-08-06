SRC=alloc.c
BIN=alloc.so

BIN_PATH=bin/$(BIN)


install: $(SRC)
	gcc -o $(BIN_PATH) -fPIC -shared $(SRC)

clean: $(SRC)
	rm $(BIN_PATH)