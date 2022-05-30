objects = geoseq_unordered_flatmap56.o geoseq_benchmark.o

geoseq_benchmark : $(objects)
	g++ -Wall -o geoseq_benchmark $(objects) -isystem benchmark/include -Lbenchmark/build/src -lbenchmark -lpthread -O3 -lm
	gcc -Wall -Wextra -g -o geoseq_test geoseq_unordered_flatmap56.c geoseq_test.c -fsanitize=address -lm
	make clean

geoseq_unordered_flatmap56.o : geoseq_unordered_flatmap56.c geoseq_unordered_flatmap56.h
	gcc -Wall -c geoseq_unordered_flatmap56.c -O3

geoseq_benchmark.o : geoseq_benchmark.cpp geoseq_unordered_flatmap56.h
	g++ -Wall -c geoseq_benchmark.cpp -O3

.PHONY : clean
clean :
	rm $(objects)
