// third assignment of 6040 in Clemson
// Time:2015-09-26
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
#define maximum(x, y, z) ((x) > (y)? ((x) > (z)? (x) : (z)) : ((y) > (z)? (y) : (z)))
#define minimum(x, y, z) ((x) < (y)? ((x) < (z)? (x) : (z)) : ((y) < (z)? (y) : (z))) 
int xres = 0;
int yres = 0;
int channels = 0;
int ochannels = 0;
double k = 1;
bool pv = false;
char* filename_read = NULL;
char* filename_write = NULL;
unsigned char* pixels = NULL;
unsigned char* oRgbaPixels = NULL;
unsigned char* rgbaPixels = NULL;
double* hsvPixels = NULL;
unsigned char *pixels_write = NULL;
//double* pixels_write = NULL;
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
	channels = 4;
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
/*
Input RGB color primary values : r , g, and b on scale 0 − 255 Output HSV colors : h on scale 0−360, s and v on scale 0−1
*/
//#define maximum(x, y, z) ((x) > (y)? ((x) > (z)? (x) : (z)) : ((y) > (z)? (y) : (z)))
//#define minimum(x, y, z) ((x) < (y)? ((x) < (z)? (x) : (z)) : ((y) < (z)? (y) : (z))) 
void RGBtoHSV(int r, int g, int b, double &h, double &s, double &v){
	double red, green, blue; double max, min, delta;
	red=r/255.0;green=g/255.0;blue=b/255.0; /*r,g,bto0−1scale*/
	max = maximum(red, green, blue);
	min = minimum(red, green, blue);
	v = max; /*value is maximum of r, g, b */
	if (max == 0) { /* saturation and hue 0 if value is 0 */
		s = 0;
		h = 0; } 
	else {
			s = (max - min) / max;
			delta = max - min; 
			if (delta == 0) {
				h = 0; }
			else {
                if (red == max) {
                    h = (green - blue) / delta;
                    /*saturation is color purity on scale 0 − 1 */
                    /* hue doesn’t matter if saturation is 0 */
                    /* otherwise , determine hue on scale 0 − 360 */
                    }
				else if (green == max) {
					h = 2.0 + (blue - red) / delta;
                    } 
				else{/* (blue == max) */
					h = 4.0 + (red - green) / delta;
                    }
                h = h * 60.0;
                if(h < 0) {
                    h = h + 360.0;
                } 
			}
    } 
}


bool imgRGBtoImgHSV(unsigned char *p1,double *p2){
    channels = 4;
    for(int i = 0; i< xres * yres ; i++){
        RGBtoHSV(p1[i*channels],p1[i*channels+1],p1[i*channels+2],p2[i*channels],p2[i*channels+1],p2[i*channels+2]);
        p2[i*channels + 3] = p1[i*channels + 3]/255.0;
    }
	std::cout << "imgRGB --> imgHSV done" << std::endl;
    return true;
}


void threeToFour(unsigned char *p1,unsigned char *p2){//rgb to rgba
	
		for(int i = 0 ;i < xres*yres;i++){
			p2[i*4]=p1[i*channels];
			p2[i*4+1]=p1[i*channels+1];
			p2[i*4+2]=p1[i*channels+2];
			if(channels == 4){
				p2[i*4+3]=p1[i*channels+3];
			}else if(channels == 3){
				p2[i*4+3]=255;
			}
		}
}
void displayImage(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPixelZoom(1,-1);// set display direction                      
	glRasterPos2i(0,yres-1);// set display start point
	glDrawPixels(xres,yres,GL_RGBA,GL_UNSIGNED_BYTE,rgbaPixels);
	glFlush();
}
void handleKey(unsigned char key,int x,int y){
	switch(key){
		case 'q':                      // q - quit
		case 'Q':// freeMemo(); 
		case 27: exit(0);                  // esc - quit
		default: return;                           // not a valid key -- just ignore i     
	 }
}

