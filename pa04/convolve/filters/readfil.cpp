#include <iostream>  
#include <fstream>  
#include <stdlib.h>
using namespace std;

int main () {  
	          
	ifstream kernelfile; 
	kernelfile.open("sobol-horiz.filt");  
	if (! kernelfile.is_open())  {
		cout << "Error opening file"; exit (1);
	}  
	int N;
	kernelfile >> N;  
	double **kernel = new double*[N];
	for(int i = 0 ; i < N ; i++){
		filt[i] = new double[N];
	}
	for(int i = 0 ; i < N ; i++){
		for(int j = 0 ; j < N ;j++){
			kernelfile >> kernel[i][j];
			cout << kernel[i][j]<< endl;
		}
	}
	return 0;  
										    
}  
