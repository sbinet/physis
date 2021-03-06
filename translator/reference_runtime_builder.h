// Copyright 2011, Tokyo Institute of Technology.
// All rights reserved.
//
// This file is distributed under the license described in
// LICENSE.txt.
//
// Author: Naoya Maruyama (naoya@matsulab.is.titech.ac.jp)

#ifndef PHYSIS_TRANSLATOR_REFERENCE_RUNTIME_BUILDER_H_
#define PHYSIS_TRANSLATOR_REFERENCE_RUNTIME_BUILDER_H_

#include "translator/translator_common.h"
#include "translator/map.h"
#include "translator/runtime_builder.h"

namespace physis {
namespace translator {

class ReferenceRuntimeBuilder: public RuntimeBuilder {
 public:
  ReferenceRuntimeBuilder(SgScopeStatement *global_scope);
  virtual ~ReferenceRuntimeBuilder() {}
  virtual SgFunctionCallExp *BuildGridGetID(SgExpression *grid_var);
  virtual SgBasicBlock *BuildGridSet(
      SgExpression *grid_var, int num_dims,
      const SgExpressionPtrList &indices, SgExpression *val);
  virtual SgFunctionCallExp *BuildGridDim(SgExpression *grid_ref,
                                          int dim);
  virtual SgExpression *BuildGridRefInRunKernel(
      SgInitializedName *gv,
      SgFunctionDeclaration *run_kernel);

  virtual SgExpression *BuildGridGet(
      SgExpression *gvref,
      GridType *gt,      
      SgExpressionPtrList *offset_exprs,
      const StencilIndexList *sil,      
      bool is_kernel,
      bool is_periodic);
  
  //!
  /*!
   */
  virtual SgExpression *BuildGridOffset(
      SgExpression *gvref, int num_dim,
      SgExpressionPtrList *offset_exprs, bool is_kernel,
      bool is_periodic, const StencilIndexList *sil);

  /*
  virtual SgExpression *BuildGet(  
    SgInitializedName *gv,
    SgExprListExp *offset_exprs,
    SgScopeStatement *scope,
    TranslationContext *tx, bool is_kernel,
    bool is_periodic);
  */
  
  
 protected:
  SgType *index_t_;
  static const std::string  grid_type_name_;
  SgClassDeclaration *GetGridDecl();
};

} // namespace translator
} // namespace physis


#endif /* PHYSIS_TRANSLATOR_REFERENCE_RUNTIME_BUILDER_H_ */
