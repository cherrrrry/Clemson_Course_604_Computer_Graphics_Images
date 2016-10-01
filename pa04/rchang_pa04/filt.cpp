// Forth assignment of 6040 in Clemson
// Time:2015-10-10
//
// Author: RUI CHANG
// Email: rchang@g.clemson.edu
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <math.h>
#include <OpenImageIO/imageio.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

using namespace std;
OIIO_NAMESPACE_USING
int xres = 0;
int yres = 0;
int N = 0;
int channels = 0;
int ochannels = 0;
char* filename_read = NULL;
char* kernelfile_name = NULL;
char* filename_write = NULL;
double ** filt = NULL;
unsigned char* pixels = NULL;
unsigned char* displayPixels = NULL;
unsigned char* oPixels = NULL;
unsigned char *pixels_write = NULL;
bool read()// read image and store it in pixels
{
	ImageInput *in = ImageInput::open(filename_read);
	if (!in) {
		std::cerr << "Could not create: " << geterror();
		exit(-1);
	}
	//after opening the image we can access 
	//information about it
	const ImageSpec &spec = in->spec();
	xres = spec.width;
	yres = spec.height;
	int dept = spec.depth;
	channels = spec.nchannels;
	ochannels = channels;

	std::cout << "x resolution: " << xres << std::endl;
	std::cout << "y resolution: " << yres << std::endl;
	std::cout << "d resolution: " << dept << std::endl;
	std::cout << "num channels: " << channels << std::endl;

	//declare memory, open and read it
	pixels = new unsigned char[xres*yres*channels]; 
	oPixels = new unsigned char[xres*yres*channels];
	
	//TypeDesc::UINT8 maps the data into a desired type (unsigned char),
	//even if it wasn’t originally of that type
	if (!in->read_image (TypeDesc::UINT8, pixels)) {
		std::cerr << "Could not read pixels from " << filename_read;
		std::cerr << ", error = " << in->geterror() << std::endl;
		delete in;
		exit(-1); 
	}
	for(int i = 0 ;i < yres ; i++){
		for(int j = 0 ; j< xres ; j++){
			for(int k = 0 ; k < channels ; k ++){
				oPixels[i*xres*channels + j*channels +k] = pixels[i*xres*channels+ j*channels +k];
			}
		}
	}

	if (!in->close ()) {
		std::cerr << "Error closing " << filename_read;
		std::cerr << ", error = " << in->geterror() << std::endl;
		delete in;
		exit(-1);
	}
	delete in;
   	return true;
}
bool readKernel(char* filename){
	ifstream kernelfile; 
	kernelfile.open(filename);  
	if (! kernelfile.is_open())  {
		cout << "Error opening file"; exit (1);
	}  
	kernelfile >> N;  
	filt = new double*[N];
	for(int i = 0 ; i < N ; i++){
		filt[i] = new double[N];
	}
	for(int i = 0 ; i < N ; i++){
		for(int j = 0 ; j < N ;j++){
			kernelfile >> filt[i][j];
			cout << filt[i][j]<<",";
		}
		cout <<"," <<endl;
	}
	kernelfile.close();
	return true;
}
void flipFilt(double ** kernel){//filp filter horizontally and vertically 
	double temp = 0;
	for(int i=0 ; i < N ;i++){
		for(int j=0 ; j < N ; j++){
			temp = kernel[i][j];
			kernel[i][j] = kernel[i][N-j-1];
			kernel[i][N-j-1] = temp;
		}
	}
	for(int i=0 ; i < N ;i++){
		for(int j=0 ; j < N ; j++){
			temp = kernel[i][j];
			kernel[i][j] = kernel[N-i-1][j];
			kernel[N-i-1][j] = temp;
		}
	}
	cout<<"flip done!"<<endl;
}
void resetPixels(unsigned char* srcPixels,unsigned char* desPixels){
//	desPixels = new unsigned char[xres*yres*channels];
		for(int i = 0 ;i < yres ; i++){
		for(int j = 0 ; j< xres ; j++){
			for(int k = 0 ; k < channels ; k ++){
				desPixels[i*xres*channels + j*channels +k] = srcPixels[i*xres*channels+ j*channels +k];
			}
		}
	}

}
void filtImage(unsigned char * image,double ** kernel){ //use kernel to convolve image
	double factor = 0;
	double posFactor = 0;
	double negFactor = 0;
	double sum = 0;
	flipFilt(kernel);
	unsigned char* tempPixels ;
	tempPixels = new unsigned char[xres*yres*channels];
	resetPixels(image,tempPixels);
	for(int y = 0 ; y < N ; y++){          //count the total number of kernel
		for(int x = 0 ; x < N ; x++){
			if(kernel[y][x] > 0){
				posFactor += kernel[y][x];
			}
			else {
				negFactor += (-kernel[y][x]);
			}
		}
	}
	cout<<"posFactor = "<<posFactor<<endl;
	cout<<"negFactor = "<<negFactor<<endl;
	if(posFactor > negFactor){
		factor = posFactor;
	}else{
		factor = negFactor;
	}
	if (factor == 0){
		factor = 1;
	}
	cout<< "factor = "<<factor << endl;
	cout<< "channels = "<<channels << endl;
	for(int y = 0 ; y < N ; y++){          //output the weights of kernel
		for(int x = 0 ; x < N ; x++){
			cout<<kernel[y][x]<<",";
		}
		cout <<"" <<endl;
	}
	for(int i = 0 ; i < yres ; i++){
		for(int j = 0 ; j < xres ; j++){
			for(int k = 0 ; k < channels; k++){
				for (int y = 0 ; y < N ; y++){
					for(int x = 0 ; x < N ; x++){
						double edgeH = i + y - N/2;
						double edgeW = j + x - N/2;
						if(edgeH>=0 && edgeH < yres && edgeW >=0 && edgeW < xres){
							sum += kernel[y][x]*tempPixels[(i+y-N/2)*xres*channels +(j+x-N/2)*channels + k];
						}
					}	
				}
				
				int result = (sum / factor);
				result = result>255? 255: result;
				result = result<0? 0: result;
				image[i*xres*channels+ j*channels + k] = result;
				sum = 0;
				result = 0;
			}
		}
	}
	delete tempPixels;
}	

