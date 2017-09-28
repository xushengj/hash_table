CC=gcc
CXX=g++
HASH_TABLE_FLAG=-std=c89 -pedantic -I..
TEST_FLAG=-std=c++11 -I..

all: test.exe

test.exe: hash_table.o test.o
	$(CXX) test.o hash_table.o -o test.exe
	
hash_table.o: hash_table_test_impl.c
	$(CC) $(HASH_TABLE_FLAG) -c hash_table_test_impl.c -o hash_table.o
	
test.o: test.cpp
	$(CXX) $(TEST_FLAG) -c test.cpp -o test.o
	
