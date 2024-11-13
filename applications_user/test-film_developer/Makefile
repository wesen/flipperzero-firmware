CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -DDEBUG -DHOST -g -O0 \
           -I.. \
           -I../film_developer

# Source directories
FILM_DEV_DIR = ../film_developer
TEST_DIR = .

# Source files - exclude embedded-only files when building for host
TEST_SRCS = $(TEST_DIR)/main.cpp
FILM_DEV_SRCS = $(wildcard $(FILM_DEV_DIR)/movement/*.cpp) \
                $(wildcard $(FILM_DEV_DIR)/processes/*.cpp) \
                $(filter-out $(FILM_DEV_DIR)/film_developer.cpp $(FILM_DEV_DIR)/*_embedded.cpp, \
                $(wildcard $(FILM_DEV_DIR)/*.cpp))

SRCS = $(TEST_SRCS) $(FILM_DEV_SRCS)

# Object files (maintain directory structure)
OBJS = $(TEST_SRCS:.cpp=.o) \
       $(FILM_DEV_SRCS:.cpp=.o)

# Header dependencies
DEPS = $(OBJS:.o=.d)

# Target executable
TARGET = test_film_developer

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Source files used:"
	@echo $(SRCS)
	@echo "Film developer source files:"
	@echo $(FILM_DEV_SRCS)
	@echo "Test source files:"
	@echo $(TEST_SRCS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Rule to generate dependency files
%.d: %.cpp
	@$(CXX) $(CXXFLAGS) -MM -MT $(@:.d=.o) $< > $@

# Include generated dependency files
-include $(DEPS)

# Compilation rule
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(DEPS) 