// first assignment of 6040 in Clemson
// Time:2015-09-08
//
// Author: RUI CHANG
// Email: rchang@g.clemson.edu
#include <cstdlib>
#include <iostream>
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
int channels = 0;
int ochannels = 0;
char* filename_write = NULL;
unsigned char* pixels = NULL;
unsigned char*redPixels = NULL;
unsigned char*greenPixels = NULL;
unsigned char*bluePixels = NULL ;
unsigned char* pixels_write = NULL;
unsigned char* displayPixels = NULL;
unsigned char* h_f_pixels = NULL;
unsigned char* v_f_pixels = NULL;
char* filename_read = NULL;
int flag = 0 ;
int flag_i = 0 ;
int flag_r = 0 ;
int flag_g = 0 ; 
int flag_b = 0 ;
void handleKey(unsigned char key,int x,int y);
void horizonFlip()
{
	int width = xres*channels;
	unsigned char tmp;
	h_f_pixels = displayPixels;
	for(int j=0;j<yres-1;j++){
		for(int k =0 ; k < channels ; k++){
			for(int i =0 ;i < xres/2; i++){
				 tmp = h_f_pixels[j*width+i*channels+k];
			         h_f_pixels[j*width+i*channels+k] = h_f_pixels[j*width+(xres-1-i)*channels+k];
				 h_f_pixels[j*width+(xres-1-i)*channels+k]= tmp;
			}
		}
	}
	displayPixels = h_f_pixels;
}
void verticalFlip()
{
	v_f_pixels = displayPixels;
	unsigned char tmp;
	int width = xres*channels;
	for(int j = 0; j < yres/2 ; j++){
		for(int i =0; i < width; i++){
			 tmp = v_f_pixels[j*width+i];
			 v_f_pixels[j*width+i] = v_f_pixels[(yres-j-1)*width+i];
			 v_f_pixels[(yres-j-1)*width+i] = tmp ;
		}
	}
	displayPixels = v_f_pixels;
}
void save_greyscale_pixels(){
    int i,j;
	for(i = 0; i < xres*yres; i++){
		redPixels[i] = pixels[i*channels];		
		greenPixels[i+1] = pixels[i*channels+1];
		bluePixels[i+2] = pixels[i*channels+2];
	}
}

