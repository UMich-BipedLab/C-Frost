CXX      := g++
CXXFLAGS := -Wall -Wno-unused-variable -Wno-unused-value
LDFLAGS  := -lm
BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
APP_DIR  := $(BUILD)/apps
TARGET   := program
INCLUDE  := -Iinclude/
SRCS     :=                     \
   $(wildcard src/gen/*.cc)	\
   $(wildcard src/*.cc)         \
   $(wildcard src/*.cpp)	\

OBJECTS := $(SRCS:%.cc=$(OBJ_DIR)/%.o)
#OBJECTS := $(foreach program,$(SRCS),$(OBJ_DIR)/$(program).o)

all: build $(APP_DIR)/$(TARGET)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $<
$(OBJ_DIR)/%.o: %.cc
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $<


$(APP_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LDFLAGS) $(OBJECTS) -o $@

.PHONY: all build clean debug release

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: all

release: CXXFLAGS += -O2
release: all

install:
	cp $(APP_DIR)/$(TARGET) .



clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*
