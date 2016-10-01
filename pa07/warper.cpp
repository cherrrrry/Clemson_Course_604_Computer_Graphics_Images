// Project06 of 6040 in Clemson
// Time:2015-10-27
//
// Author: RUI CHANG
// Email: rchang@g.clemson.edu
#include "Matrix.h"
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
int width = 0;
int height = 0;
int channels = 0;
bool twirlFlag = false;
bool fixWarp = false;
char* filename_read = NULL;
char* filename_write = NULL;
char* kernelfile_name = NULL;
float* pixels = NULL;
float* displayPixels = NULL;
float* oPixels = NULL;
float* writePixels = NULL;
float* oWritePixels = NULL;
float cx,cy,s;
Matrix3x3 M(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
float bilinearWarp(float * inputPixels,float u,float v,int k);
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
    channels = spec.nchannels;
    
    std::cout << "x resolution: " << xres << std::endl;
    std::cout << "y resolution: " << yres << std::endl;
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
    for(int i = 0 ;i < yres ; i++){
        for(int j = 0 ; j< xres ; j++){
            for(int k = 0 ; k < channels ; k ++){
                pixels[i*xres*channels + j*channels +k] = oPixels[(yres-i-1)*xres*channels+ j*channels +k];
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
bool write()               // write image
{
    float* tempPixels;
	oWritePixels = new float[width*height*channels];
	tempPixels = new float[width*height*channels];
	for(int i = 0 ;i < height ; i++){
        for(int j = 0 ; j< width ; j++){
            for(int k = 0 ; k < channels ; k ++){
                oWritePixels[i*width*channels + j*channels +k] = writePixels[i*width*channels+ j*channels +k];
            }
        }
    }
    for(int i = 0 ;i < height ; i++){
        for(int j = 0 ; j< width ; j++){
            for(int k = 0 ; k < channels ; k ++){
                tempPixels[i*width*channels + j*channels +k] = oWritePixels[(height-i-1)*width*channels+ j*channels +k];
            }
        }
    }

    writePixels = tempPixels;
	ImageOutput *out = ImageOutput::create(filename_write);
    if (!out) {
        std::cerr << "Could not create: " << geterror();
        exit(-1);
    }
   	ImageSpec spec_w (width, height, channels, TypeDesc::FLOAT);
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
    //glPixelZoom(1,-1);// set display direction
    //glRasterPos2i(0,height-1);// set display start point
    if (channels == 3){
        glDrawPixels(width,height,GL_RGB,GL_FLOAT,displayPixels);
    }
    if (channels == 4){
        glDrawPixels(width,height,GL_RGBA,GL_FLOAT,displayPixels);
    }
    if (channels == 1){
        glDrawPixels(width,height,GL_LUMINANCE,GL_FLOAT,displayPixels);
    }
    glFlush();
}
void lowercase(char *s){//Convert the string s to lower case
	int i;
	if(s != NULL){
		for(i=0; s[i]!='\0'; i++){
			if(s[i] >= 'A' && s[i] <= 'Z')
				s[i] += ('a'-'A');
		}
	}
}
void Rotate(Matrix3x3 &M, float theta){//Multiply M by a rotation matrix of angle theta
	int row, col;
	Matrix3x3 R(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
	double rad,c,s;
	rad = PI * theta / 180.0;
	c = cos(rad);
	s = sin(rad);
	R[0][0] = c;
	R[0][1] = -s;
	R[1][0] = s;
	R[1][1] = c;
	Matrix3x3 Prod = R * M;
	for (row = 0; row < 3; row ++){
		for(col = 0; col < 3; col ++){
			M[row][col] = Prod[row][col];		
		}
	}
}
void Scale(Matrix3x3 &M, float sx, float sy){
	int row, col;
	Matrix3x3 R(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
	R[0][0] = sx;
	R[1][1] = sy;
	Matrix3x3 Prod = R * M;
	for (row = 0; row < 3; row ++){
		for(col = 0; col < 3; col ++){
			M[row][col] = Prod[row][col];		
		}
	}
}
void Translate(Matrix3x3 &M, int dx, int dy){
	int row, col;
	Matrix3x3 R(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
	R[0][2] = dx;
	R[1][2] = dy;
	Matrix3x3 Prod = R * M;
	for (row = 0; row < 3; row ++){
		for(col = 0; col < 3; col ++){
			M[row][col] = Prod[row][col];		
		}
	}
}

void Shear(Matrix3x3 &M, float hx, float hy){
	int row, col;
	Matrix3x3 R(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
	R[0][1] = hx;
	R[1][0] = hy;
	Matrix3x3 Prod = R * M;
	for (row = 0; row < 3; row ++){
		for(col = 0; col < 3; col ++){
			M[row][col] = Prod[row][col];		
		}
	}
}

void Perspective(Matrix3x3 &M, float px, float py){
	int row, col;
	Matrix3x3 R(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
	R[2][0] = px;
	R[2][1] = py;
	Matrix3x3 Prod = R * M;
	for (row = 0; row < 3; row ++){
		for(col = 0; col < 3; col ++){
			M[row][col] = Prod[row][col];		
		}
	}
}

float* Twirl(float s, float cx, float cy, float * inputPixels){
	int minDim;
	float u,v;
	float r,theta;
	float* outputPixels;
	width = xres;
	height = yres;
	cx = width*cx;
	cy = height*cy;
	cout<< "width:"<<width <<",height:"<<height<<endl;
	outputPixels = new float[width * height * channels];
	minDim = Min(width, height);
	for(int y = 0; y < height; y++){
		for(int x = 0; x < width; x++){
			for(int k = 0 ; k < channels ; k++){
					if(fixWarp == false){
						r = sqrt ((y-cy)*(y-cy) + (x-cx)*(x-cx));
						theta = atan2(y-cy,x-cx);
						u =  r*cos(( theta + s*(r - minDim)/minDim ) ) + cx;
						v =  r*sin(( theta + s*(r - minDim)/minDim ) ) + cy;
						if( u<0 || u > xres-1 || v<0 || v > yres-1){
							outputPixels [y*width*channels + x*channels + k] = 0;
						}else{
							outputPixels [y*width*channels + x*channels + k] = inputPixels[int (round(v)*xres*channels + round(u)*channels + k)];
						}		
					}else if(fixWarp == true){
						r = sqrt ((y-cy)*(y-cy) + (x-cx)*(x-cx));
						theta = atan2(y-cy,x-cx);
						u =  r*cos(( theta + s*(r - minDim)/minDim ) ) + cx;
						v =  r*sin(( theta + s*(r - minDim)/minDim ) ) + cy;
						if( u<0 || u > xres-1 || v<0 || v > yres-1){
							outputPixels [y*width*channels + x*channels + k] = 0;
						}else{
							int _x = x;
							int _y = y;
							float left = (_x - 0.5)>0 ? (_x-0.5):0;
							float right = (_x + 0.5)<width ? (_x+0.5):width-0.5;
							float bottom = (_y - 0.5)>0 ? (_y-0.5):0;
							float top = (_y + 0.5)<height ? (_y+0.5):height-0.5;
							float sum  = 0;
							int num = 0;
							for(float i = bottom ; i<=top ; i+=0.25){
								for(float j = left; j<=right; j+=0.25){
									r = sqrt ((i-cy)*(i-cy) + (j-cx)*(j-cx));
									theta = atan2(i-cy,j-cx);
								    float _u =  r*cos(( theta + s*(r - minDim)/minDim ) ) + cx;
									float _v =  r*sin(( theta + s*(r - minDim)/minDim ) ) + cy;
									if( _u<0 || _u > xres-1 || _v<0 || _v > yres-1){
										sum += 0;
									}else{
										sum += bilinearWarp(inputPixels,_u,_v,k);
										num ++;
									}
								}	
							}
							outputPixels [y*width*channels + x*channels + k] = sum/num ;
						}				
					}
			}
		}
	}
	cout<<"call twirl"<<endl;
	return outputPixels;
}


/*Routine to build a projective transform from input text, display, or write transformed image to file*/
void processInput(Matrix3x3 &M){
	char command[1024];
	bool done;
	float theta,sx,sy,hx,hy,px,py;	
	int dx,dy;
	M.identity();//build identity matrix
	
	for(done = false; !done;){
		/*prompt and accept input, converting text to lower case*/
		printf("enter r, s, t, h, n, d >");
		scanf("%s",command);
		lowercase(command);
		/* parse the input command, and read parameters as needed*/
		if(strcmp(command,"d") == 0){
			done = true;
		} else if(strlen(command) != 1){
			printf("invalid command, enter r, s, t, h, n, d\n");
		} else {
			switch(command[0]){
				case 'r':  /*Rotation, accept angle in degrees */
					printf("enter rotation angle in degree >");
					if(scanf("%f", &theta) == 1)
						Rotate(M, theta);
					else
						fprintf(stderr,"invalid rotation angle\n");
					break;
				case 's':  /* Scale, accept scale factors*/
					printf("enter scaling x size >");
					if(scanf("%f",&sx) == 1){
						printf("\nenter scaling y size >");
						if(scanf("%f",&sy) == 1){
							if(sx==0 || sy==0){
								fprintf(stderr,"sx and sy can not be 0");
							}else{	
								Scale(M,sx,sy);
							}
						}
					}else{
						fprintf(stderr,"invalid scale size\n");
					}
					break;
				case 't':  /* Translation, accept translation*/
				    printf("enter translation x direction >");
					if(scanf("%d",&dx) == 1){
						printf("enter translation y direction >");
						if(scanf("%d",&dy) == 1){
							Translate(M, dx, dy);
						}
					}else{
						fprintf(stderr,"invalid translation");
					}
					break;
				case 'h':/* Shear, accept shear factors */
					printf("enter shearing x >");
					if(scanf("%f",&hx) == 1){
						printf("enter shearing y >");
						if(scanf("%f",&hy) == 1){
							Shear(M, hx, hy);
						}
					}
					break;
				case 'p':/* Perspective, accept perspective factors */
					printf("enter perspective px >");
					if(scanf("%f",&px) == 1){
						printf("enter perspective py >");
						if(scanf("%f",&py) == 1){
							Perspective(M, px, py);
						}
					}
					break;

				case 'n':/* twirl , accept twirl factor */
					twirlFlag = true;
					printf("enter twirl strength s >");
					if(scanf("%f",&s) == 1){
						printf("enter twirl center cx >");
						if(scanf("%f",&cx) ==1 ){
							printf("enter twirl center cy >");
							if(scanf("%f",&cy) ==1 ){
								if(cx<=0 || cx>1 || cy<=0 || cy>1){
									fprintf(stderr,"cx and cy should be in (0,1]");
								}else{
									cout<<"there should twirl"<<endl;
								//	displayPixels = Twirl(s, cx, cy,pixels);			
								}
							}
						}
					}
					break;
				case 'd': /*  Done, that's all for now*/
					done = true;
					break;
				default:
					printf("invalid command , enter r, s, t, h, d\n");
			}
		}
	}
}
float bilinearWarp(float * inputPixels,float u,float v,int k){
	float value1 = inputPixels[(int) (ceil(v)*xres*channels + floor(u)*channels + k) ];//upperleft
	float value2 = inputPixels[(int) (floor(v)*xres*channels + floor(u)*channels + k)];//bottomleft
	float value3 = inputPixels[(int) (floor(v)*xres*channels + ceil(u)*channels + k)];//bottomright
	float value4 = inputPixels[(int) (ceil(v)*xres*channels + ceil(u)*channels + k)];//upperrightw
	float value12 = value1*(v-floor(v)) + value2*(ceil(v)-v);
	float value34 = value4*(v-floor(v)) + value3*(ceil(v)-v);
	float value = value12*(ceil(u)-u) + value34*(u-floor(u));
    return value;
}
float supersample(float * inputPixels, Matrix3x3 &_invM,Vector3d& _pixelOut, int k){
    int _x = _pixelOut[0];
	int _y = _pixelOut[1];
	float left = (_x - 0.5)>0 ? (_x-0.5):0;
    float right = (_x + 0.5)<width ? (_x+0.5):width-0.5;
    float bottom = (_y - 0.5)>0 ? (_y-0.5):0;
    float top = (_y + 0.5)<height ? (_y+0.5):height-0.5;
	float sum  = 0;
	int num = 0;
	for(float i = bottom ; i<=top ; i+=0.25){
	  for(float j = left; j<=right; j+=0.25){
		    Vector3d tempOut(j,i,1);
			Vector3d _pixelIn = _invM * tempOut;
		    _pixelIn[0]=_pixelIn[0]/_pixelIn[2];
			_pixelIn[1]=_pixelIn[1]/_pixelIn[2];
		    _pixelIn[2]=_pixelIn[2]/_pixelIn[2];
        	if( _pixelIn[0]<0 || _pixelIn[0] > xres-1 || _pixelIn[1]<0 || _pixelIn[1] > yres-1){
				sum += 0;
			}else{
				sum += bilinearWarp(inputPixels,_pixelIn[0],_pixelIn[1],k);  
				num ++;
			}
		}
	}
	return 	sum/num;
}
float*  warpImage(Matrix3x3 &M, float * inputPixels){
	float right, left, top, bottom;
	float* outputPixels;
	/*count out conners location*/
	Vector3d bottomLeft(0, 0 , 1);
	Vector3d bottomRight(xres-1 , 0 , 1);
	Vector3d topLeft(0, yres-1 , 1);
	Vector3d topRight(xres-1, yres-1 , 1);
	
	Vector3d bottomLeft1 = M * bottomLeft;
	Vector3d bottomRight1 = M * bottomRight;
	Vector3d topLeft1 = M * topLeft;
	Vector3d topRight1 = M * topRight;
    bottomLeft1[0] = bottomLeft1[0]/bottomLeft1[2];//normalization
	bottomLeft1[1] = bottomLeft1[1]/bottomLeft1[2];
	bottomLeft1[2] = bottomLeft1[2]/bottomLeft1[2];
    bottomRight1[0] = bottomRight1[0]/bottomRight1[2];
    bottomRight1[1] = bottomRight1[1]/bottomRight1[2];
    bottomRight1[2] = bottomRight1[2]/bottomRight1[2];
    topLeft1[0] = topLeft1[0]/topLeft1[2];
    topLeft1[1] = topLeft1[1]/topLeft1[2];
    topLeft1[2] = topLeft1[2]/topLeft1[2];
    topRight1[0] = topRight1[0]/topRight1[2];
    topRight1[1] = topRight1[1]/topRight1[2];
    topRight1[2] = topRight1[2]/topRight1[2];
    cout<<"bottomLeft1:"<<bottomLeft1<<endl;
    cout<<"bottomRight1:"<<bottomRight1<<endl;
    cout<<"topLeft1:"<<topLeft1<<endl;
    cout<<"topRight1:"<<topRight1<<endl;
	top = Max(Max(bottomLeft1[1],bottomRight1[1]),Max(topLeft1[1],topRight1[1]));
	bottom =Min(Min(bottomLeft1[1],bottomRight1[1]),Min(topLeft1[1],topRight1[1]));
	right = Max(Max(bottomLeft1[0],bottomRight1[0]),Max(topLeft1[0],topRight1[0]));
	left =Min(Min(bottomLeft1[0],bottomRight1[0]),Min(topLeft1[0],topRight1[0]));
    cout<<"top:"<< top << ", bottom:"<< bottom << ", left"<< left << ", right"<< right<< endl;	
	Vector3d origin(left, bottom, 0);
	width =  round(right - left );
    cout<< "width = "<< width <<endl;
	height = round(top - bottom);
	cout<< "height = "<< height <<endl;
	outputPixels = new float[height * width * channels];
	Matrix3x3 invM = M.inv();
	cout << "invM" << invM << endl;
	for(int y = 0 ; y < height ; y++){
		for(int x = 0 ; x < width ; x++){
			//map the pixel coordinates
			Vector3d pixelOut(x,y,1);
		   	pixelOut = pixelOut + origin;
			Vector3d pixelIn = invM * pixelOut;
			//normalize the pixelmap
		    pixelIn[0]=pixelIn[0]/pixelIn[2];
			pixelIn[1]=pixelIn[1]/pixelIn[2];
		    pixelIn[2]=pixelIn[2]/pixelIn[2];
			for(int k = 0 ; k < channels ; k++){
				if (fixWarp == false){/*normal warp*/
	                    if( pixelIn[0]<0 || pixelIn[0] > xres-1 || pixelIn[1]<0 || pixelIn[1] > yres-1){
							outputPixels [y*width*channels + x*channels + k] = 0;
						}else{
							int u = round(pixelIn[0]);
							int v = round(pixelIn[1]);
							outputPixels [y*width*channels + x*channels + k] = inputPixels[v*xres*channels + u*channels + k];
						}
					}else if(fixWarp == true){/*fix warp*/
						if( pixelIn[0]<0 || pixelIn[0] > xres-1 || pixelIn[1]<0 || pixelIn[1] > yres-1){
							outputPixels [y*width*channels + x*channels + k] = 0;
						}else{
						    //outputPixels [y*width*channels + x*channels + k] = bilinearWarp(inputPixels,pixelIn[0],pixelIn[1],k);
							outputPixels [y*width*channels + x*channels + k] = supersample(inputPixels,invM,pixelOut,k);
						}
					}
			}
		}
	}
	return outputPixels;
}


void handleKey(unsigned char key,int x,int y){// handle write and quit
    static int countr = 0;
	switch(key){
        case 'w':
        case 'W': {
            writePixels = displayPixels ;
            write();
            break;
        }
		case 'R':
		case 'r':{
			++countr;
            cout<<"countr = "<<countr<< endl;
			cout << " twirlFlag" << twirlFlag << endl;
			if(countr%2 == 1){
				fixWarp = true;		 
				if (twirlFlag != true){
					displayPixels = warpImage(M , pixels);
				}
				if (twirlFlag == true){
					displayPixels = Twirl(s,cx,cy,pixels);
				}
			}else if(countr%2 == 0){
				fixWarp = false;		 
				if (twirlFlag != true){
					displayPixels = warpImage(M , pixels);
				}
				if (twirlFlag == true){
					displayPixels = Twirl(s,cx,cy,pixels);
				}
			}

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
	float * tempPixels;
    if(argc < 2 || argc > 4){
        std::cerr << "Usage:" << argv[0] <<" read_filename" << " [write_filename]" <<endl;
    }
    filename_read = argv[1];
    filename_write = argv[2];
    read();

	processInput(M);
	cout << "Accumulated Matrix:" << endl;
	cout << M << endl;
	if (twirlFlag != true){
		displayPixels = warpImage(M , pixels);
	}
	if (twirlFlag == true){
		displayPixels = Twirl(s, cx, cy,pixels);			
		cout<<"s = "<< s <<endl;
		cout<<"cx = "<< cx <<endl;
		cout<<"cy = "<< cy <<endl;
	//	displayPixels = warpImage(M , displayPixels);
	}
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowSize(width, height);
    glutCreateWindow("display");
    glutDisplayFunc(displayImage);    // display callback
    glutKeyboardFunc(handleKey);      // keyboard callback
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluOrtho2D(0, width, 0, height);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0);
    glClearColor(1, 1, 1, 1);
    glutMainLoop();
	return 0;
}
