#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <math.h>

#include <CL/cl.h>

#include "bitmap.h"
#include "oclHelper.h"
#include "medianFilter.h"
#include "param.h"
1234
#define PIX_PER_KP 10  //pre_allocate buffers for keypoints

/* Global Variables */
cl_kernel mKernel_local_maxmin, mKernel_interp_keypoint, mKernel_compute_gradient_orientation,\
			mKernel_orientation_assignment, mKernel_descriptor;


const float global_sigmaRatio = pow(2.0, (1.0 / par_Scales));
const int global_shape[2] = {386, 217}; // a size of image which is read from "scipy.misc.imread" function
int global_width, global_height;
oclHardware hardware;
oclSoftware software;

void checkErrorStatus(cl_int error, const char* message)
{
  if (error != CL_SUCCESS)
  {
	printf("the error detected by checkErrorStatus()\n");
    printf("%s\n", message) ;
    printf("%s\n", oclErrorCode(error)) ;
    exit(0) ;
  }
}

/*
 *  input_type:
 *   It must have space between two arguments.
 *   ex) "%f %f %d %cl"
 */
void setArguments(cl_kernel mKernel, char *input_type, ...) {
	va_list ap;
	cl_int err = 0;
	int num = 0;

	va_start(ap, input_type);
    char *ptr = strtok(input_type, " ");

    while (ptr != NULL)
    {
        if(!strcmp(ptr, "%f")) {
        	float *temp = (float *)va_arg(ap, double*);
    		err = clSetKernelArg(mKernel, num, sizeof(float), temp);
    		checkErrorStatus(err, "Unable to set argument");
        }
        else if(!strcmp(ptr, "%d")) {
        	int *temp = (int *)va_arg(ap, int*);
    		err = clSetKernelArg(mKernel, num, sizeof(int), temp);
    		checkErrorStatus(err, "Unable to set argument");
        }
        else if(!strcmp(ptr, "%cl")) {
        	cl_mem *temp = (cl_mem *)va_arg(ap, cl_mem*);
    		err = clSetKernelArg(mKernel, num, sizeof(cl_mem), temp);
    		checkErrorStatus(err, "Unable to set argument");
        }
        ptr = strtok(NULL, " ");
        num += 1;
    }

	va_end(ap);
}



/*
 *  sift_pyocl, plan.py functions
 */

void __init__(shape, dtype) {

}

