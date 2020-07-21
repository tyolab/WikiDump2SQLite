
SRCDIR = src
OBJDIR = obj
BINDIR = .

UTILSDIR = utils

HEADERDIR = include

HEADERS = $(wildcard $(HEADERDIR)/*.h) \
			$(wildcard $(SRCDIR)/*.h)

CC := g++

CFLAGS  := -std=c++11 -I$(HEADERDIR) -I$(SRCDIR) -I/data/tyolab/code/antelope/source

LDFLAGS := -lsqlite3 -ldl -lpthread -lz -lcurl -lantelope_core

ISRELEASE = 0
ifeq ($(REL), 1)
	ISRELEASE = 1
endif

ifeq ($(RELEASE), 1)
	ISRELEASE = 1
endif

ifeq ($(ISRELEASE), 1)
	CFLAGS += -DRELEASE -O3
else
	CFLAGS +=  -g 
endif

SOURCES  := $(wildcard $(SRCDIR)/*.cpp)

UTILS := $(wildcard $(UTILSDIR)/*.cpp)

OBJECTS  := $(SOURCES:.cpp=.o)

UTILS_OBJS := $(UTILS:.cpp=.o)

#MAINS := $(UTILS:.cpp=.o)

#main = $(UTILS:.cpp=)
main = dump2sqlite

#test = test_article_insert

executables = $(main) dump2indexable dump2db import_dump merge_sqlite_db dump2template math2db merge_articles 

all: $(executables)

$(main): $(OBJECTS) $(UTILSDIR)/$(main).o
	$(CC) -o $@ $^ $(LDFLAGS)
	
dump2indexable: $(OBJECTS) $(UTILSDIR)/dump2indexable.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	
dump2db: $(OBJECTS) $(UTILSDIR)/dump2db.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	
import_dump: $(OBJECTS) $(UTILSDIR)/import_dump.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 	
	
dump2template: $(OBJECTS) $(UTILSDIR)/dump2template.o
	$(CC) -o $@ $^ $(LDFLAGS) 
	
merge_sqlite_db: $(OBJECTS) $(UTILSDIR)/merge_sqlite_db.o
	$(CC) -o $@ $^ $(LDFLAGS)	
	
merge_articles: $(OBJECTS) $(UTILSDIR)/merge_articles.o
	$(CC) -o $@ $^ $(LDFLAGS)	
	
math2db: $(OBJECTS) $(UTILSDIR)/math2db.o
	$(CC) -o $@ $^ $(LDFLAGS)	
	
#dump2indexable:	dump2indexable.o
#	$(CC) -o $@ $^ -lantelope_core
#	
#dump2indexable.o: uitls/dump2indexable.cpp
#	$(CC) $(CFLAGS) -c $< -o $@
	
#$(OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.cpp
%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@
	
install: $(executables)
	cp $(executables) /usr/local/bin/	

clean:
	\rm -rf $(OBJECTS) $(UTILS_OBJS) $(executables)