void displayImage(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	glPixelZoom(1,-1);// set display direction                      
	glRasterPos2i(0,yres-1);// set display start point
   	//displayPixels = pixels;
	if (flag_r == 1){
		displayPixels = redPixels;
		glDrawPixels(xres,yres,GL_LUMINANCE,GL_UNSIGNED_BYTE,displayPixels);
	}
	if (flag_g == 1){
		displayPixels = greenPixels;
		glDrawPixels(xres,yres,GL_LUMINANCE,GL_UNSIGNED_BYTE,displayPixels);
	} 
	if (flag_b == 1){
		displayPixels = bluePixels;
		glDrawPixels(xres,yres,GL_LUMINANCE,GL_UNSIGNED_BYTE,displayPixels);
	}
       	if(channels == 3 && flag_r == 0 && flag_g == 0 && flag_b == 0 ){
       		glDrawPixels(xres,yres,GL_RGB,GL_UNSIGNED_BYTE,displayPixels);
	}
        if(channels == 4 && flag_r == 0 && flag_g == 0 && flag_b == 0){
       		glDrawPixels(xres,yres,GL_RGBA,GL_UNSIGNED_BYTE,displayPixels);
   }
   glFlush();
}
bool freeMemo()// when procedure ends,free the memory
{
  	delete pixels;
	delete displayPixels;
	delete redPixels;
	delete bluePixels;
	delete greenPixels;
	delete bluePixels;
	return true;
}
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

	//TypeDesc::UINT8 maps the data into a desired type (unsigned char),
	//even if it wasn’t originally of that type
	if (!in->read_image (TypeDesc::UINT8, pixels)) {
		std::cerr << "Could not read pixels from " << filename_read;
		std::cerr << ", error = " << in->geterror() << std::endl;
		delete in;
		exit(-1); 
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


bool write()               //read image frome frame buffer and write it
{
	if(channels == 1){
		channels =  3;
	   	pixels_write = new unsigned char[xres*yres*channels];
		glReadPixels(0,0,xres,yres,GL_RGB,GL_UNSIGNED_BYTE,pixels_write);
	}
	if(channels == 3){
		pixels_write = new unsigned char[xres*yres*channels];
	    glReadPixels(0,0,xres,yres,GL_RGB,GL_UNSIGNED_BYTE,pixels_write);
	}
	else if (channels == 4){
		pixels_write = new unsigned char[xres*yres*channels];
        glReadPixels(0,0,xres,yres,GL_RGBA,GL_UNSIGNED_BYTE,pixels_write);
	}
	std::cout << "read buffer start" << std::endl;
	ImageOutput *out = ImageOutput::create(filename_write);
	if (!out) {
		std::cerr << "Could not create: " << geterror();
		exit(-1);
	}
	//create the ImageSpec that describes how you will write the output data
	unsigned char tmp;
	int i,j,c;
	int width = xres*channels;
	for(j = 0; j < yres/2 ; j++){
		for(i =0; i < width; i++){
			 tmp = pixels_write[j*width+i];
			 pixels_write[j*width+i] = pixels_write[(yres-j-1)*width+i];
			 pixels_write[(yres-j-1)*width+i] = tmp ;
		}
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
	delete pixels_write;
	return true;
}
void handleKey(unsigned char key,int x,int y){
        switch(key){
        case 'w':		
		case 'W': write(); break;
		case 'f':
		case 'F': horizonFlip();break;           
		case 'i':
		case 'I': verticalFlip(); break;           
		case 'r': {flag_r = 1;flag_b=0; flag_g=0;channels =1;} break;
		case 'g': {flag_g = 1;flag_r=0; flag_b =0;channels =1;}break;
		case 'b': {flag_b = 1;flag_g=0; flag_r=0;channels =1;} break;
		case 'o':
		case 'O': {flag_b = 0;flag_g=0; flag_r=0;channels = ochannels;displayPixels = pixels;} break;
		case 'q':	                   // q - quit
		case 'Q':// freeMemo();	
		case 27: exit(0);		           // esc - quit
		default: return;		                   // not a valid key -- just ignore i     
	}
        glutPostRedisplay();
}
/*void mouseClick(int btn, int state, int xp, int yp)
{ 
	if(state == 1)
		{
			cout<<"(x,y)"<< '='<<'('<<xp<<','<<yres-yp<<")"<<endl;
			if(channels == 3)
			{
			cout<<"red= "<< (unsigned int)pixels[(yp)*xres*channels +xp*channels]<<" green= "<<(unsigned int)pixels[(yp)*xres*channels +xp*channels + 1]<<" blue= "<<(unsigned int)pixels[(yp)*xres*channels + xp*channels +2]<<endl;
			}
			if(channels == 4)
			{
			cout<<"red= "<< (unsigned int)pixels[(yp)*xres*channels +xp*channels]<<" green= "<<(unsigned int)pixels[(yp)*xres*channels +xp*channels + 1]<<" blue= "<<(unsigned int)pixels[(yp)*xres*channels + xp*channels +2]<<endl;
			}
		}
}*/

void mouseClick(int btn, int state, int xp, int yp)
{ 
	if(state == 1)
		{
			cout<<"(x,y)"<< '='<<'('<<xp<<','<<yres-yp<<")"<<endl;
			if(channels == 3)
			{
			cout<<"red= "<< (float)pixels[(yp)*xres*channels +xp*channels]/255<<" green= "<<(float)pixels[(yp)*xres*channels +xp*channels + 1]/255<<" blue= "<<(float)pixels[(yp)*xres*channels + xp*channels +2]/255<<endl;
			}
			if(channels == 4)
			{
			cout<<"red= "<< (float)pixels[(yp)*xres*channels +xp*channels]/255<<" green= "<<(float)pixels[(yp)*xres*channels +xp*channels + 1]/255<<" blue= "<<(float)pixels[(yp)*xres*channels + xp*channels +2]/255<<endl;
			}
		}
}
int main(int argc, char** argv) {
	if (argc < 2 || argc >3) {
		std::cerr << "Usage: " << argv[0] << " [filename] " << std::endl;
		exit(1);
	}
	h_f_pixels = new unsigned char[xres*yres*channels];
    v_f_pixels = new unsigned char[xres*yres*channels];
	filename_read = argv[1];
	filename_write = argv[2];
    read();
	redPixels = new unsigned char[xres*yres];
	greenPixels = new unsigned char[xres*yres];
	bluePixels = new unsigned char[xres*yres];
	save_greyscale_pixels();
   	displayPixels = pixels;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(xres, yres);
	glutCreateWindow("display");
	glutDisplayFunc(displayImage);	  // display callback
	glutKeyboardFunc(handleKey);	  // keyboard callback
	glutMouseFunc(mouseClick);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, xres, 0, yres);
	glClearColor(1, 1, 1, 1);
	glutMainLoop();
	freeMemo();
    return 0;
}
