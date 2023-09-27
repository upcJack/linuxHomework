objects = main.o blkConsumer.o blkProducer.o  \
		  buffer.o setNoblocking.o global.o

targets = parallel_copy

#.PHONY: 
#	all clean

all: $(targets)

parallel_copy: main.o blkConsumer.o  blkProducer.o \
	buffer.o setNoblocking.o global.o
	$(CXX)  $(CXXFLAGS) -o $@ $^ -lpthread

#-rm *.gch

%.o: %.cpp 
	$(CXX) $(CPPFLAGS) -O2 -c $^ -D_THREAD_SAFE


clean:
	-rm $(objects) $(targets)