void _one_octave(int octave) {
	float prevSigma = par_InitSigma, sigma;
	int kpsize32 = floor((global_shape[0] * global_shape[1]) / PIX_PER_KP);
	int octsize = pow(2, octave);
	int last_start = 0;
	int scale, newcnt;
	int wgsize2;//??? not 'int'
	int procsize2;

	for(scale=0; scale<par_Scales+2; scale++) {
		sigma = prevSigma * sqrt(pow(global_sigmaRatio, 2) - 1.0);

	// Calculate gaussian blur and DoG

		//_gaussian_convolution(buffers[scale], buffers[scale + 1], sigma, octave);//??
		prevSigma *= global_sigmaRatio;
		//combine( //???
	}

	// Define Variables
	size_t globalSize[3] = { 1, 1, 1 } ;
	size_t localSize[3] = { 1, 1, 1} ;
	cl_event seq_complete ;
	// Initialize OpenCL buffers with pointers to allocated memory
	cl_mem DoGToDevice ;
	cl_mem KpFromDevice ;

	for(scale=1; scale<par_Scales+1; scale++) {
		/*
		* local_maxmin() Kernel function Execution
		*/
		  temp_string = new char[129];
		  strcpy(temp_string, "%cl %cl %d %f %d %f %f %cl %d %d %d %d");
		  //???/DoG , Kp, counter need setting
		  setArguments(mKernel_local_maxmin, temp_string, \
				  &DoGToDevice, &KpFromDevice, &(int)par_border_dist, &(float)par_peak_thresh, &octsize, &(float)par_EdgeThresh0,\
				  &(float)par_EdgeThresh, &counterToDevice, &kpsize32, &scale, &global_width, &global_height);

		  // Actually start the kernels on the hardware
		  std::cout<<"Start the kernels...\n";
		  err = clEnqueueNDRangeKernel(hardware.mQueue, mKernel_local_maxmin, 1, NULL,\
					       globalSize, localSize, 0, NULL, &seq_complete) ;
		  checkErrorStatus(err, "Unable to enqueue NDRange") ;

		  // Wait for kernel to finish
		  std::cout<<"Wait for kernel to finish...\n";
		  clWaitForEvents(1, &seq_complete) ;

		  // Read back the image from the kernel
		  std::cout << "Reading output image and writing to file...\n";
		  err = clEnqueueReadBuffer(hardware.mQueue, KpFromDevice, CL_TRUE,  0, Kp_1_size,\
					    Kp_1, 0, NULL, &seq_complete) ;
		  checkErrorStatus(err, "Unable to enqueue read buffer") ;
		  clWaitForEvents(1, &seq_complete) ;

		  /*
		   * interp_keypoint() Kernel function Execution
		   */
		  //Write to input buffer
		  err = clEnqueueWriteBuffer(hardware.mQueue, DoGToDevice, CL_TRUE, 0,\
		 			     DoGs_size, DoGs, 0, NULL, NULL) ;
		  checkErrorStatus(err, "Unable to enqueue write buffer") ;

		  err = clEnqueueWriteBuffer(hardware.mQueue, KpFromDevice, CL_TRUE, 0,\
				  	  	 Kp_1_size, Kp_1, 0, NULL, NULL) ;
		  checkErrorStatus(err, "Unable to enqueue write buffer") ;

		  //Set arguments
		  int start_keypoints = 45, end_keypoints = 87;
		  float InitSigma = 1.6;

		  temp_string = new char[129];
		  strcpy(temp_string, "%cl %cl %d %d %f %f %d %d");
		  setArguments(mKernel_interp_keypoint, temp_string, \
				  &DoGToDevice, &KpFromDevice, &last_start, &end_keypoints,\
				  &(float)par_peak_thresh, &(float)par_InitSigma, &global_width, &global_height);

		  // Actually start the kernels on the hardware
		  std::cout<<"Start the kernels...\n";
		  err = clEnqueueNDRangeKernel(hardware.mQueue, mKernel_interp_keypoint, 1, NULL,\
					       globalSize, localSize, 0, NULL, &seq_complete) ;
		  checkErrorStatus(err, "Unable to enqueue NDRange") ;
		  clWaitForEvents(1, &seq_complete) ;

		  // Read back the image from the kernel
		  std::cout << "Reading output image and writing to file...\n";
		  err = clEnqueueReadBuffer(hardware.mQueue, KpFromDevice, CL_TRUE, 0, Kp_1_size,\
					    Kp_1, 0, NULL, &seq_complete) ;
		  checkErrorStatus(err, "Unable to enqueue read buffer") ;
		  clWaitForEvents(1, &seq_complete) ;

		  /*
		   *  self._compact()
		   */
		  newcnt = _compact(last_start);

		  /*
		   * Rescale all images to populate all octaves
		   */
		  /*
		   * compute_gradient_orientation() Kernel function Execution
		   */
		  //Write to input buffer
		  //??? igray, grad, ori should be initialized.
		  //Set arguments
		  temp_string = new char[129];
		  strcpy(temp_string, "%cl %cl %cl %d %d");
		  setArguments(mKernel_compute_gradient_orientation, temp_string, \
				  , , ,\//igray, grad, ori
				  &global_width, &global_height);

		  // Actually start the kernels on the hardware
		  std::cout<<"Start the kernels...\n";
		  err = clEnqueueNDRangeKernel(hardware.mQueue, mKernel_compute_gradient_orientation, 1, NULL,\
						   globalSize, localSize, 0, NULL, &seq_complete) ;
		  checkErrorStatus(err, "Unable to enqueue NDRange") ;
		  clWaitForEvents(1, &seq_complete) ;

		  // Read back the image from the kernel
		  /*std::cout << "Reading output image and writing to file...\n";
		  err = clEnqueueReadBuffer(hardware.mQueue, KpFromDevice, CL_TRUE, 0, Kp_1_size,\
						Kp_1, 0, NULL, &seq_complete) ;
		  checkErrorStatus(err, "Unable to enqueue read buffer") ;
		  clWaitForEvents(1, &seq_complete) ;*/

		  //Orientation assignement: 1D kernel, rather heavy kernel
		  if(newcnt && newcnt > last_start) {
			  wgsize2 = global_kernels[file_to_use];//??? plan.py 719
			  procsize2 = int(newcnt * wgsize2[0]);

			  /*
			   * orientation_assignment() Kernel function Execution
			   */
			  //Write to input buffer
			  //keypoint, grad, ori, counter should be initialized.
			  //Set arguments
			  temp_string = new char[129];
			  strcpy(temp_string, "%cl %cl %cl %cl %d %f %d %d %d %d %d");
			  setArguments(mKernel_orientation_assignment, temp_string, \
					  , , , ,\//keypoint, grad, ori, counter
					  &octsize, &par_OriSigma, &kpsize32, &last_start, &newcnt, &width, &height);

			  // Actually start the kernels on the hardware
			  std::cout<<"Start the kernels...\n";
			  err = clEnqueueNDRangeKernel(hardware.mQueue, mKernel_orientation_assignment, 1, NULL,\
							   globalSize, localSize, 0, NULL, &seq_complete) ;
			  checkErrorStatus(err, "Unable to enqueue NDRange") ;
			  clWaitForEvents(1, &seq_complete) ;

			  // Read back the image from the kernel
			  /*std::cout << "Reading output image and writing to file...\n";
			  err = clEnqueueReadBuffer(hardware.mQueue, KpFromDevice, CL_TRUE, 0, Kp_1_size,\
							Kp_1, 0, NULL, &seq_complete) ;
			  checkErrorStatus(err, "Unable to enqueue read buffer") ;
			  clWaitForEvents(1, &seq_complete) ;*/


			  //??? plan.py 713, evt_cp = pyopencl.enqueue_copy(self.queue, self.cnt, self.buffers["cnt"].data)
			  newcnt = global_cnt[0];

			  wgsize2 = global_kernels[file_to_use];//???tuple........
			  procsize2 = int(newcnt * wgsize2[0]), wgsize2[1], wgsize2[2];
			  try{
				  /*
				   * descriptor() Kernel function Execution
				   */
				  //Write to input buffer
				  //keypoint, descriptor, grad, ori, keypoints_end should be initialized.
				  //Set arguments
				  temp_string = new char[129];
				  strcpy(temp_string, "%cl %cl %cl %cl %d %d %cl %d %d");
				  setArguments(mKernel_orientation_assignment, temp_string, \
						  , , , ,\//keypoint, descriptor, grad, ori
						  &octsize, &last_start, , &width, &height);//keypoints_end

				  // Actually start the kernels on the hardware
				  std::cout<<"Start the kernels...\n";
				  err = clEnqueueNDRangeKernel(hardware.mQueue, mKernel_orientation_assignment, 1, NULL,\
								   globalSize, localSize, 0, NULL, &seq_complete) ;
				  checkErrorStatus(err, "Unable to enqueue NDRange") ;
				  clWaitForEvents(1, &seq_complete) ;

				  // Read back the image from the kernel
				  /*std::cout << "Reading output image and writing to file...\n";
				  err = clEnqueueReadBuffer(hardware.mQueue, KpFromDevice, CL_TRUE, 0, Kp_1_size,\
								Kp_1, 0, NULL, &seq_complete) ;
				  checkErrorStatus(err, "Unable to enqueue read buffer") ;
				  clWaitForEvents(1, &seq_complete) ;*/
			  }
			  catch(int e){
				  //???743 line~769
			  }
			  //??? 776 line  evt_cp = pyopencl.enqueue_copy(self.queue, self.cnt, self.buffers["cnt"].data)
			  last_start = global_cnt[0];
		  }
	}//for() end

	/*
	 * Rescale all images to populate all octaves
	 */
	if(octave < global_octave_max - 1) {
		/*
		* shrink() Kernel function Execution
		*/
	}
}

