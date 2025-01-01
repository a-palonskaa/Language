CC = g++
CFLAGS = -Wall -std=c++17 -Wall -Wextra -Weffc++ -Wc++14-compat -Wmissing-declarations   \
		 -Wcast-align -Wcast-qual -Wchar-subscripts -Wconversion -Wctor-dtor-privacy     \
		 -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat=2     \
		 -Winline -Wnon-virtual-dtor -Woverloaded-virtual -Wpacked -Wpointer-arith       \
		 -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo           \
		 -Wstrict-overflow=2 -Wsuggest-override -Wswitch-default -Wswitch-enum -Wundef   \
		 -Wunreachable-code -Wunused -Wvariadic-macros -Wno-literal-range 			     \
		 -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast 			 \
		 -Wno-varargs -Wstack-protector -Wsuggest-override -Wbounds-attributes-redundant \
		 -Wlong-long -Wopenmp -fcheck-new -fsized-deallocation -fstack-protector 		 \
		 -fstrict-overflow -fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-protector  \
		 -fPIE -Werror=vla
COMMON_DIR = ../build/libs
LIBS_DIR = $(COMMON_DIR)

BUILD_DIR = ../build/frontend

INCLUDES = include ../common/logger ../common/text
SOURCES = dump.cpp main.cpp parser.cpp prog_tree.cpp tokenization.cpp serialization.cpp
EXCLUDE_SOURCES = main.cpp
OBJECTS = $(addprefix $(BUILD_DIR)/src/, $(SOURCES:%.cpp=%.o))
OBJECTS_FOR_LIB = $(filter-out $(addprefix $(BUILD_DIR)/src/, $(EXCLUDE_SOURCES:%.cpp=%.o)), $(OBJECTS))

DEPS = $(OBJECTS:%.o=%.d)
LIBS = ../build/libs/libmylibrary.a
EXECUTABLE = ../build/front

CFLAGS += $(addprefix -I, $(INCLUDES))
LDFLAGS = -L$(LIBS_DIR) -lcommon

.PHONY: all libs prog clean

all: libs prog

prog: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@$(CC) $(LDFLAGS) $^ -o $@

$(OBJECTS): $(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MP -MMD -c $< -o $@

libs: $(LIBS)

$(LIBS): $(OBJECTS_FOR_LIB)
	@mkdir -p $(BUILD_DIR)/libs
	@ar rcs $@ $^

clean:
	@for dir in $(SUBDIRS); do  \
		$(MAKE) -C $$dir clean; \
	done
	@rm -f $(OBJECTS) $(EXECUTABLE) $(LIBS)

echo:
	echo $(OBJECTS)
