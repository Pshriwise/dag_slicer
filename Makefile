

SRC_DIR = ./src/
TEST_DIR = ./test/


.PHONY: src test

all: src test 

src: 
	$(MAKE) -C $(SRC_DIR)

test: src
	$(MAKE) -C $(TEST_DIR)

check: test
	$(MAKE) check -C $(TEST_DIR)

clean: 
	rm -f *~
	$(MAKE) clean -C $(TEST_DIR)
	$(MAKE) clean -C $(SRC_DIR)
