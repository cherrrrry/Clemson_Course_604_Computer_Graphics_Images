/*
 * Program template for PA06 to warp an image using matrix-based warps
 *
 * Command line parameters are as follows:
 *
 * warper infile.png [outfile.png]
 *
 * Author: Joshua A. Levine, 11/3/15
 */

#include "Matrix.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

/*
 Convert the string s to lower case
 */
void lowercase(char *s){
    int i;
    
    if(s != NULL) {
        for(i = 0; s[i] != '\0'; i++) {
            if(s[i] >= 'A' && s[i] <= 'Z')
                s[i] += ('a' - 'A');
        }
    }
}



/*
 Multiply M by a rotation matrix of angle theta
 */

void Rotate(Matrix3x3 &M, float theta){
    int row, col;
    Matrix3x3 R(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    double rad, c, s;
    
    rad = PI * theta / 180.0;
    c = cos(rad);
    s = sin(rad);
    
    R[0][0] = c;
    R[0][1] = -s;
    R[1][0] = s;
    R[1][1] = c;
    
    Matrix3x3 Prod = R * M;
    
    for(row = 0; row < 3; row++) {
        for(col = 0; col < 3; col++) {
            M[row][col] = Prod[row][col];
        }
    }
}



/*
 Routine to build a projective transform from input text, display, or
 write transformed image to a file
 */
void process_input(Matrix3x3 &M){
    char command[1024];
    bool done;
    float theta;
    
    
    /* build identity matrix */
    M.identity();
    
    for(done = false; !done;) {
        
        /* prompt and accept input, converting text to lower case */
        printf("> ");
        scanf("%s", command);
        lowercase(command);
        
        /* parse the input command, and read parameters as needed */
        if(strcmp(command, "d") == 0) {
            done = true;
        } else if(strlen(command) != 1) {
            printf("invalid command, enter r, s, t, h, d\n");
        } else {
            switch(command[0]) {
                    
                case 'r':		/* Rotation, accept angle in degrees */
                    if(scanf("%f", &theta) == 1)
                        Rotate(M, theta);
                    else
                        fprintf(stderr, "invalid rotation angle\n");
                    break;
                case 's':		/* Scale, accept scale factors */
                    break;
                case 't':		/* Translation, accept translations */
                    break;
                case 'h':		/* Shear, accept shear factors */
                    break;
                case 'd':		/* Done, that's all for now */
                    done = true;
                    break;
                default:
                    printf("invalid command, enter r, s, t, h, d\n");
            }
        }
    }
}

/*
 Main program to read an image file, then ask the user
 for transform information, transform the image and display
 it using the appropriate warp.  Optionally save the transformed
 images in  files.
 */
int main(int argc, char *argv[]){
   // Matrix3x3 M(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
     Matrix3x3 M(0, 2, -100, 2, 0, 0, 0.01, 0, 1);
	 cout<<" M :"<<endl;
	 cout<< M << endl;
	 Matrix3x3 invM = M.inv();
     cout<<"invM :"<<endl;
	 cout<< invM << endl;
     
	 Vector3d firstOut(40,40,1);
	 Vector3d firstIn = invM*firstOut;
	 firstIn[0] = round(firstIn[0]/firstIn[2]); 
	 firstIn[1] = round(firstIn[1]/firstIn[2]); 
	 firstIn[2] = round(firstIn[2]/firstIn[2]); 
	 cout<<"firstIn:"<<firstIn<<endl; 
	 
	 
	 
	 //read in the input image
    //
    
    //next, build the transformation matrix
    //process_input(M);
    
    
    cout << "Accumulated Matrix: " << endl;
    cout << M << endl;
    
    //finally display the warped image
    
    return 0;
}


