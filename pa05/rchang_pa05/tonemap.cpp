// Project05 of 6040 in Clemson
// Time:2015-10-27
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
int N = 0;//the filter's width = height = N
int C = 20;
int channels = 0;
int ochannels = 0;
float gammaValue = 0.5;
char* filename_read = NULL;
char* kernelfile_name = NULL;
char* filename_write = NULL;
float* pixels = NULL;
float* displayPixels = NULL;
float* oPixels = NULL;
float* writePixels = NULL;
float ** filter = NULL;
bool SMOOD = false;
bool CMOOD = false;
bool BMOOD = false;
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
	pixels = new float[xres*yres*channels]; 
	oPixels = new float[xres*yres*channels];
	
	//TypeDesc::UINT8 maps the data into a desired type (unsigned char),
	//even if it wasn’t originally of that type
	if (!in->read_image (TypeDesc::FLOAT, pixels)) {
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
float** readKernel(char* filename){
		ifstream kernelfile; 
		kernelfile.open(filename);  
		if (! kernelfile.is_open())  {
			cout << "Error opening file"; exit (1);
		}  
		kernelfile >> N;  
		float** filt;
		filt = new float*[N];
		for(int i = 0 ; i < N ; i++){
			filt[i] = new float[N];
		}
		for(int i = 0 ; i < N ; i++){
			for(int j = 0 ; j < N ;j++){
				kernelfile >> filt[i][j];
				cout << filt[i][j]<<",";
			}
			cout <<"," <<endl;
		}
		kernelfile.close();
		return filt;
}
void resetPixels(float* srcPixels,float* desPixels,int channel){
    for(int i = 0 ;i < yres ; i++){
        for(int j = 0 ; j< xres ; j++){
            for(int k = 0 ; k < channel ; k ++){
                desPixels[i*xres*channel + j*channel +k] = srcPixels[i*xres*channel+ j*channel +k];
            }
        }
    }
}
float* filtImage(float * inputPixels,int width,int height,int channel, float**kernel){ //use kernel to convolve image
    float factor = 0;
    float posFactor = 0;
    float negFactor = 0;
    float sum = 0;
    float* tempPixels ;
    float* outputPixels ;
    tempPixels = new float[width*height*channel];
    outputPixels = new float[width*height*channel];
    resetPixels(inputPixels,tempPixels,channel);
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
    cout<< "channel = "<<channel << endl;
    for(int i = 0 ; i < height ; i++){
        for(int j = 0 ; j < width ; j++){
            for(int k = 0 ; k < channel; k++){
                for (int y = 0 ; y < N ; y++){
                    for(int x = 0 ; x < N ; x++){
                        int edgeH = i + y - N/2;
                        int edgeW = j + x - N/2;
                        if(edgeH>=0 && edgeH < yres && edgeW >=0 && edgeW < xres){
                            sum += kernel[y][x]*tempPixels[(i+y-N/2)*width*channel +(j+x-N/2)*channel + k];
                        }
                    }	
                }
			    float result = (sum / factor);
                outputPixels[i*width*channel+ j*channel + k] = result;
                sum = 0;
                result = 0;
            }
        }
    }
	delete tempPixels;
	return outputPixels;
}
float* bilateralFiltImage(float * inputPixels,int width,int height,int channel, float**kernel){ //use kernel to convolve image
    float factor = 0;
    float sum = 0;
    float weight = 0;
	float* tempPixels ;
    float* outputPixels ;
	
    tempPixels = new float[width*height*channel];
    outputPixels = new float[width*height*channel];
    resetPixels(inputPixels,tempPixels,channel);
    cout<< "channel = "<<channel << endl;
    for(int i = 0 ; i < height ; i++){
        for(int j = 0 ; j < width ; j++){
            for(int k = 0 ; k < channel; k++){
                for (int y = 0 ; y < N ; y++){
                    for(int x = 0 ; x < N ; x++){
                        int edgeH = i + y - N/2;
                        int edgeW = j + x - N/2;
                        if(edgeH>=0 && edgeH < yres && edgeW >=0 && edgeW < xres){
						    weight = exp(-pow(tempPixels[edgeH*width*channel + edgeW*channel + k] - tempPixels[i*width*channel+j*channel + k],2));	
							sum += weight*kernel[y][x]*tempPixels[(edgeH)*width*channel +(edgeW)*channel + k];
							factor += kernel[y][x]*weight;
						}
                    }
                }
                float result = (sum / factor);
                outputPixels[i*width*channel+ j*channel + k] = result;
                sum = 0;
                result = 0;
				factor = 0;
			}
        }
    }
    delete tempPixels;
    return outputPixels;
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
void displayImage(){
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
float* simpleToneMapping(float* inputPixels,float gamma, int width ,int height , int channels){
	float* Ld;
	float* Lw;
	float* outputPixels;
	Lw = new float[width*height];
	Ld = new float[width*height];
	outputPixels = new float[width*height*channels];
	for(int i = 0 ; i < height ; i++){
		for(int j = 0; j < width ; j++){
			Lw[i*width + j]=(1/61.0)*(20*inputPixels[i*width*channels+j*channels+0]+40*inputPixels[i*width*channels+j*channels+1]+inputPixels[i*width*channels+j*channels+2]);
		}
	}
	for(int i = 0 ; i < height ; i++){
		for(int j = 0; j < width ; j++){
			Ld[i*width + j]=exp(gamma*log(Lw[i*width + j]));
		}
	}
	for(int i = 0 ; i < height ; i++){
		for(int j = 0; j < width ; j++){
			for(int k = 0 ; k < channels ; k++){
				outputPixels[i*width*channels+j*channels+k] = ( Ld[i*width+j]/Lw[i*width+j])*inputPixels[i*width*channels+j*channels+k];
			}
		}
	}
	delete Ld;
	delete Lw;
	return outputPixels;
}
float* toneMappingWithConvolution(float* inputPixels, int width, int height,int channels, float** kernel){
	float* Lw;
	float* logLw;
	float* Ld;
	float* B;
	float* S;
	float* outputPixels;
	float maxB;
	float minB;
	float gamma;
	Lw = new float[width*height];
	logLw = new float[width*height];
	Ld = new float[width*height];
	B = new float[width*height];
	S = new float[width*height];
	outputPixels = new float[width*height*channels];
	for(int i = 0 ; i < height ; i++){
		for(int j = 0; j < width ; j++){
			Lw[i*width + j]=(1/61.0)*(20*inputPixels[i*width*channels+j*channels+0]+40*inputPixels[i*width*channels+j*channels+1]+inputPixels[i*width*channels+j*channels+2]);
			logLw[i*width + j]= log(Lw[i*width + j]);
		}
	}//count out Lw and logLw;
	if(BMOOD == false){
		B = filtImage(logLw,width,height,1,kernel);//count out B;
		cout << "CMOOD ON"<<endl;
	}
	if(BMOOD == true){
		B = bilateralFiltImage(logLw,width,height,1,kernel);//count out B;
		cout << "BMOOD ON"<<endl;
	}
	minB = maxB = B[0];
	for(int i = 0 ; i < height ; i++){
		for(int j = 0; j < width ; j++){
			if(B[i*width + j]>maxB){
				maxB = B[i*width +j];
			}
			if(B[i*width + j]<minB){
				minB = B[i*width + j];
			}
			S[i*width + j]=logLw[i*width + j] - B[i*width + j];
		}
	}//count out S and maxB and minB
    cout<<"minB ="<< minB <<endl;
    cout<<"maxB ="<< maxB <<endl;

	gamma = log(C)/(maxB-minB);//count out gammaValue;
	for(int i = 0 ; i < height ; i++){
		for(int j = 0; j < width ; j++){
			Ld[i*width + j] = exp(gamma*B[i*width + j] + S[i*width + j]);
		}
	}
	for(int i = 0 ; i < height ; i++){
		for(int j = 0; j < width ; j++){
			for(int k = 0 ; k < channels ; k++){
				outputPixels[i*width*channels+j*channels+k] = (Ld[i*width+j]/Lw[i*width+j])*inputPixels[i*width*channels+j*channels+k];
			}
		}
	}
	delete Lw;
	delete logLw;
	delete Ld;
	delete B;
	delete S;
	return outputPixels;
}

void handleKey(unsigned char key,int x,int y){
	switch(key){
		case 'b':
		case 'B':{
			BMOOD = true;
			CMOOD = false;
			SMOOD = false;
			displayPixels = toneMappingWithConvolution(pixels,xres,yres,channels,filter);
			break;
		}
		case 'c':
		case 'C':{
			CMOOD = true;
			SMOOD = false;
			BMOOD = false;
			displayPixels = toneMappingWithConvolution(pixels,xres,yres,channels,filter);
			break;
		}
		case 'r':
	    case 'R':{
			SMOOD = false;
			CMOOD = false;
			BMOOD = false;
			displayPixels = oPixels;
			break;
		}
		case 's':
		case 'S':{
			SMOOD = true;
			CMOOD = false;
			BMOOD = false;
			displayPixels = simpleToneMapping(pixels,gammaValue,xres,yres,channels); 
			break;
		}
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
void specialKeyHandle(int key , int x , int y){
	switch (key){
		case GLUT_KEY_UP:
			{   if(SMOOD){
					if (gammaValue < 0)	{gammaValue = 0;}
					if (gammaValue > 1) {gammaValue = 1;}
					if (gammaValue>=0 && gammaValue<=0.95){
						gammaValue += 0.05;
						std::cout << "gammaValue = "<< gammaValue << std::endl;
						displayPixels = simpleToneMapping(pixels,gammaValue,xres,yres,channels); 
					}
					break;
				}
				if(CMOOD == true|| BMOOD == true){
					if(C < 5) {	C = 0;}	
					if(C > 100)	{C = 100;}
					if(C>=5 && C<= 95){
						C += 5;
						std::cout << "C="<< C << std::endl;
						displayPixels = toneMappingWithConvolution(pixels,xres,yres,channels,filter);
					}
					break;
				}
			}
		case GLUT_KEY_DOWN:
			{
				if(SMOOD){
					if (gammaValue < 0) gammaValue = 0;
					if(gammaValue>1) gammaValue = 1;
					if(gammaValue>=0.05 && gammaValue<=1){
						gammaValue -= 0.05;
						std::cout << "gammaValue = "<< gammaValue << std::endl;
						displayPixels = simpleToneMapping(pixels,gammaValue,xres,yres,channels); 
					}	
				}
				if(CMOOD == true || BMOOD == true){
					if(C < 5) C = 0;
					if(C > 100)	C = 100;
					if(C>=10 && C<= 100){
						C -= 5;
						std::cout << "C="<< C << std::endl;
						displayPixels = toneMappingWithConvolution(pixels,xres,yres,channels,filter);
					}
					break;
				}
			}
		}
		glutPostRedisplay();
}
int main(int argc, char** argv) {
	if(argc < 3 || argc > 5){
		std::cerr << "Usage:" << argv[0] <<"[read_filename]" << "[write_filename]" <<"[kernel_filename]"<< endl;
	}
    filename_read = argv[1];
	filename_write = argv[2];
    read();
	if(argc == 4){
		filter = readKernel(argv[3]);
	}
	displayPixels = pixels;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(xres, yres);
	glutCreateWindow("display");
	glutDisplayFunc(displayImage);    // display callback
	glutKeyboardFunc(handleKey);      // keyboard callback
	glutSpecialFunc(specialKeyHandle);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, xres, 0, yres);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER,0);
	glClearColor(1, 1, 1, 1);
	glutMainLoop();
	return 0;
}
