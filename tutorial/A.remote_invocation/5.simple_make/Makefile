solution:
	icpc hello_hStreams_world_src_solution.cpp -o hello_hStreams_world_solution -I/usr/include/hStreams  -lhstreams_source
	icpc -fPIC -mmic -O3  hello_hStreams_world_sink.cpp -I/usr/include/hStreams  -o hello_hStreams_world_solution_mic.so   -shared

clean:
	rm *.so hello_hStreams_world_solution

