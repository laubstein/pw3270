
include gtk.mk

TARGET		= g3270
OBJECTS     = main.o log.o terminal.o

ifdef DEBUG
   DFLAGS=-DDEBUG=1 -g -O0
else
   DFLAGS=-DNDEBUG=1
endif

GCFLAGS     = -c `pkg-config --cflags $(GTK)` \
              -DBUILD=\"`date +%G%m%d%H%M`\" -Wall -Werror \
              -DTARGET=\"$(TARGET)\" $(DFLAGS)

GLFLAGS     = `pkg-config --libs $(GTK)`

#---[ Leak check ]-------------------------------------------------------------#

# LeakTracer: http://freshmeat.net/projects/leaktracer/
DEBUG_RUN = LeakCheck ./$(TARGET) ; leak-analyze ./$(TARGET)

# Valgrind
# DEBUG_RUN = valgrind --tool=memcheck --leak-check=yes --db-attach=no ./$(TARGET)


ifndef DEBUG_RUN
   DEBUG_RUN=./$(TARGET)
endif

#---[ Rules ]------------------------------------------------------------------#

%.o: %.cpp *.h Makefile *.mk
	@echo $< ...
	@g++ $(GCFLAGS) -o $@ -c $<

%.o: %.c *.h Makefile *.mk
	@echo $< ...
	@gcc $(GCFLAGS) -o $@ -c $<

#---[ Targets ]----------------------------------------------------------------#

$(TARGET): $(OBJECTS)
	@g++ $(GLFLAGS) -o $@ $(OBJECTS)

run: $(TARGET)
ifdef DEBUG
	@$(DEBUG_RUN)
else
	@./$(TARGET)
endif

gtk.mk: ../configure
	@cd .. ; ./configure

clean:
	@rm -f $(TARGET)
	@rm -f $(OBJECTS)
