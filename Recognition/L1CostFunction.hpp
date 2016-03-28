#ifndef _L1COSTFUNCTION_H
#define _L1COSTFUNCTION_H

#include <iostream>
#include <list>
#include <tuple>
#include <math.h> 

#include <dlib/matrix.h>

#include "Dictionary.hpp"

class L1CostFunction
{
public:
    L1CostFunction(dlib::matrix<double> d,  double lambda, dlib::matrix<unsigned char> signal);
    double operator() ( const column_vector& arg) const;
private:
    dlib::matrix<double> _D;
    double _lambda;
    column_vector _signal;
};

#endif