CC = g++
CFLAGS = -g -pthread
TARGET = proxy
SRCS = main.cpp cache.cpp proxy.cpp server.cpp request.cpp client.cpp response.cpp client_info.hpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(filter-out client_info.hpp, $(OBJS) $(TARGET))