void greenscale(double *p1,unsigned char *p2){
	channels = 4;
	for(int i = 0; i < xres*yres ;i++){
		if(80<p1[i*channels]&&p1[i*channels]<145/*&&?<p1[i*channels+1]<?&&?<p1[i*channels+2]=?*/){//-----------------------change parameters here!!!
			p2[i*channels+3]=0;
		}
	}
	std::cout << "greenscale matte done!" << std::endl;
}

void petroVlahos(unsigned char *p1,unsigned char *p2){//--------------------------   -pv  p1=src, p2=des
	channels = 4;
	for(int i = 0; i<xres*yres ;i++){
		p2[i*channels]=p1[i*channels];
		p2[i*channels+1]=p1[i*channels+1];
		p2[i*channels+2]=p1[i*channels+2];
		p2[i*channels+3]=p1[i*channels+3];
		if(p2[i*channels+1]>(p2[i*channels+2]*k)){
			p2[i*channels+3]=0;
		}
	}
}
void spillSupression(unsigned char *p1){
	channels = 4;
	int min;
	for(int i = 0; i<xres*yres ; i++){
		min = minimum((int)(p1[i*channels]),(int)(p1[i*channels+1]),(int)(p1[i*channels+2]));
		p1[i*channels+1] = (char)(min);
	}
	std::cout << "spillSupression done!" << std::endl;
}
void specialKeyHandle(int key,int x,int y){
	switch (key){
		case GLUT_KEY_UP:
			{
				if(!pv) break;
				if(k<5){
					k += 0.1;
				}
				std::cout << "k = " << k << std::endl;
				petroVlahos(oRgbaPixels,rgbaPixels);
				write();
			}break;
			
		case GLUT_KEY_DOWN:{
				if(!pv) break;
				if(k>0.5){
					k -= 0.1;
				}
				std::cout << "k = " << k << std::endl;
				petroVlahos(oRgbaPixels,rgbaPixels);
				write();
			}break;
	}
	glutPostRedisplay();
}
int main(int argc, char** argv) {
	if (argc < 2 || argc >4) {
		std::cerr << "Usage: " << argv[0] << " [filename] " << std::endl;
		exit(1);
	}
	filename_read = argv[1];
	if(argc == 3){             //------------basic mood
		filename_read = argv[1];
		filename_write = argv[2];
	}
	else if (argc == 4){      //------------- -pv or -ss
		filename_read = argv[2];
		filename_write = argv[3];
	}
	read();
	oRgbaPixels = new unsigned char [xres*yres*4];
	rgbaPixels = new unsigned char[xres*yres*4];
	hsvPixels = new double[xres*yres*4];
	threeToFour(pixels,rgbaPixels);
	threeToFour(pixels,oRgbaPixels);
	imgRGBtoImgHSV(rgbaPixels,hsvPixels);
	if(argc == 3){
		greenscale(hsvPixels,rgbaPixels);
	}else if(argc == 4 && (strcmp(argv[1],"-pv")==0)){
		pv = true ;
		petroVlahos(oRgbaPixels,rgbaPixels);
	}else if(argc == 4 && (strcmp(argv[1],"-ss")==0)){
		spillSupression(rgbaPixels);
	}
	glutInit(&argc, argv);
//	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(xres, yres);
	glutCreateWindow("display");
	glutDisplayFunc(displayImage);    // display callback
	glutKeyboardFunc(handleKey);      // keyboard callback
	glutSpecialFunc(specialKeyHandle);//responde to up arrow and down arrow
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, xres, 0, yres);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER,0);
	glClearColor(1, 1, 1, 1);

	pixels_write = rgbaPixels;
	write();
	glutMainLoop();
	if( pixels != NULL){
		delete pixels;
	}
	if(hsvPixels != NULL){
		delete hsvPixels;
	}
	if(pixels_write != NULL){
		delete pixels_write;
	}
	return 0;
}