/*
 *	Compact the vector of keypoints starting from start
 *
 *  @param start: start compacting at this adress. Before just copy
 *	@type start: numpy.int32
 */
int _compact(int start) {
	int wgsize, kpsize32;
	wgsize = global_max_workgroup_size; //(max(self.wgsize[0]),) #TODO: optimize
	kpsize32 = global_kpsize;
	cp0_evt = pyopencl.enqueue_copy(self.queue, self.cnt, self.buffers["cnt"].data)
	kp_counter = self.cnt[0]
	procsize = calc_size((self.kpsize,), wgsize)

	if kp_counter > 0.9 * self.kpsize:
	 logger.warning("Keypoint counter overflow risk: counted %s / %s" % (kp_counter, self.kpsize))
	logger.info("Compact %s -> %s / %s" % (start, kp_counter, self.kpsize))
	self.cnt[0] = start
	cp1_evt = pyopencl.enqueue_copy(self.queue, self.buffers["cnt"].data, self.cnt)
	evt = self.programs["algebra"].compact(self.queue, procsize, wgsize,
				 self.buffers["Kp_1"].data,  # __global keypoint* keypoints,
				 self.buffers["Kp_2"].data,  # __global keypoint* output,
				 self.buffers["cnt"].data,  # __global int* counter,
				 start,  # int start,
				 kp_counter)  # int nbkeypoints
	cp2_evt = pyopencl.enqueue_copy(self.queue, self.cnt, self.buffers["cnt"].data)
	# swap keypoints:
	self.buffers["Kp_1"], self.buffers["Kp_2"] = self.buffers["Kp_2"], self.buffers["Kp_1"]
	# memset buffer Kp_2
	#        self.buffers["Kp_2"].fill(-1, self.queue)
	mem_evt = self.programs["memset"].memset_float(self.queue, calc_size((4 * self.kpsize,), wgsize), wgsize, self.buffers["Kp_2"].data, numpy.float32(-1), numpy.int32(4 * self.kpsize))
	if self.profile:
	 self.events += [("copy cnt D->H", cp0_evt),
					 ("copy cnt H->D", cp1_evt),
					 ("compact", evt),
					 ("copy cnt D->H", cp2_evt),
					 ("memset 2", mem_evt)
					 ]
	return self.cnt[0]
}

