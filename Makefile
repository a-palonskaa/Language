BACKEND_DIR = backend
FRONTEND_DIR = frontend
MIDDLEEND_DIR = middleend

all: front middle back

front:
	@$(MAKE) -C $(FRONTEND_DIR) all

middle:
	@$(MAKE) -C $(MIDDLEEND_DIR) all

back:
	@$(MAKE) -C $(BACKEND_DIR) all

clean:
	@for dir in $(SUBDIRS); do  \
		$(MAKE) -C $$dir clean; \
	done
	@rm -f $(OBJECTS) $(EXECUTABLE) build/libs/libmylibrary.a

echo:
	echo $(OBJECTS)
