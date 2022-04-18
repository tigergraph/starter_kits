/******************************************************************************
 * Copyright (c) 2015-2016, TigerGraph Inc.
 * All rights reserved.
 * Project: TigerGraph Query Language
 * udf.hpp: a library of user defined functions used in queries.
 *
 * - This library should only define functions that will be used in
 *   TigerGraph Query scripts. Other logics, such as structs and helper
 *   functions that will not be directly called in the GQuery scripts,
 *   must be put into "ExprUtil.hpp" under the same directory where
 *   this file is located.
 *
 * - Supported type of return value and parameters
 *     - int
 *     - float
 *     - double
 *     - bool
 *     - string (don't use std::string)
 *     - accumulators
 *
 * - Function names are case sensitive, unique, and can't be conflict with
 *   built-in math functions and reserve keywords.
 *
 * - Please don't remove necessary codes in this file
 *
 * - A backup of this file can be retrieved at
 *     <tigergraph_root_path>/dev_<backup_time>/gdk/gsql/src/QueryUdf/ExprFunctions.hpp
 *   after upgrading the system.
 *
 ******************************************************************************/

#ifndef EXPRFUNCTIONS_HPP_
#define EXPRFUNCTIONS_HPP_

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <gle/engine/cpplib/headers.hpp>

#include <math.h>
#include <random>
#include <chrono>


/**     XXX Warning!! Put self-defined struct in ExprUtil.hpp **
 *  No user defined struct, helper functions (that will not be directly called
 *  in the GQuery scripts) etc. are allowed in this file. This file only
 *  contains user-defined expression function's signature and body.
 *  Please put user defined structs, helper functions etc. in ExprUtil.hpp
 */
#include "ExprUtil.hpp"

namespace UDIMPL {
  typedef std::string string; //XXX DON'T REMOVE

  /****** BIULT-IN FUNCTIONS **************/
  /****** XXX DON'T REMOVE ****************/
  inline int64_t str_to_int (string str) {
    return atoll(str.c_str());
  }

  inline int64_t float_to_int (float val) {
    return (int64_t) val;
  }

  inline string to_string (double val) {
    char result[200];
    sprintf(result, "%g", val);
    return string(result);
  }

  inline double randn () {
      unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
      std::default_random_engine generator(seed);
      std::normal_distribution<double> distribution(0,1);
      double y = distribution(generator);
      return y;
  }
    
  inline double rand_uniform () {
      unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
      srand(seed);
      double y = ((double) rand() / (RAND_MAX));
      return y;
  }
    
  inline int64_t rand_choice (MapAccum<int64_t, double>& Prob_Map) {
      std::vector<double> y_prob;
      std::vector<int64_t> y_id;
      for (auto yp : Prob_Map){
        y_id.push_back(yp.first);
        y_prob.push_back(yp.second);
      }
      unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
      std::default_random_engine generator(seed);
      std::discrete_distribution<int> distribution(y_prob.begin(),y_prob.end());
      int id = distribution(generator);
      return y_id[id];
  }
    
  inline int random_distribution(ListAccum<float> p){
//    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
//    std::default_random_engine gen(seed);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> dis(p.data_.begin(), p.data_.end());
    return dis(gen);
  }
    
  inline double sum_ArrayAccum (ArrayAccum<SumAccum<double>>& xArray) {
      double res = 0;
      for (uint32_t i=0; i < xArray.data_.size(); i++){
        res += xArray.data_[i];
    }
      return res;
  }
    
  inline ArrayAccum<SumAccum<double>> product_List_const (const ListAccum<double>& xList, double x) {
      gvector<int64_t> dim;
      dim.push_back(xList.size());
      ArrayAccum<SumAccum<double>> yArray(dim);
      for (uint32_t i=0; i < xList.size(); i++){
        yArray.data_[i] = x*xList.get(i);
    }
// return std::move(yArray);
      return yArray;
  }
    
  inline ListAccum<double> AXplusBY_List (double a, const ListAccum<double>& xList, double b, const ListAccum<double>& yList) {
      ListAccum<double> resList;
      for (uint32_t i=0; i < xList.size(); i++){
        resList.data_.push_back(a*xList.get(i)+b*yList.get(i));
    }
      return resList;
  }
    
  inline ListAccum<double> updateFeatures (double alpha, double a, const ListAccum<double>& xList, double b, const ListAccum<double>& yList) {
      ListAccum<double> resList;
      for (uint32_t i=0; i < xList.size(); i++){
        resList.data_.push_back(yList.get(i)-alpha*(a*xList.get(i)+b*yList.get(i)));
    }
      return resList;
  }

