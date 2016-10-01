// Final Project of 6040 in Clemson
// Time:2015-12-01
//
// Author: RUI CHANG
// Email: rchang@g.clemson.edu

#include <cstdlib>
#include <iostream>
#include <math.h>
#include <OpenImageIO/imageio.h>
#include <time.h>
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
char* filename_write = NULL;
char* imageTop = NULL;
char* imageBase = NULL;
float* topPixels = NULL;
float* basePixels = NULL;
float* displayPixels = NULL;
float* writePixels = NULL;

float* read(char* filename_read)// read image and store it in pixels
{
	float* pixels = NULL;
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
    channels = spec.nchannels;
    
    std::cout << "x resolution: " << xres << std::endl;
    std::cout << "y resolution: " << yres << std::endl;
    std::cout << "num channels: " << channels << std::endl;
    
    //declare memory, open and read it
    pixels = new float[xres*yres*channels];
    
    //TypeDesc::UINT8 maps the data into a desired type (unsigned char),
    //even if it wasn’t originally of that type
    if (!in->read_image (TypeDesc::FLOAT, pixels)) {
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

#define maximum(x, y, z) ((x) > (y)? ((x) > (z)? (x) : (z)) : ((y) > (z)? (y) : (z)))
#define minimum(x, y, z) ((x) < (y)? ((x) < (z)? (x) : (z)) : ((y) < (z)? (y) : (z)))
void RGBtoHSV(float r, float g, float b, float &h, float &s, float &v){//change RGB to HSV for one pixel
    float red, green, blue; 
	double max, min, delta;
    red=r; 
	green=g;
	blue=b; /*r,g,bto0−1scale*/
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

void HSVtoRGB ( float h, float s, float v, float &r, float &g, float &b )//change HSV to RGB for one pixel
{
    int i;
    float f, p, q, t, red, green, blue;
    
    if (s == 0) {
        // grey
        red = green = blue = v;
    }
    else
    {
        h /= 60.0;
        i = (int) floor(h);
        f = h - i;
        p = v * (1-s);
        q = v * (1-s*f);
        t = v * (1 - s * (1 - f));
        
        switch (i) {
            case 0:
                red = v;
                green = t;
                blue = p;
                break;
            case 1:
                red = q;
                green = v;
                blue = p;
                break;
            case 2:
                red = p;
                green = v;
                blue = t;
                break;
            case 3:
                red = p;
                green = q;
                blue = v;
                break;
            case 4:
                red = t;
                green = p;
                blue = v;
                break;
            default:
                red = v;
                green = p;
                blue = q;
                break;
        }
    }
    
    r = red;//(unsigned char) (red*255.0 + .5);
    g = green;//(unsigned char) (green*255.0 + .5);
    b = blue;//(unsigned char) (blue*255.0 + .5);
}


bool imgRGBtoImgHSV(float *pixelSrc, float *pixelDes){//change RGB to HSV for whole image
    for(int i = 0; i< xres * yres ; i++){
        RGBtoHSV(pixelSrc[i*channels],pixelSrc[i*channels+1],pixelSrc[i*channels+2],pixelDes[i*channels],pixelDes[i*channels+1],pixelDes[i*channels+2]);
		if(channels == 4){
			pixelDes[i*channels + 3] = pixelSrc[i*channels + 3];
		}
	}
    std::cout << "imgRGB --> imgHSV done" << std::endl;
    return true;
}


bool imgHSVtoImgRGB(float *pixelSrc, float *pixelDes){//change HSV to RGB for whole image
    for(int i = 0; i< xres * yres ; i++){
        HSVtoRGB(pixelSrc[i*channels],pixelSrc[i*channels+1],pixelSrc[i*channels+2],pixelDes[i*channels],pixelDes[i*channels+1],pixelDes[i*channels+2]);
		if(channels == 4){
			pixelDes[i*channels + 3] = pixelSrc[i*channels + 3];
		}
	}
    std::cout << "imgRGB --> imgHSV done" << std::endl;
    return true;
}

float* darken(float* pixelTop, float* pixelBase,int width,int height,int channels){//darken mode (-da)
	float* outPixels;
	outPixels = new float[width * height * channels];
	for(int i=0; i<height; i++){
		for(int j = 0; j< width; j++){
			for(int k = 0; k<channels; k++){
				float a = pixelBase[i*width*channels + j*channels + k];
				float b = pixelTop[i*width*channels + j*channels + k];
				outPixels[i*width*channels + j*channels +k] = a > b ? b:a; 
			}
		}
	}
	return outPixels;
}

float* lighten(float* pixelTop, float* pixelBase,int width,int height,int channels){//lighten mode (-li)
	float* outPixels;
	outPixels = new float[width * height * channels];
	for(int i=0; i<height; i++){
		for(int j = 0; j< width; j++){
			for(int k = 0; k<channels; k++){
				float a = pixelBase[i*width*channels + j*channels + k];
				float b = pixelTop[i*width*channels + j*channels + k];
				outPixels[i*width*channels + j*channels +k] = a > b ? a:b; 
			}
		}
	}
	return outPixels;
}



float* multiply(float* pixelTop, float* pixelBase,int width,int height,int channels){//multiply mode (-mu)
	float* outPixels;
	outPixels = new float[width * height * channels];
	for(int i=0; i<height; i++){
		for(int j = 0; j< width; j++){
			for(int k = 0; k<channels; k++){
				float a = pixelBase[i*width*channels + j*channels + k];
				float b = pixelTop[i*width*channels + j*channels + k];
				outPixels[i*width*channels + j*channels +k] = a * b; 
			}
		}
	}
	return outPixels;
}

float* screen(float* pixelTop, float* pixelBase,int width,int height,int channels){//screen mode (-sc)
	float* outPixels;
	outPixels = new float[width * height * channels];
	for(int i=0; i<height; i++){
		for(int j = 0; j< width; j++){
			for(int k = 0; k<channels; k++){
				float a = pixelBase[i*width*channels + j*channels + k];
				float b = pixelTop[i*width*channels + j*channels + k];
				outPixels[i*width*channels + j*channels +k] = 1 - (1-a)*(1-b); 
			}
		}
	}
	return outPixels;
}



float* dissolve(float* pixelTop, float* pixelBase,int width,int height,int channels){//dissolve mode (-di)
	float* outPixels;
	outPixels = new float[width * height * channels];
	for(int i=0; i<height; i++){
		for(int j = 0; j< width; j++){
			for(int k = 0; k<channels; k++){
				int factor = rand()%2;
				outPixels[i*width*channels + j*channels +k] = factor * pixelTop[i*width*channels+j*channels +k] + (1-factor) * pixelBase[i*width*channels + j*channels + k];	
			}
		}
	}
	return outPixels;
}



float* overlay(float* pixelTop, float* pixelBase,int width,int height,int channels){//overlay mode (-ol)
	float* outPixels;

	outPixels = new float[width * height * channels];
	for(int i=0; i<height; i++){
		for(int j = 0; j< width; j++){
			for(int k = 0; k<channels; k++){
				int factor = rand()%2;
				float a = pixelBase[i*width*channels + j*channels + k];
				float b = pixelTop[i*width*channels + j*channels +k];
				if(a < 0.5){
					outPixels[i*width*channels + j*channels + k] = 2 * a * b;	
				}else{
					outPixels[i*width*channels + j*channels + k] = 1 - 2*(1-a)*(1-b);
				}
			}
		}
	}
	return outPixels;
}

float* hardlight(float* pixelTop, float* pixelBase,int width,int height,int channels){//hardlight mode (-hl)
	float* outPixels;

	outPixels = new float[width * height * channels];
	for(int i=0; i<height; i++){
		for(int j = 0; j< width; j++){
			for(int k = 0; k<channels; k++){
				float a = pixelBase[i*width*channels + j*channels + k];
				float b = pixelTop[i*width*channels + j*channels +k];
				if(b < 0.5){
					outPixels[i*width*channels + j*channels + k] = 2 * a * b;	
				}else{
					outPixels[i*width*channels + j*channels + k] = 1 - 2*(1-a)*(1-b);
				}

			}
		}
	}
	return outPixels;
}



float* colorDodge(float* pixelTop, float* pixelBase,int width,int height,int channels){//colorDodge mode (-cd)
	float* outPixels;
	outPixels = new float[width * height * channels];
	for(int i=0; i<height; i++){
		for(int j = 0; j< width; j++){
			for(int k = 0; k<channels; k++){
				int factor = rand()%2;
				float a = pixelBase[i*width*channels + j*channels + k];
				float b = pixelTop[i*width*channels + j*channels +k];
				float s = (1-b);
				outPixels[i*width*channels + j*channels + k] = a/(1-b)>1?1:a/(1-b);//( ( a/s )> 1? 1:(a/s));	
			}
		}
	}
	return outPixels;
}

float* linearDodge(float* pixelTop, float* pixelBase,int width,int height,int channels){//linearDodge mode (-ld)
	float* outPixels;
	outPixels = new float[width * height * channels];
	for(int i=0; i<height; i++){
		for(int j = 0; j< width; j++){
			for(int k = 0; k<channels; k++){
				float a = pixelBase[i*width*channels + j*channels + k];
				float b = pixelTop[i*width*channels + j*channels +k];
				outPixels[i*width*channels + j*channels + k] = a + b;	
			}
		}
	}
	return outPixels;
}

float* colorBurn(float* pixelTop, float* pixelBase,int width,int height,int channels){//colorBurn mode (-cb)
	float* outPixels;
	outPixels = new float[width * height * channels];
	for(int i=0; i<height; i++){
		for(int j = 0; j< width; j++){
			for(int k = 0; k<channels; k++){
				int factor = rand()%2;
				float a = pixelBase[i*width*channels + j*channels + k];
				float b = pixelTop[i*width*channels + j*channels +k];
				outPixels[i*width*channels + j*channels + k] = 1 - (1 - a)/b;
			}
		}
	}
	return outPixels;
}

float* linearBurn(float* pixelTop, float* pixelBase,int width,int height,int channels){//linearBurn mode (-lb)
	float* outPixels;
	outPixels = new float[width * height * channels];
	for(int i=0; i<height; i++){
		for(int j = 0; j< width; j++){
			for(int k = 0; k<channels; k++){
				float a = pixelBase[i*width*channels + j*channels + k];
				float b = pixelTop[i*width*channels + j*channels +k];
				outPixels[i*width*channels + j*channels + k] = 1 - ((1-a)+(1-b)); //> 0 ? :(1-((1-a)+(1-b))) , 0;
			}
		}
	}
	return outPixels;
}


float* hue(float* pixelTop, float* pixelBase,int width,int height,int channels){//hue mode (-hu)
	float* outPixels;
	float* outPixelsHSV;
	float* pixelTopHSV;
	float* pixelBaseHSV;
	outPixels = new float[width * height * channels];
	outPixelsHSV = new float[width * height * channels];
	pixelTopHSV = new float[width * height * channels];
	pixelBaseHSV = new float[width * height * channels];
	imgRGBtoImgHSV(pixelTop, pixelTopHSV);
    imgRGBtoImgHSV(pixelBase, pixelBaseHSV);
	for(int i=0; i<height; i++){
		for(int j = 0; j< width; j++){
			float hue = pixelTopHSV[i*width*channels + j*channels];
			float saturation = pixelBaseHSV[i*width*channels + j*channels + 1];
			float value = pixelBaseHSV[i*width*channels + j*channels + 2];
			//cout<<"hue = "<< hue << "saturation = "<< saturation << "value = "<<value <<endl;
			outPixelsHSV[i*width*channels + j*channels] = hue;
			outPixelsHSV[i*width*channels + j*channels + 1] = saturation;
			outPixelsHSV[i*width*channels + j*channels + 2] = value;
		}
	}
    imgHSVtoImgRGB(outPixelsHSV, outPixels);
	delete outPixelsHSV;
	delete pixelTopHSV;
	delete pixelBaseHSV;
	return outPixels;
}

float* saturation(float* pixelTop, float* pixelBase,int width,int height,int channels){//saturation mode (-sa)
	float* outPixels;
	float* outPixelsHSV;
	float* pixelTopHSV;
	float* pixelBaseHSV;
	outPixels = new float[width * height * channels];
	outPixelsHSV = new float[width * height * channels];
	pixelTopHSV = new float[width * height * channels];
	pixelBaseHSV = new float[width * height * channels];
	imgRGBtoImgHSV(pixelTop, pixelTopHSV);
    imgRGBtoImgHSV(pixelBase, pixelBaseHSV);
	for(int i=0; i<height; i++){
		for(int j = 0; j< width; j++){
			float hue = pixelBaseHSV[i*width*channels + j*channels];
			float saturation = pixelTopHSV[i*width*channels + j*channels + 1];
			float value = pixelBaseHSV[i*width*channels + j*channels + 2];
			outPixelsHSV[i*width*channels + j*channels] = hue;
			outPixelsHSV[i*width*channels + j*channels + 1] = saturation;
			outPixelsHSV[i*width*channels + j*channels + 2] = value;
		}
	}
    
	imgHSVtoImgRGB(outPixelsHSV, outPixels);
	delete outPixelsHSV;
	delete pixelTopHSV;
	delete pixelBaseHSV;
	return outPixels;
}

float* color(float* pixelTop, float* pixelBase,int width,int height,int channels){//color mode (-co)
	float* outPixels;
	float* outPixelsHSV;
	float* pixelTopHSV;
	float* pixelBaseHSV;
	outPixels = new float[width * height * channels];
	outPixelsHSV = new float[width * height * channels];
	pixelTopHSV = new float[width * height * channels];
	pixelBaseHSV = new float[width * height * channels];
	imgRGBtoImgHSV(pixelTop, pixelTopHSV);
    imgRGBtoImgHSV(pixelBase, pixelBaseHSV);
	for(int i=0; i<height; i++){
		for(int j = 0; j< width; j++){
			float hue = pixelTopHSV[i*width*channels + j*channels];
			float saturation = pixelTopHSV[i*width*channels + j*channels + 1];
			float value = pixelBaseHSV[i*width*channels + j*channels + 2];
			outPixelsHSV[i*width*channels + j*channels] = hue;
			outPixelsHSV[i*width*channels + j*channels + 1] = saturation;
			outPixelsHSV[i*width*channels + j*channels + 2] = value;
		}
	}
    
	imgHSVtoImgRGB(outPixelsHSV, outPixels);
	delete outPixelsHSV;
	delete pixelTopHSV;
	delete pixelBaseHSV;
	return outPixels;
}

float* luminosity(float* pixelTop, float* pixelBase,int width,int height,int channels){//luminosity mode (-lu)
	float* outPixels;
	float* outPixelsHSV;
	float* pixelTopHSV;
	float* pixelBaseHSV;
	outPixels = new float[width * height * channels];
	outPixelsHSV = new float[width * height * channels];
	pixelTopHSV = new float[width * height * channels];
	pixelBaseHSV = new float[width * height * channels];
	imgRGBtoImgHSV(pixelTop, pixelTopHSV);
    imgRGBtoImgHSV(pixelBase, pixelBaseHSV);
	for(int i=0; i<height; i++){
		for(int j = 0; j< width; j++){
			float hue = pixelBaseHSV[i*width*channels + j*channels];
			float saturation = pixelBaseHSV[i*width*channels + j*channels + 1];
			float value = pixelTopHSV[i*width*channels + j*channels + 2];
			outPixelsHSV[i*width*channels + j*channels] = hue;
			outPixelsHSV[i*width*channels + j*channels + 1] = saturation;
			outPixelsHSV[i*width*channels + j*channels + 2] = value;
		}
	}
    
	imgHSVtoImgRGB(outPixelsHSV, outPixels);
	delete outPixelsHSV;
	delete pixelTopHSV;
	delete pixelBaseHSV;
	return outPixels;
}




bool write()               // write image
{
    ImageOutput *out = ImageOutput::create(filename_write);
    if (!out) {
        std::cerr << "Could not create: " << geterror();
        exit(-1);
    }
   	ImageSpec spec_w (xres, yres, channels, TypeDesc::FLOAT);
    out->open(filename_write, spec_w);
    //it is possible that this TypeDesc does not match the ImageSpec,
    //in which case it will convert the raw data into the spec.
    //But it MUST match the datatype of raw data
    out->write_image(TypeDesc::FLOAT, writePixels);
    std::cout << "write done" << std::endl;
    if (!out->close ()) {
        std::cerr << "Error closing " << filename_write;
        std::cerr << ", error = " << out->geterror() << std::endl;
        delete out;
        exit(-1);
    }
    return true;
}
void displayImage(){ //for function call glutDisplayFunc(displayImage);    
    cout<< "start display"<<endl;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPixelZoom(1,-1);// set display direction
    glRasterPos2i(0,yres-1);// set display start point
    if (channels == 3){
        glDrawPixels(xres,yres,GL_RGB,GL_FLOAT,displayPixels);
    }
    if (channels == 4){
        glDrawPixels(xres,yres,GL_RGBA,GL_FLOAT,displayPixels);
    }
    if (channels == 1){
        glDrawPixels(xres,yres,GL_LUMINANCE,GL_FLOAT,displayPixels);
    }
    glFlush();
}

void copy(float* pixelSrc, float* pixelDes, int width, int height, int channels){//copy image from pixelSrc to pixelDes
	for(int i=0; i<height; i++){
		for(int j = 0; j< width; j++){
			for(int k = 0; k<channels; k++){
				pixelDes[i*width*channels + j*channels +k] = pixelSrc[i*width*channels+j*channels +k]; 
			}
		}
	}
}
void handleKey(unsigned char key,int x,int y){// handle write and quit
    switch(key){
        case 'w':
        case 'W': {
            writePixels = displayPixels ;
            write();
            break;
        }
        case 'q':                      // q - quit
        case 'Q':// freeMemo();
        case 27: exit(0);                  // esc - quit
        default: return;                           // not a valid key -- just ignore i
    }
    glutPostRedisplay();
}



int main(int argc, char** argv) {
    if(argc < 5 || argc > 6){
        std::cerr << "Usage:" << argv[0] <<" read_filename" << " [write_filename]" <<endl;
    }
    imageTop = argv[2];
	imageBase = argv[3];
    filename_write = argv[4];
    
	float* tempPixels;

	tempPixels = read(imageTop);
	topPixels = new float[xres*yres*channels];
	cout<<"read top"<<endl;
    copy(tempPixels, topPixels, xres, yres, channels); 

	tempPixels = read(imageBase);
	basePixels = new float[xres*yres*channels];
	copy(tempPixels, basePixels, xres, yres, channels);
	
	srand(time((NULL)));
	cout<<"read down"<<endl;
	if(strcmp(argv[1],"-da") == 0 ){
	    displayPixels =	darken(topPixels,basePixels,xres,yres,channels);
	}else if(strcmp(argv[1],"-li") == 0 ){
	    displayPixels = lighten(topPixels,basePixels,xres,yres,channels);
	}else if(strcmp(argv[1],"-mu") == 0 ){
	    displayPixels =	multiply(topPixels,basePixels,xres,yres,channels);
	}else if(strcmp(argv[1],"-sc") == 0 ){
	    displayPixels =	screen(topPixels,basePixels,xres,yres,channels);
	}else if(strcmp(argv[1],"-di") == 0 ){
	    displayPixels =	dissolve(topPixels,basePixels,xres,yres,channels);
	}else if(strcmp(argv[1],"-hl") == 0 ){
	    displayPixels =	hardlight(topPixels,basePixels,xres,yres,channels);
	}else if(strcmp(argv[1],"-ov") == 0 ){
	    displayPixels =	overlay(topPixels,basePixels,xres,yres,channels);
	}else if(strcmp(argv[1],"-cd") == 0 ){
	    displayPixels =	colorDodge(topPixels,basePixels,xres,yres,channels);
	}else if(strcmp(argv[1],"-ld") == 0 ){
	    displayPixels =	linearDodge(topPixels,basePixels,xres,yres,channels);
	}else if(strcmp(argv[1],"-cb") == 0 ){
	    displayPixels =	colorBurn(topPixels,basePixels,xres,yres,channels);
	}else if(strcmp(argv[1],"-lb") == 0 ){
	    displayPixels =	linearBurn(topPixels,basePixels,xres,yres,channels);
	}else if(strcmp(argv[1],"-hu") == 0 ){
	    displayPixels =	hue(topPixels,basePixels,xres,yres,channels);
	}else if(strcmp(argv[1],"-sa") == 0 ){
	    displayPixels =	saturation(topPixels,basePixels,xres,yres,channels);
	}else if(strcmp(argv[1],"-co") == 0 ){
	    displayPixels =	color(topPixels,basePixels,xres,yres,channels);
	}else if(strcmp(argv[1],"-lu") == 0 ){
	    displayPixels =	luminosity(topPixels,basePixels,xres,yres,channels);
	}else{
		cout << "invalid mode"<< endl;
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
	return 0;
}
