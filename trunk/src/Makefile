
TARGET		= g3270
OBJECTS     = main.o log.o terminal.o screen_callbacks.o gsource.o actions.o \
				clipboard.o draw.o print.o

ifeq ($(DEBUG),1)
   DFLAGS=-DDEBUG=1 -g -O0
else
   DFLAGS=-DNDEBUG=1
endif

#---[ Set GTK Versions ]-------------------------------------------------------#

ifndef GTKLIB
   GTKLIB=gtk+-2.0
endif

ifndef GTK_VRS
   GTK_VRS=`pkg-config --modversion $(GTKLIB)`
endif

ifndef GTK
   GTK=`pkg-config --modversion $(GTKLIB) | cut -b 1`
endif

ifndef TMPPATH
   TMPPATH=/tmp
endif

ifndef LOGPATH
   LOGPATH=/var/log
endif

ifeq ($(GTKLIB),gtk+)
   GTK_MODULES=$(GTKLIB)
else
   GTK_MODULES=$(GTKLIB) gthread-2.0 glib-2.0
endif

PKG_MODULES=$(GTK_MODULES)


#---[ Build flags ]------------------------------------------------------------#

GCFLAGS     = -c `pkg-config --cflags $(PKG_MODULES)` \
              -DBUILD=\"`date +%G%m%d%H%M`\" -Wall -Werror \
              -DTARGET=\"$(TARGET)\" $(DFLAGS) \
              -DGTK=$(GTK) -DGTKVERSION=\"$(GTK_VRS)\" \
              -DTMPPATH=\"$(TMPPATH)\" -DLOGPATH=\"$(LOGPATH)\"

GLFLAGS     = `pkg-config --libs $(PKG_MODULES)` -L. -l3270

#---[ Leak check ]-------------------------------------------------------------#

# LeakTracer: http://freshmeat.net/projects/leaktracer/
DEBUG_RUN = LeakCheck ./$(TARGET) ; leak-analyze ./$(TARGET)

# Valgrind
# DEBUG_RUN = valgrind --tool=memcheck --leak-check=yes --db-attach=no ./$(TARGET)


ifndef DEBUG_RUN
   DEBUG_RUN=./$(TARGET)
endif

#---[ Rules ]------------------------------------------------------------------#

%.o: %.cpp *.h Makefile
	@echo $< ...
	g++ $(GCFLAGS) -o $@ -c $<

%.o: %.c *.h Makefile
	@echo $< ...
	gcc $(GCFLAGS) -o $@ -c $<

#---[ Targets ]----------------------------------------------------------------#

$(TARGET): lib3270.so $(OBJECTS)
	g++ $(GLFLAGS) -o $@ $(OBJECTS)

run: $(TARGET)
	@sync
ifdef DEBUG
	@LD_LIBRARY_PATH=. $(DEBUG_RUN)
else
	@LD_LIBRARY_PATH=. ./$(TARGET)
endif

lib/Makefile:
	@cd lib ; ./configure

lib3270.so: lib/Makefile lib/*.c lib/*.h
	@make -C lib lib3270.so
	@mv lib/lib3270.so $@

gtk.mk: ../configure
	@cd .. ; ./configure

clean:
	@rm -f $(TARGET)
	@rm -f $(OBJECTS)
	@make -C lib clean
	@rm -f lib3270.so