  inline ArrayAccum<SumAccum<double>> product_ArrayAccum_const (ArrayAccum<SumAccum<double>>& xArray, double x) {
      ArrayAccum<SumAccum<double>> yArray(xArray.dim_);
      for (uint32_t i=0; i < xArray.data_.size(); i++){
        yArray.data_[i] = x*xArray.data_[i];
    }
//    return std::move(yArray);
      return yArray;
  }
  
  inline ArrayAccum<SumAccum<double>> product_Matrix_const (const ArrayAccum<SumAccum<double>>& X, double c) {
      ArrayAccum<SumAccum<double>> Y(X.dim_);
      for (uint32_t i=0; i < X.data_.size(); i++){
        Y.data_[i] = c*X.data_[i];
    }
//    return std::move(yArray);
      return Y;
  }
    
  inline ArrayAccum<SumAccum<double>> product_MatrixSqr_const (const ArrayAccum<SumAccum<double>>& X, double c) {
      ArrayAccum<SumAccum<double>> Y(X.dim_);
      for (uint32_t i=0; i < X.data_.size(); i++){
        Y.data_[i] = c*X.data_[i]*X.data_[i];
    }
//    return std::move(yArray);
      return Y;
  }
    
  inline ArrayAccum<SumAccum<double>> AdamGrdient (const ArrayAccum<SumAccum<double>>& VdW, const ArrayAccum<SumAccum<double>>& SdW, double t, double alpha, double beta1, double beta2) {
      ArrayAccum<SumAccum<double>> D(VdW.dim_);
      for (uint32_t i=0; i < VdW.data_.size(); i++){
        D.data_[i] = -alpha*(VdW.data_[i]/(1-pow(beta1,t))/(sqrt(SdW.data_[i]/(1-pow(beta2,t)))+0.00000001));
    }
//    return std::move(yArray);
      return D;
  }
    
  inline ArrayAccum<SumAccum<double>> product_ConstArrayAccum_const (const ArrayAccum<SumAccum<double>>& xArray, double x) {
      ArrayAccum<SumAccum<double>> yArray(xArray.dim_);
      for (uint32_t i=0; i < xArray.data_.size(); i++){
        yArray.data_[i] = x*xArray.data_[i];
    }
//    return std::move(yArray);
      return yArray;
  }
    
  inline ArrayAccum<SumAccum<double>> diff_ArrayAccum_List (ArrayAccum<SumAccum<double>>& aArray, const ListAccum<double>& bList) {
      ArrayAccum<SumAccum<double>> cArray(aArray.dim_);
      for (uint32_t i=0; i < aArray.data_.size(); i++){
        cArray.data_[i] = aArray.data_[i]-bList.get(i);
    }
      return cArray;
  }
    
  inline ArrayAccum<SumAccum<double>> product_sparseVector_ArrayAccum (MapAccum<int64_t,double>& u, ArrayAccum<SumAccum<double>>& v, int64_t m, int64_t n) {
      gvector<int64_t> y_dim;
      y_dim.push_back(m);
      y_dim.push_back(n);
      ArrayAccum<SumAccum<double>> y(y_dim);
      for (uint32_t i=0; i < v.data_.size(); i++){
          for (auto it = u.data_.begin(); it != u.data_.end(); ++it){
              y.data_[(it->first)*n+i] += v.data_[i]*(it->second);
          }
    }
      return y;
  }
   
  inline ArrayAccum<SumAccum<double>> product_ArrayAccum_ArrayAccum (ArrayAccum<SumAccum<double>>& u, ArrayAccum<SumAccum<double>>& v) {
      gvector<int64_t> y_dim;
      int64_t n = v.data_.size();
      y_dim.push_back(u.data_.size());
      y_dim.push_back(n);
      ArrayAccum<SumAccum<double>> y(y_dim);
      for (uint32_t i=0; i < u.data_.size(); i++){
          for (uint32_t j=0; j < v.data_.size(); j++){
              y.data_[i*n+j] += u.data_[i]*v.data_[j];
          }
    }
      return y;
  }
    
  inline ArrayAccum<SumAccum<double>> product_Matrix_SparseVector (const ArrayAccum<SumAccum<double>>& M, MapAccum<int64_t,double>& x) {
      gvector<int64_t> y_dim;
      int64_t n = M.dim_[1];
      y_dim.push_back(n);
      ArrayAccum<SumAccum<double>> y(y_dim);
      for (uint32_t i=0; i < y.data_.size(); i++){
          for (auto it = x.data_.begin(); it != x.data_.end(); ++it){
              y.data_[i] += M.data_[(it->first)*n+i]*(it->second);
          }
    }
      return y;
  }
  
