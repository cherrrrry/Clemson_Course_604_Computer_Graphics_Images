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
unsigned char* pixels = NULL;
char* filename_write = NULL;
unsigned char * forePixels = NULL;
unsigned char * backPixels = NULL;
unsigned char * outPixels = NULL;
unsigned char *pixels_write = NULL;
//double* pixels_write = NULL;
unsigned char* read(char *filename_read)// read image and store it in pixels
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
	//	pixels_write = pixels;
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
   	return pixels;
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
void composition(unsigned char* pf,unsigned char *pb,unsigned char *po){
	for(int i = 0; i<xres*yres ; i++){
		double af = pf[i*channels+3]/255;
		po[i*channels]= (unsigned char)(pf[i*channels]*af+(1-af)*pb[i*channels]);
		po[i*channels+1]= (unsigned char)(pf[i*channels+1]*af+(1-af)*pb[i*channels+1]);
		po[i*channels+2]= (unsigned char)(pf[i*channels+2]*af+(1-af)*pb[i*channels+2]);
		po[i*channels+3]= 255;
	}
}
int main(int argc, char** argv) {
	if (argc < 3 || argc >4) {
		std::cerr << "Usage: " << argv[0] << " [filename] " << std::endl;
		exit(1);
	}
	forePixels = read(argv[1]);
	backPixels = read(argv[2]);
	filename_write = argv[3];
	threeToFour(backPixels,backPixels);
	outPixels = new unsigned char [xres*yres*4];
	composition(forePixels,backPixels,outPixels);
	pixels_write = outPixels;
	write();
	return 0;
}
