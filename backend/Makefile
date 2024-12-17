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

BUILD_DIR = ../build
BACKEND_DIR = backend
INCLUDES = ../include ../common/logger ../common/text include
SOURCES = src/main.cpp src/backend.cpp
OBJECTS = $(addprefix $(BUILD_DIR)/backend/, $(SOURCES:%.cpp=%.o))

CFLAGS += $(addprefix -I, $(INCLUDES))
LDFLAGS = -L$(BUILD_DIR)/libs -lcommon -lmylibrary
EXECUT = $(BUILD_DIR)/backy

all: $(EXECUT)

$(EXECUT): $(OBJECTS)
	@mkdir -p $(@D)
	@$(CC) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/backend/src/%.o: src/%.cpp
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MP -MMD -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR)/backend/src/*.o $(BUILD_DIR)/backend/backend
