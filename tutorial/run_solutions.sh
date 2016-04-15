echo  A.remote_invocation/1.hello_hStreams_world
pushd  A.remote_invocation/1.hello_hStreams_world > /dev/null
./solution_run.sh
popd
echo  A.remote_invocation/2.pass_scalar_args_get_result
pushd  A.remote_invocation/2.pass_scalar_args_get_result > /dev/null
./solution_run.sh
popd
echo  A.remote_invocation/3.pass_pointer
pushd  A.remote_invocation/3.pass_pointer > /dev/null
./solution_run.sh
popd
echo  B.using_streams/1.straight_line_code_host
pushd  B.using_streams/1.straight_line_code_host > /dev/null
./solution_run.sh
popd
echo  B.using_streams/2.task_code_host
pushd  B.using_streams/2.task_code_host > /dev/null
./solution_run.sh
popd
echo  B.using_streams/3.task_code_sink_buffer_sync
pushd  B.using_streams/3.task_code_sink_buffer_sync > /dev/null
./solution_run.sh
popd
echo  B.using_streams/4.cross_buffer_xfer
pushd  B.using_streams/4.cross_buffer_xfer > /dev/null
./solution_run.sh
popd
echo  C.tiling/0.compute_math_not_tiled_host
pushd  C.tiling/0.compute_math_not_tiled_host > /dev/null
./solution_run.sh
popd
echo  C.tiling/1.compute_math_tiled_host
pushd  C.tiling/1.compute_math_tiled_host > /dev/null
./solution_run.sh
popd
echo  C.tiling/2.compute_math_not_tiled_hstreams_naive
pushd  C.tiling/2.compute_math_not_tiled_hstreams_naive > /dev/null
./solution_run.sh
popd
echo  C.tiling/3.compute_math_not_tiled_hstreams_streaming
pushd  C.tiling/3.compute_math_not_tiled_hstreams_streaming > /dev/null
./solution_run.sh
popd
echo  C.tiling/4.compute_math_tiled_hstreams_streaming
pushd  C.tiling/4.compute_math_tiled_hstreams_streaming > /dev/null
./solution_run.sh
popd
echo  C.tiling/5.compute_math_tiled_hstreams_multicard
pushd  C.tiling/5.compute_math_tiled_hstreams_multicard > /dev/null
./solution_run.sh
popd
echo  C.tiling/6.compute_math_not_tiled_hstreams_host_multicard
pushd  C.tiling/6.compute_math_not_tiled_hstreams_host_multicard > /dev/null
./solution_run.sh
popd
