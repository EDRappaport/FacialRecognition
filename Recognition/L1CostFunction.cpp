#include "L1CostFunction.hpp"

L1CostFunction::L1CostFunction(dlib::matrix<double> d, double lambda, dlib::matrix<unsigned char> signal)
{
    _D = d;
    _lambda = lambda;
    column_vector signalCol = dlib::reshape_to_column_vector(dlib::matrix_cast<double>(signal));
    double l2Norm = std::sqrt(dlib::sum(dlib::pointwise_multiply(signalCol, signalCol)));
    _signal = signalCol/l2Norm; 
}


double L1CostFunction::operator()(const column_vector& arg) const
{
    double sum = 0;
    for (int i = 0; i < arg.size(); i++)
    {
	sum += std::abs(arg(i));
    }
    
    column_vector r = (_D * arg) - _signal;   
    column_vector r2 = dlib::pointwise_multiply(r, r); 
    double val = (dlib::sum(r2)) + _lambda*sum;
    
    return val;
}

