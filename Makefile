# Makefile

CXX       := g++
CXXFLAGS  := -std=c++17 -O2 -Wall
TARGET    := solver
SRC       := solver.cpp

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

# The test target runs each .in through the solver and compares output,
# ignoring blank-line differences (-B) to avoid failures due to trailing newlines.
test: $(TARGET)
	@for in_file in tests/*.in; do \
		test_name=$$(basename $$in_file .in); \
		echo "Running test $$test_name..."; \
		./$(TARGET) < $$in_file > tests/$$test_name.actual; \
		if diff -u -B tests/$$test_name.out tests/$$test_name.actual; then \
			echo "Test $$test_name: PASS"; \
		else \
			echo "Test $$test_name: FAIL"; \
		fi; \
	done

clean:
	rm -f $(TARGET) tests/*.actual
