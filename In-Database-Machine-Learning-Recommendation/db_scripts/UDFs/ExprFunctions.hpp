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

    static gshared_ptr<topology4::GraphUpdates> graphupdate_static = NULL;
    
  template <typename MESSAGE>
    inline EDGE UDF_insert_edge(gpelib4::SingleValueMapContext<MESSAGE>* context,ServiceAPI* serviceapi, EngineServiceRequest& request, int64_t mc1, int64_t mc2){
      const std::string& edgeType = "hasChildCompany";
        
      auto graphupdate = serviceapi->CreateGraphUpdates(&request);
      auto eid = serviceapi->GetTopologyMeta()->GetEdgeTypeId(edgeType, request.graph_id_);
        auto mc1Type = context->GraphAPI()->GetVertexType(mc1);
        auto mc2Type = context->GraphAPI()->GetVertexType(mc2);
        std::cout<<"UDF%% eid:"<<std::endl;
        std::cout<<eid<<std::endl;
        std::cout<<mc1Type<<std::endl;
        std::cout<<mc2Type<<std::endl;
      auto attr_up = graphupdate->GetEdgeAttributeUpdate(eid);
        std::cout<<attr_up<<std::endl;
      graphupdate->UpsertEdge(
      topology4::DeltaVertexId(context->GraphAPI()->GetVertexType(mc1), mc1),
      topology4::DeltaVertexId(context->GraphAPI()->GetVertexType(mc2), mc2),
      attr_up,
      eid);
      graphupdate->Commit();
      return EDGE(VERTEX(mc1), VERTEX(mc2), eid);
      std::cout<<"UDF%% insert edge"<<std::endl;
    }
    
    
    
    inline int UDF_createGraphUpdate(ServiceAPI* serviceapi, EngineServiceRequest& request){
      graphupdate_static = serviceapi->CreateGraphUpdates(&request);
    }
    
    
    inline int UDF_commit(ServiceAPI* serviceapi, EngineServiceRequest& request){
//      auto graphupdate = serviceapi->CreateGraphUpdates(&request);
      graphupdate_static->Commit();
    }
    
