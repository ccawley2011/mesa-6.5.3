# progs/riscos/Makefile

TOP = ../..
include $(TOP)/configs/current

INCDIR = $(TOP)/include

OSMESA_LIBS = -L$(TOP)/$(LIB_DIR) -lglut -lOSMesa $(APP_LIB_DEPS)

LIB_DEP = $(TOP)/$(LIB_DIR)/$(GL_LIB_NAME) $(TOP)/$(LIB_DIR)/$(GLU_LIB_NAME) $(TOP)/$(LIB_DIR)/$(GLUT_LIB_NAME)

PROGS = \
	desk_vgear \
	morph32 \
	movelight \
	rosdemo32 \
	teapot32


##### RULES #####

.SUFFIXES:
.SUFFIXES: .c


# make executable from .c file:
.c: $(LIB_DEP)
	$(CC) -I$(INCDIR) $(CFLAGS) $< $(APP_LIB_DEPS) -o $@


##### TARGETS #####

default: $(PROGS)


# special case: need the -lOSMesa library:
desk_vgear: desk_vgear.c
	$(CC) -I$(INCDIR) $(CFLAGS) desk_vgear.c $(OSMESA_LIBS) -o $@

# special case: need the -lOSMesa library:
morph32: morph32.c
	$(CC) -I$(INCDIR) $(CFLAGS) morph32.c $(OSMESA_LIBS) -o $@

# special case: need the -lOSMesa library:
movelight: movelight.c
	$(CC) -I$(INCDIR) $(CFLAGS) movelight.c $(OSMESA_LIBS) -o $@

# special case: need the -lOSMesa library:
rosdemo32: rosdemo32.c
	$(CC) -I$(INCDIR) $(CFLAGS) rosdemo32.c $(OSMESA_LIBS) -o $@

# special case: need the -lOSMesa library:
teapot32: teapot32.c
	$(CC) -I$(INCDIR) $(CFLAGS) teapot32.c $(OSMESA_LIBS) -o $@



clean:
	-rm -f $(PROGS)
	-rm -f *.o *~