  inline ArrayAccum<SumAccum<double>> product_Matrix_Vector (const ArrayAccum<SumAccum<double>>& M, ArrayAccum<SumAccum<double>>& x) {
      gvector<int64_t> y_dim;
      int64_t n = M.dim_[1];
      y_dim.push_back(n);
      ArrayAccum<SumAccum<double>> y(y_dim);
      for (uint32_t i=0; i < y.data_.size(); i++){
          for (uint32_t j=0; j < x.data_.size(); j++){
              y.data_[i] += M.data_[j*n+i]*x.data_[j];
          }
    }
      return y;
  }

  inline ArrayAccum<SumAccum<double>> product_Vector_Matrix (const ArrayAccum<SumAccum<double>>& M, ArrayAccum<SumAccum<double>>& x) {
      gvector<int64_t> y_dim;
      int64_t m = M.dim_[0];
      y_dim.push_back(m);
      ArrayAccum<SumAccum<double>> y(y_dim);
      for (uint32_t i=0; i < y.data_.size(); i++){
          for (uint32_t j=0; j < x.data_.size(); j++){
              y.data_[i] += M.data_[i*m+j]*x.data_[j];
          }
    }
      return y;
  }
    
  inline double dotProduct_ArrayAccum_List (ArrayAccum<SumAccum<double>>& aArray, const ListAccum<double>& bList) {
      double c = 0;
      for (uint32_t i=0; i < aArray.data_.size(); i++){
        c += bList.get(i)*aArray.data_[i];
    }
      return c;
  }
    
  inline double dotProduct_List_List (const ListAccum<double>& aList, const ListAccum<double>& bList) {
      double c = 0;
      for (uint32_t i=0; i < aList.size(); i++){
        c += aList.get(i)*bList.get(i);
    }
      return c;
  }
    
  inline double dotProduct_ArrayAccum_ArrayAccum (ArrayAccum<SumAccum<double>>& aArray, ArrayAccum<SumAccum<double>>& bArray) {
      double c = 0;
      for (uint32_t i=0; i < aArray.data_.size(); i++){
        c += bArray.data_[i]*aArray.data_[i];
    }
      return c;
  }
    
  inline int test_2dArray (ArrayAccum<SumAccum<double>>& aArray) {
      std::cout<<"Matrix_info"<<std::endl;
      aArray.print();
  }
    
    //delta = t.@delta[i]*t.@a[i]*(1-t.@a[i])
  /*
  inline void delta_ArrayAccum (ArrayAccum<SumAccum<double>>& aArray, ArrayAccum<SumAccum<double>>& bArray) {
      for (uint32_t i=0; i < aArray.data_.size(); i++){
        aArray.data_[i] = aArray.data_[i]*bArray.data_[i]*(1-bArray.data_[i]);
    }
  } */
    
//  inline ArrayAccum<SumAccum<double>> delta_ArrayAccum (ArrayAccum<SumAccum<double>>& aArray, ArrayAccum<SumAccum<double>>& bArray) {
//      ArrayAccum<SumAccum<double>> yArray(aArray.dim_);
//      std::vector<SumAccum<double>>& data = yArray.data_;
//      for (uint32_t i=0; i < aArray.data_.size(); i++){
//        yArray.data_[i] = aArray.data_[i]*bArray.data_[i]*(1-bArray.data_[i])-aArray.data_[i];
//    }
//      return yArray;
//  }
   
  //cross entropy for one vs all logistic regression
  inline double cross_entropy_ArrayAccum_label (ArrayAccum<SumAccum<double>>& aArray, int indx) {
      double y = 0;
      for (uint32_t i=0; i < aArray.data_.size(); i++){
          if (i == indx){
              y -= log(aArray.data_[i]);
          } else{
              y -= log(1-aArray.data_[i]);
          }
      }
      return y;
  }
    
  inline ArrayAccum<SumAccum<double>> diff_ArrayAccum_oneHotVec (ArrayAccum<SumAccum<double>>& aArray, int indx) {
      ArrayAccum<SumAccum<double>> yArray(aArray.dim_);
      for (uint32_t i=0; i < aArray.data_.size(); i++){
        yArray.data_[i] = aArray.data_[i];
      }
      yArray.data_[indx] += -1;
      return yArray;
  }
    