//topology4::GraphUpdates* GetGraphUpdate()
//    gpr::Context* context
//    auto graphupdate = context->topology4::GraphUpdates* GetGraphUpdate();
    
  template <typename MESSAGE>
  inline double connect(gpelib4::SingleValueMapContext<MESSAGE>* context,ServiceAPI* serviceapi, EngineServiceRequest& request, VERTEX mc, VERTEX attr, BagAccum<VERTEX>& neighbors , float wts, float threshold){
      enum GVs {GV_SYS_PC, GV_PARAM_realmWeight, GV_SYS_realmWeight_flag, GV_PARAM_emailWeight, GV_SYS_emailWeight_flag, GV_PARAM_phoneWeight, GV_SYS_phoneWeight_flag, GV_PARAM_nameWeight, GV_SYS_nameWeight_flag, GV_PARAM_threshold, GV_SYS_threshold_flag, GV_PARAM_maxOutdegree, GV_SYS_maxOutdegree_flag, GV_GACC_score, GV_GACC_current_MC, GV_SYS_rangeobject_1, FE_GV_1, FE_GV_1_NULL, MONITOR, MONITOR_ALL, GV_SYS_OLD_PC, GV_SYS_EXCEPTION, GV_SYS_TO_BE_COMMITTED, GV_SYS_LAST_ACTIVE, GV_SYS_EMPTY_INITIALIZED, GV_SYS_VTs, GV_SYS_CompanyAttrs_SIZE, GV_SYS_CompanyAttrs_ORDERBY, GV_SYS_CompanyAttrs_LASTSET, GV_SYS_Start_SIZE, GV_SYS_Start_ORDERBY, GV_SYS_Start_LASTSET};
//      std::cout<<"UDF%% started"<<std::endl;
      MaxAccum<VERTEX>& current = context->GetGlobalVariableLocal(GV_GACC_current_MC)->template GetValue<MaxAccum<VERTEX>>();
      MapAccum<VERTEX, SumAccum<float>>& score =  context->GetGlobalVariableLocal(GV_GACC_score)->template GetValue<MapAccum<VERTEX,SumAccum<float>>>();
      auto graphupdate = serviceapi->CreateGraphUpdates(&request);
      const std::string& edgeType = "sameAs";
      
//      std::cout<<"UDF%% initialization"<<std::endl;
      if (current.data_ != mc){
          current.data_ = mc;
          score.clear();
//          std::cout<<"UDF%% new vertex"<<std::endl;
      }
      for (auto comp_i = neighbors.data_.begin(); comp_i != neighbors.data_.end(); ++comp_i) {
//          score.data_[*comp_i] += wts;
          if (*comp_i != current.data_){
              auto it = score.data_.find(*comp_i);
              if (it != score.data_.end()) {
                it->second += wts;
              } else {
                score.data_.insert(std::pair<int64_t,float> (*comp_i, wts));
    //              std::cout<<"UDF%% add weight"<<std::endl;
              }
              if (score.data_[*comp_i] > threshold){
                  auto eid = serviceapi->GetTopologyMeta()->GetEdgeTypeId(edgeType, request.graph_id_);
                  auto attr_up = graphupdate->GetEdgeAttributeUpdate(eid);
                  graphupdate->UpsertEdge(
                  topology4::DeltaVertexId(context->GraphAPI()->GetVertexType(*comp_i), *comp_i),
                  topology4::DeltaVertexId(context->GraphAPI()->GetVertexType(current.data_), current.data_),
                  attr_up,
                  eid);
              
//              std::cout<<"UDF%% insert edge"<<std::endl;
              }
          }
          
      }
      graphupdate->Commit();
  }
    
  template <typename MESSAGE>
  inline double connectNew(gpelib4::SingleValueMapContext<MESSAGE>* context,ServiceAPI* serviceapi, EngineServiceRequest& request, VERTEX mc, VERTEX attr, ListAccum<VERTEX>& neighbors , float wts, float threshold){
      enum GVs {GV_SYS_PC, GV_PARAM_realmWeight, GV_SYS_realmWeight_flag, GV_PARAM_emailWeight, GV_SYS_emailWeight_flag, GV_PARAM_phoneWeight, GV_SYS_phoneWeight_flag, GV_PARAM_nameWeight, GV_SYS_nameWeight_flag, GV_PARAM_threshold, GV_SYS_threshold_flag, GV_PARAM_maxOutdegree, GV_SYS_maxOutdegree_flag, GV_GACC_score, GV_GACC_current_MC, GV_SYS_rangeobject_1, FE_GV_1, FE_GV_1_NULL, MONITOR, MONITOR_ALL, GV_SYS_OLD_PC,GV_SYS_EXCEPTION, GV_SYS_TO_BE_COMMITTED, GV_SYS_LAST_ACTIVE, GV_SYS_EMPTY_INITIALIZED, GV_SYS_VTs, GV_SYS_CompanyAttrs_SIZE, GV_SYS_CompanyAttrs_ORDERBY, GV_SYS_CompanyAttrs_LASTSET, GV_SYS_Start_SIZE,GV_SYS_Start_ORDERBY, GV_SYS_Start_LASTSET};
//      std::cout<<"UDF%% started"<<std::endl;
      MaxAccum<VERTEX>& current = context->GetGlobalVariableLocal(GV_GACC_current_MC)->template GetValue<MaxAccum<VERTEX>>();
      MapAccum<VERTEX, SumAccum<float>>& score =  context->GetGlobalVariableLocal(GV_GACC_score)->template GetValue<MapAccum<VERTEX,SumAccum<float>>>();
//      auto graphupdate = serviceapi->CreateGraphUpdates(&request);
      const std::string& edgeType = "sameAs";
      
//      std::cout<<"UDF%% initialization"<<std::endl;
      if (current.data_ != mc){
          current.data_ = mc;
          score.clear();
//          std::cout<<"UDF%% new vertex"<<std::endl;
      }
      for (auto comp_i = neighbors.data_.begin(); comp_i != neighbors.data_.end(); ++comp_i) {
//          score.data_[*comp_i] += wts;
          if (*comp_i != current.data_){
              auto it = score.data_.find(*comp_i);
              if (it != score.data_.end()) {
                it->second += wts;
              } else {
                score.data_.insert(std::pair<int64_t,float> (*comp_i, wts));
    //              std::cout<<"UDF%% add weight"<<std::endl;
              }
              if (score.data_[*comp_i] > threshold){
                  auto eid = serviceapi->GetTopologyMeta()->GetEdgeTypeId(edgeType, request.graph_id_);
                  auto attr_up = graphupdate_static->GetEdgeAttributeUpdate(eid);
                  graphupdate_static->UpsertEdge(
                  topology4::DeltaVertexId(context->GraphAPI()->GetVertexType(*comp_i), *comp_i),
                  topology4::DeltaVertexId(context->GraphAPI()->GetVertexType(current.data_), current.data_),
                  attr_up,
                  eid);
              
//              std::cout<<"UDF%% insert edge"<<std::endl;
              }
          }
          
      }
//      graphupdate->Commit();
  }
    
  inline double randn () {
      unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
      std::default_random_engine generator(seed);
      std::normal_distribution<double> distribution(0,1);
      double y = distribution(generator);
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
    
  inline ArrayAccum<SumAccum<double>> diff_ArrayAccum_List (ArrayAccum<SumAccum<double>>& aArray, const ListAccum<double>& bList) {
      ArrayAccum<SumAccum<double>> cArray(aArray.dim_);
      for (uint32_t i=0; i < aArray.data_.size(); i++){
        cArray.data_[i] = aArray.data_[i]-bList.get(i);
    }
      return cArray;
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
    
    //delta = t.@delta[i]*t.@a[i]*(1-t.@a[i])
  /*
  inline void delta_ArrayAccum (ArrayAccum<SumAccum<double>>& aArray, ArrayAccum<SumAccum<double>>& bArray) {
      for (uint32_t i=0; i < aArray.data_.size(); i++){
        aArray.data_[i] = aArray.data_[i]*bArray.data_[i]*(1-bArray.data_[i]);
    }
  } */
    
  inline ArrayAccum<SumAccum<double>> delta_ArrayAccum (ArrayAccum<SumAccum<double>>& aArray, ArrayAccum<SumAccum<double>>& bArray) {
      ArrayAccum<SumAccum<double>> yArray(aArray.dim_);
      for (uint32_t i=0; i < aArray.data_.size(); i++){
        yArray.data_[i] = aArray.data_[i]*bArray.data_[i]*(1-bArray.data_[i])-aArray.data_[i];
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
  
  inline ArrayAccum<SumAccum<double>> sigmoid_ArrayAccum (ArrayAccum<SumAccum<double>>& xArray) {
      ArrayAccum<SumAccum<double>> yArray(xArray.dim_);
      for (uint32_t i=0; i < xArray.data_.size(); i++){
        yArray.data_[i] = 1/(1+exp(-xArray.data_[i]))-xArray.data_[i];
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