double** gaberFilt( double theta , int sigma , double period ){//generate gabor filter
	double ** gfilt;
	N = 2 * sigma + 1;
	gfilt = new double * [2*sigma+1];
	for(int i = 0 ;i < 2*sigma+1 ; i++){
		gfilt[i]= new double[2*sigma +1];
	}
	double x = 0;
	double y = 0;
	double result=0;
	for( int i = 0; i < 2*sigma+1 ;i++ ){
		for(int j = 0 ; j < 2*sigma + 1; j++){
			x = (j - sigma)*cos(theta)+( i - sigma)*sin(theta);
			y = -(j - sigma)*sin(theta)+( i - sigma)*cos(theta);
			result = cos(2*3.14*x/period)*exp(-(x*x+y*y)/(2*sigma*sigma));			
			gfilt[i][j] = result;
			result = 0;
		}
	}

	return gfilt;
}
bool write()               // write image
{
	ImageOutput *out = ImageOutput::create(filename_write);
	if (!out) {
		std::cerr << "Could not create: " << geterror();
		exit(-1);
	}
   	ImageSpec spec_w (xres, yres, channels, TypeDesc::UINT8);
	out->open(filename_write, spec_w);
	//it is possible that this TypeDesc does not match the ImageSpec, 
	//in which case it will convert the raw data into the spec. 
	//But it MUST match the datatype of raw data
	out->write_image(TypeDesc::UINT8, pixels_write);
	std::cout << "write done" << std::endl;
	if (!out->close ()) {
		std::cerr << "Error closing " << filename_write;
		std::cerr << ", error = " << out->geterror() << std::endl;
		delete out;
		exit(-1);
    }
		return true;
}
void displayImage(){
	cout<< "start display"<<endl; 	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPixelZoom(1,-1);// set display direction                      
	glRasterPos2i(0,yres-1);// set display start point
	if (channels == 3){	
		glDrawPixels(xres,yres,GL_RGB,GL_UNSIGNED_BYTE,displayPixels);
	}
	if (channels == 4){
		glDrawPixels(xres,yres,GL_RGBA,GL_UNSIGNED_BYTE,displayPixels);
	}
	if (channels == 1){
		glDrawPixels(xres,yres,GL_LUMINANCE,GL_UNSIGNED_BYTE,displayPixels);
	}
	glFlush();
}
void handleKey(unsigned char key,int x,int y){
	switch(key){
		case 'c':
		case 'C': filtImage(displayPixels,filt);
				  break;
		case 'r':
		case 'R': resetPixels(oPixels,displayPixels);
				  break;// display callback
		case 'w':
		case 'W': {
					pixels_write = displayPixels ;
					write();
			      }			
				  break;
		case 'q':                      // q - quit
		case 'Q':// freeMemo(); 
		case 27: exit(0);                  // esc - quit
		default: return;                           // not a valid key -- just ignore i     
	 }
	glutPostRedisplay();
}
int main(int argc, char** argv) {
	if (argc >= 3 && argc <=4) {
		kernelfile_name = argv[1];
		filename_read = argv[2];
		if(argc == 4){
			filename_write = argv[3];
		}
		readKernel(kernelfile_name);        //read kernelfile X.filt
		read();
		displayPixels = pixels;
	}
	else if(strcmp(argv[1],"-g") == 0){
	//else if ( strcmp(argv[1],"-g")==0){
		double theta_ = atof(argv[2]);
		double sigma_ = atoi(argv[3]);
		double period_ = atof(argv[4]);
		filename_read = argv[5];
		printf("read_path:%s\n",filename_read);
		read();
		displayPixels = pixels;
		filt = gaberFilt(theta_,sigma_,period_); 
	    if(argc == 7){
			filename_write = argv[6];
		}
	}	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(xres, yres);
	glutCreateWindow("display");
	glutDisplayFunc(displayImage);    // display callback
	glutKeyboardFunc(handleKey);      // keyboard callback
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, xres, 0, yres);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER,0);
	glClearColor(1, 1, 1, 1);
	glutMainLoop();
	if(filt!=NULL){
		delete filt;
	}
	return 0;
}