int main(int argc, char* argv[])
{
    //TARGET_DEVICE macro needs to be passed from gcc command line
#if defined(SDX_PLATFORM) && !defined(TARGET_DEVICE)
  #define STR_VALUE(arg)      #arg
  #define GET_STRING(name) STR_VALUE(name)
  #define TARGET_DEVICE GET_STRING(SDX_PLATFORM)
#endif
  //TARGET_DEVICE macro needs to be passed from gcc command line
  const char *target_device_name = TARGET_DEVICE;
  
  const int width = 217 ; // Default size
  const int height = 386 ; // Default size
  char *temp_string;

  // Allocate memory in host memory
  int image_size = height*width;
  int DoGs_size = 5*height*width*sizeof(float);
  int Kp_1_size = int(height*width/10)*4*sizeof(float);
  float ***DoGs;
  DoGs = new float **[5];
  for (int x=0 ; x<5; x++){
	  DoGs[x] = new float *[height];
	  for(int y=0; y<height; y++){
		  DoGs[x][y] = new float [width];
	  }
  }
  float **Kp_1;
  Kp_1 = new float *[(int)(Kp_1_size/sizeof(float))];
  for (int x=0 ; x<(int)(Kp_1_size/sizeof(float)); x++){
	  Kp_1[x] = new float [4];
  }

  //file read
  //read DoGs
  FILE *fl;
  fl = fopen("/home/ncl/eyoh/pyocl_DoGs.txt", "r");

  if(fl == NULL)
  {
	  std::cout<<"No DoGs file\n";
	  return -1;
  }

  std::cout<<"Before for loop\n";
  for(int s=0; s<5; s++){
	  for(int r=0; r<height; r++){
		  for(int c=0; c<width; c++){
			  if(!feof(fl)){
				  fscanf(fl, "%f", &(DoGs[s][r][c]));
			  }
		  }
	  }
  }

  // Set up OpenCL hardware and software constructs
  std::cout << "Setting up OpenCL hardware and software...\n";
  cl_int err = 0 ;
  const char* xclbinFilename = argv[2] ;

  oclHardware hardware = getOclHardware(CL_DEVICE_TYPE_ACCELERATOR, target_device_name) ;
  oclSoftware software;
  memset(&software, 0, sizeof(oclSoftware));
  strcpy(software.mFileName, xclbinFilename) ;
  strcpy(software.mCompileOptions, "-g -Wall") ;

  //"get_software()" start
  cl_device_type deviceType = CL_DEVICE_TYPE_DEFAULT;
  err = clGetDeviceInfo(hardware.mDevice, CL_DEVICE_TYPE, sizeof(deviceType), &deviceType, 0);
  if ( err != CL_SUCCESS) {
	  std::cout << oclErrorCode(err) << "\n";
	  return -1;
  }

  unsigned char *kernelCode = 0;
  std::cout << "Loading " << software.mFileName << "\n";

  int size = loadFile2Memory_global(software.mFileName, (char **) &kernelCode);
  if (size < 0) {
	  std::cout << "Failed to load kernel\n";
	  return -2;
  }

  if (deviceType == CL_DEVICE_TYPE_ACCELERATOR) {
	  size_t n = size;
	  software.mProgram = clCreateProgramWithBinary(hardware.mContext, 1, &hardware.mDevice, &n,
													(const unsigned char **) &kernelCode, 0, &err);
  }
  else {
	  software.mProgram = clCreateProgramWithSource(hardware.mContext, 1, (const char **)&kernelCode, 0, &err);
  }
  if (!software.mProgram || (err != CL_SUCCESS)) {
	  std::cout << oclErrorCode(err) << "\n";
	  return -3;
  }

  //Below codes are different by functions
  mKernel_local_maxmin = clCreateKernel(software.mProgram, "local_maxmin", NULL);
  mKernel_interp_keypoint = clCreateKernel(software.mProgram, "interp_keypoint", NULL);
  mKernel_compute_gradient_orientation = clCreateKernel(software.mProgram, "compute_gradient_orientation", NULL);
  mKernel_orientation_assignment = clCreateKernel(software.mProgram, "orientation_assignment", NULL);
  mKernel_descriptor = clCreateKernel(software.mProgram, "descriptor", NULL);

  if (mKernel_local_maxmin == 0 || mKernel_interp_keypoint == 0 || mKernel_compute_gradient_orientation == 0\
		  || mKernel_orientation_assignment == 0 || mKernel_descriptor == 0) {
	  std::cout << oclErrorCode(err) << "\n";
	  return -4;
  }

  delete [] kernelCode;
  //"get_software()" end




  /*
   * Function Start!!!!!!!!!!
   */
	// Define Variables
	size_t globalSize[3] = { 1, 1, 1 } ;
	size_t localSize[3] = { 1, 1, 1} ;
	cl_event seq_complete ;

	int octsize = 2;

	DoGToDevice = clCreateBuffer(hardware.mContext,
				 CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
				 DoGs_size,
				 DoGs,
				 &err) ;
	checkErrorStatus(err, "Unable to create read buffer") ;

	KpFromDevice = clCreateBuffer(hardware.mContext,
				   CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
				   Kp_1_size,
				   Kp_1,
				   &err) ;
	checkErrorStatus(err, "Unable to create write buffer") ;

  // Send the image to the hardware
  std::cout << "Writing input image to buffer...\n";

  err = clEnqueueWriteBuffer(hardware.mQueue,
 			     DoGToDevice,
 			     CL_TRUE,
 			     0,
 			     DoGs_size,
 			     DoGs,
 			     0,
 			     NULL,
 			     NULL) ;
  checkErrorStatus(err, "Unable to enqueue write buffer") ;

  int border_dist = 5;
  float peak_thresh = 3.4;

  float EdgeThresh0 = 0.08;
  float EdgeThresh = 0.06;
  cl_mem counterToDevice;
  int* counter = new int[1];
  counter[0] = 45;

  counterToDevice = clCreateBuffer(hardware.mContext,
				 CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
				 sizeof(int),
				 counter,
				 &err) ;
  checkErrorStatus(err, "Unable to create read buffer") ;

  err = clEnqueueWriteBuffer(hardware.mQueue,
  			     counterToDevice,
  			     CL_TRUE,
  			     0,
  			     sizeof(int),
  			     counter,
  			     0,
  			     NULL,
  			     NULL) ;
   checkErrorStatus(err, "Unable to enqueue write buffer") ;

  int nb_keypoints = 8376;
  int scale = 2;

  temp_string = new char[129];
  strcpy(temp_string, "%cl %cl %d %f %d %f %f %cl %d %d %d %d");

  setArguments(mKernel_local_maxmin, temp_string, \
		  &DoGToDevice, &KpFromDevice, &border_dist, &peak_thresh, &octsize, &EdgeThresh0,\
		  &EdgeThresh, &counterToDevice, &nb_keypoints, &scale, &width, &height);


  // Actually start the kernels on the hardware
  std::cout<<"Start the kernels...\n";
  err = clEnqueueNDRangeKernel(hardware.mQueue,
		  	  	   mKernel_local_maxmin,
			       1,
			       NULL,
			       globalSize,
			       localSize,
			       0,
			       NULL,
			       &seq_complete) ;
  checkErrorStatus(err, "Unable to enqueue NDRange") ;

  // Wait for kernel to finish
  std::cout<<"Wait for kernel to finish...\n";
  clWaitForEvents(1, &seq_complete) ;

  // Read back the image from the kernel
  std::cout << "Reading output image and writing to file...\n";
  err = clEnqueueReadBuffer(hardware.mQueue,
			    KpFromDevice,
			    CL_TRUE,
			    0,
			    Kp_1_size,
			    Kp_1,
			    0,
			    NULL,
			    &seq_complete) ;
  checkErrorStatus(err, "Unable to enqueue read buffer") ;

  clWaitForEvents(1, &seq_complete) ;

  /*
   * interp_keypoint() function Execution
   */

  //Write to input buffer
  err = clEnqueueWriteBuffer(hardware.mQueue,
 			     DoGToDevice,
 			     CL_TRUE,
 			     0,
 			     DoGs_size,
 			     DoGs,
 			     0,
 			     NULL,
 			     NULL) ;
  checkErrorStatus(err, "Unable to enqueue write buffer") ;

  err = clEnqueueWriteBuffer(hardware.mQueue,
		  	     KpFromDevice,
 			     CL_TRUE,
 			     0,
				 Kp_1_size,
 			     Kp_1,
 			     0,
 			     NULL,
 			     NULL) ;
  checkErrorStatus(err, "Unable to enqueue write buffer") ;

  //Set arguments
  int start_keypoints = 45, end_keypoints = 87;
  float InitSigma = 1.6;

  temp_string = new char[129];
  strcpy(temp_string, "%cl %cl %d %d %f %f %d %d");
  setArguments(mKernel_interp_keypoint, temp_string, \
		  &DoGToDevice, &KpFromDevice, &start_keypoints, &end_keypoints,\
		  &peak_thresh, &InitSigma, &width, &height);

  // Actually start the kernels on the hardware
  std::cout<<"Start the kernels...\n";
  err = clEnqueueNDRangeKernel(hardware.mQueue,
		  	  	   mKernel_interp_keypoint,
			       1,
			       NULL,
			       globalSize,
			       localSize,
			       0,
			       NULL,
			       &seq_complete) ;

  checkErrorStatus(err, "Unable to enqueue NDRange") ;

  // Wait for kernel to finish
  std::cout<<"Wait for kernel to finish...\n";
  clWaitForEvents(1, &seq_complete) ;

  // Read back the image from the kernel
  std::cout << "Reading output image and writing to file...\n";
  err = clEnqueueReadBuffer(hardware.mQueue,
			    KpFromDevice,
			    CL_TRUE,
			    0,
			    Kp_1_size,
			    Kp_1,
			    0,
			    NULL,
			    &seq_complete) ;

  checkErrorStatus(err, "Unable to enqueue read buffer") ;

  clWaitForEvents(1, &seq_complete) ;

  // Release software and hardware
  release(software) ;
  release(hardware) ;

  return 0 ;
}

/*
//write output Kp
FILE *fo;
fo = fopen("/home/ncl/eyoh/FPGA_Kp.txt","w");

if(fo == NULL)
{

	  std::cout<<"Cannot make output file\n";
	  return -1;
}

for( int k=0; k<Kp_1_size/sizeof(float); k++){
	  for (int k2=0 ;k2<4 ; k2++){
		  fprintf(fo, "%f\n", Kp_1[k][k2]);
	  }
}
fclose(fo);*/

/*
//Compare execution Results of GPU and FPGA
FILE *GPU_result, *FPGA_result;
float temp1, temp2;
GPU_result = fopen("/home/ncl/Wan/interp_Kp_1_copy.txt","r");
FPGA_result = fopen("/home/ncl/eyoh/FPGA_Kp.txt","r");


while(!feof(GPU_result) && !feof(FPGA_result)) {
	  fscanf(GPU_result, "%f ", &temp1);
	  fscanf(FPGA_result, "%f ", &temp2);
	  if(temp1 != temp2){
		  //printf("Result is not same! : %f %f\n", temp1, temp2);
	  }
}
fclose(GPU_result);
fclose(FPGA_result);*/
