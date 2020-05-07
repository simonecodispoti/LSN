#ifndef UTILITIES_H
#define UTILITIES_H
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <algorithm>
using namespace std;

/* || Library for data analysis and generic utilitities ||

Note: The type T cannot be completely arbitrary! For functions like " MC_Mean_Error " 
T must be a 'numeric' type of some kynd, e.g. int, float, double ...

*/


template <class T> void Print(const char* filename, const vector <T>& v){        // Print to file "filename"

	ofstream out;
	out.open(filename);

	if(out.fail()){
		cerr << "Cannot open: "<< filename << "!" << endl;
		exit(-1);
	}else{
		for(int i=0; i<v.size(); i++)
			out << v[i] << endl;
	}

	out.close();
};

template <class T> void Print(const vector <T>& v){          // Print to terminal

    for(int i=0; i<v.size(); i++) cout << v[i] << endl;

};

template <class T> vector <T> Read(const char* filename){        // Read from file "filename"

	vector <T> v;

	ifstream in;
	in.open(filename);

	if(in.fail()){
		cerr << "Cannot open: "<< filename << endl;
		exit(-1);
	}else{
		while(!in.eof()){
			T data = 0;
			in >> data;
			v.push_back(data);
		}	
	}
	v.pop_back();		// delete the zero in the end of vector

	in.close();

	return v;
};

template <class T> void MC_Mean_Error(const vector <T>& input, vector <T>& average, vector <T>& error, const int n_step, const int n_cell){

	int l = n_step/n_cell;
	
	vector <T> ave;
	vector <T> ave2;
	vector <T> sum2_prog;

	for(int i=0; i<n_cell; i++){
		ave.push_back(0);
		ave2.push_back(0);
		sum2_prog.push_back(0);
		average.push_back(0);
		error.push_back(0);
	}

	for(int i=0; i<n_cell; i++){
		double sum = 0;
			for(int j=0; j<l; j++){
				int pos = j+i*l;
				sum += input[pos];
			}
		ave[i] = sum/l;             // averages for each block
		ave2[i] = pow(ave[i],2);   // squared averages for each block
	}

	for(int i=0; i<n_cell; i++){
		for(int j=0; j<i+1; j++){
			average[i] += ave[j];
			sum2_prog[i] += ave2[j];
		}
	average[i] /= (i+1);
	sum2_prog[i] /= (i+1);

	if(i!=0)
		error[i] = sqrt((sum2_prog[i]-pow(average[i],2))/i);		// statistical uncertainties
	}

	error[0] = 0;		// no statistical uncertainty after one block
};

template <class T> double Chi2(const vector <T>& observed, const vector <T>& expected){       // Chi2 test
	
	double Chi2 = 0;
    if(observed.size() != expected.size()){
        cerr << "Chi 2 test failed: The size of observations vector must be equal to the size fo expections vector!" << endl;
        exit(-2);
    }else{
       	for(int i=0; i<expected.size(); i++) Chi2 += (pow(observed[i]-expected[i],2))/expected[i];	

	    return Chi2; 
    }
};

#endif /* UTILITIES_H */