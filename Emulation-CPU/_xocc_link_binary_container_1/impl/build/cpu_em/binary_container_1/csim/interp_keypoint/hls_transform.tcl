set xmlPath "/home/ncl/eyoh/test/Emulation-CPU/_xocc_link_binary_container_1/impl/kernels/interp_keypoint/interp_keypoint/kernel.xml"
transform -f -hls "/home/ncl/eyoh/test/Emulation-CPU/_xocc_link_binary_container_1/impl/build/cpu_em/binary_container_1/csim/interp_keypoint/interp_keypoint.clc.01.bc" -top=interp_keypoint -xcl-target=cpu -xcl-xmlinfo=$xmlPath -xcl-flatten -xcl-pipes -spir-runtime-support -xcl-ports-metadata -spir-link-builtins -always-inline -xcl-scalarize -xcl-localmem -xcl-constmem -xcl-globalmem -mem2reg -instcombine -dce -loop-simplify -reg2mem -spir-kernel-coarsening -reg2mem -mem2reg -instcombine -dce -simplifycfg -spir-lower-builtins -spir-link-builtins -always-inline -spir-remove-always-inlined -xcl-hls-lowering -strip-dead-prototypes -mem2reg -o interp_keypoint.clc.csim_cu.bc
exit
