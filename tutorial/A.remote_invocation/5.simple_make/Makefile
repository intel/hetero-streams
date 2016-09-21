# Check or set (if unset) target sink architecture, you can bypass this block
# =============================================================================
ifdef TARGET
    ifneq "$(TARGET)" "knc"
    ifneq "$(TARGET)" "x200"
        $(error [ERRR] Invalid TARGET value. Possible values are "knc" "x200")
    endif
    endif
else
    ifeq ($(shell cat /sys/class/mic/mic0/family),x100)
        TARGET := knc
    else ifeq ($(shell cat /sys/class/mic/mic0/family),x200)
        TARGET := x200
    else ifeq ($(shell cat /sys/class/mic/mic0/info/mic_family),x200)
        TARGET := x200
    else
        $(warning [WARN] TARGET variable is not set. Default x200 target will be use.)
        TARGET := x200
    endif
endif
# =============================================================================


ifeq "$(TARGET)" "knc"
# KNC
solution:
	icpc hello_hStreams_world_src_solution.cpp -o hello_hStreams_world_solution -I/usr/include/hStreams  -lhstreams_source
	icpc -fPIC -mmic -O3  hello_hStreams_world_sink.cpp -I/usr/include/hStreams  -o hello_hStreams_world_solution_mic.so   -shared

else ifeq "$(TARGET)" "x200"
# x200
solution:
	icpc hello_hStreams_world_src_solution.cpp -o hello_hStreams_world_solution -I/usr/include/hStreams  -lhstreams_source
	icpc -fPIC -xMIC-AVX512 -O3  hello_hStreams_world_sink.cpp -I/usr/include/hStreams  -o hello_hStreams_world_solution_x200.so   -shared

endif

clean:
	rm *.so hello_hStreams_world_solution
