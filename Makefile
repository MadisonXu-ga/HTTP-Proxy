TARGETS=proxy

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

proxy: main.cpp proxy.cpp
	g++ -g -o $@ $<

