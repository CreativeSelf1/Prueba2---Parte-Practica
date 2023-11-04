CC=g++
CFLAGS=-Wall -std=c++17 -O3
BINS= invertedIndex searcher memcache
all: clean invertedIndex searcher memcache

invertedIndex:
	$(CC) $(CFLAGS) -o programs/backend/invertedindex programs/backend/invertedindex.cpp programs/backend/include/loads.cpp programs/backend/include/countAllWords.cpp

searcher:
	$(CC) $(CFLAGS) -o programs/frontend/searcher programs/frontend/searcher.cpp

memcache:
	$(CC) $(CFLAGS) -o programs/memcache/memcache programs/memcache/memcache.cpp


clean:
	@echo " [CLN] Removing binary files"
	@rm -f $(BINS)