  inline ArrayAccum<SumAccum<double>> greater_than_zero_ArrayAccum_ArrayAccum (ArrayAccum<SumAccum<double>>& aArray, ArrayAccum<SumAccum<double>>& bArray) {
      ArrayAccum<SumAccum<double>> yArray(aArray.dim_);
      for (uint32_t i=0; i < aArray.data_.size(); i++){
          if (bArray.data_[i] > 0){
              yArray.data_[i] = aArray.data_[i];
          } else{
              yArray.data_[i] = 0;
          }
      }
      return yArray;
  }
    
    
  // cost = -(s.y.get(i)*log(s.@a[i])+(1-s.y.get(i))*log(1-s.@a[i]))
  inline double cost_ArrayAccum_List (ArrayAccum<SumAccum<double>>& aArray, const ListAccum<double>& bList) {
      double c = 0;
      for (uint32_t i=0; i < aArray.data_.size(); i++){
        c += -(bList.get(i)*log(aArray.data_[i])+(1-bList.get(i))*log(1-aArray.data_[i]));
    }
      return c;
  }
    
  /*
  inline void sigmoid_ArrayAccum (ArrayAccum<SumAccum<double>>& xArray) {
      for (uint32_t i=0; i < xArray.data_.size(); i++){
        xArray.data_[i] = 1/(1+exp(-xArray.data_[i]));
    }
  } */

  inline double L2Norm_Matrix (const ArrayAccum<SumAccum<double>>& M) {
      double c = 0;
      for (uint32_t i=0; i < M.data_.size(); i++){
        c += M.data_[i]*M.data_[i];
    }
      return c;
  }
    
  inline MapAccum<int64_t,double> dropout_SparseVector (MapAccum<int64_t,double>& x, double p) {
      MapAccum<int64_t,double> y;
      unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
      srand(seed);
//      std::cout<<"randomNumber:"<<((double) rand() / (RAND_MAX))<<std::endl;
      for (auto it = x.data_.begin(); it != x.data_.end(); ++it) {
          if (((double) rand() / (RAND_MAX)) < p){
              y.data_[it->first] += (it->second)/p;
          }
      }
      return y;
  }
    
  inline ArrayAccum<SumAccum<double>> dropout_ArrayAccum (ArrayAccum<SumAccum<double>>& xArray, double p) {
      unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
      srand(seed);
      ArrayAccum<SumAccum<double>> yArray(xArray.dim_);
      for (uint32_t i=0; i < xArray.data_.size(); i++){
        if (((double) rand() / (RAND_MAX)) < p){
              yArray.data_[i] = xArray.data_[i]/p;
        }
    }
      return yArray;
  }
    
  inline ArrayAccum<SumAccum<double>> sigmoid_ArrayAccum (ArrayAccum<SumAccum<double>>& xArray) {
      ArrayAccum<SumAccum<double>> yArray(xArray.dim_);
      for (uint32_t i=0; i < xArray.data_.size(); i++){
        yArray.data_[i] = 1/(1+exp(-xArray.data_[i]))-xArray.data_[i];
    }
      return yArray;
  }

  inline ArrayAccum<SumAccum<double>> ReLU_ArrayAccum (ArrayAccum<SumAccum<double>>& xArray) {
      ArrayAccum<SumAccum<double>> yArray(xArray.dim_);
      for (uint32_t i=0; i < xArray.data_.size(); i++){
          if (xArray.data_[i] > 0){
              yArray.data_[i] = xArray.data_[i];
          } else{
              yArray.data_[i] = 0;
          }
      }
      return yArray;
  }
    
  inline ArrayAccum<SumAccum<double>> softmax_ArrayAccum (ArrayAccum<SumAccum<double>>& xArray) {
      ArrayAccum<SumAccum<double>> yArray(xArray.dim_);
      double sum = 0;
      for (uint32_t i=0; i < xArray.data_.size(); i++){
          yArray.data_[i] = exp(xArray.data_[i]);
          sum += yArray.data_[i];
      }
      for (uint32_t i=0; i < xArray.data_.size(); i++){
          yArray.data_[i] = yArray.data_[i]/sum;
      }
      return yArray;
  }
    
  inline ListAccum<double> unit_List (int len) {
      ListAccum<double> yList;
      for (uint32_t i=0; i < len; i++){
        yList.data_.push_back(1);
    }
      return yList;
  }
    
  inline ArrayAccum<SumAccum<double>> unit_ArrayAccum (int len) {
      gvector<int64_t> dim;
      dim.push_back(len);
      ArrayAccum<SumAccum<double>> yArray(dim);
      for (uint32_t i=0; i < yArray.data_.size(); i++){
        yArray.data_[i] = 1;
    }
      return yArray;
  }
}
/****************************************/

#endif /* EXPRFUNCTIONS_HPP_ */
