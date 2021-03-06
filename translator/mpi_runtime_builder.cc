// Copyright 2011, Tokyo Institute of Technology.
// All rights reserved.
//
// This file is distributed under the license described in
// LICENSE.txt.
//
// Author: Naoya Maruyama (naoya@matsulab.is.titech.ac.jp)

#include "translator/mpi_runtime_builder.h"
#include "translator/rose_util.h"

namespace sb = SageBuilder;
namespace si = SageInterface;

namespace physis {
namespace translator {

SgFunctionCallExp *BuildCallLoadSubgrid(SgExpression *grid_ref,
                                        SgVariableDeclaration *grid_range,
                                        SgExpression *reuse) {
  SgFunctionSymbol *fs
      = si::lookupFunctionSymbolInParentScopes("__PSLoadSubgrid");
  SgExprListExp *args = sb::buildExprListExp(
      grid_ref, sb::buildAddressOfOp(sb::buildVarRefExp(grid_range)),
      reuse);
  SgFunctionCallExp *call = sb::buildFunctionCallExp(fs, args);
  return call;
}


SgFunctionCallExp *BuildCallLoadSubgridUniqueDim(SgExpression *gref,
                                                 StencilRange &sr,
                                                 SgExpression *reuse) {
  SgExprListExp *args = sb::buildExprListExp(gref);
  for (int i = 0; i < sr.num_dims(); ++i) {
    PSAssert(sr.min_indices()[i].size() == 1);
    args->append_expression(
        rose_util::BuildIntLikeVal(sr.min_indices()[i].front().dim));
    args->append_expression(
        rose_util::BuildIntLikeVal(sr.min_indices()[i].front().offset));
  }
  for (int i = 0; i < sr.num_dims(); ++i) {
    PSAssert(sr.max_indices()[i].size() == 1);
    args->append_expression(
        rose_util::BuildIntLikeVal(sr.max_indices()[i].front().dim));
    args->append_expression(
        rose_util::BuildIntLikeVal(sr.max_indices()[i].front().offset));
  }
  args->append_expression(reuse);
  
  SgFunctionCallExp *call;
  SgFunctionSymbol *fs = NULL;
  if (sr.num_dims() == 3) {
    fs = si::lookupFunctionSymbolInParentScopes("__PSLoadSubgrid3D");
  } else if (sr.num_dims() == 2) {
    fs = si::lookupFunctionSymbolInParentScopes("__PSLoadSubgrid2D");    
  } else {
    PSAbort(1);
  }
  PSAssert(fs);
  call = sb::buildFunctionCallExp(fs, args);  
  return call;
}

SgFunctionCallExp *BuildLoadNeighbor(SgExpression *grid_var,
                                     StencilRange &sr,
                                     SgScopeStatement *scope,
                                     SgExpression *reuse,
                                     SgExpression *overlap,
                                     bool is_periodic) {
  SgFunctionSymbol *load_neighbor_func
      = si::lookupFunctionSymbolInParentScopes("__PSLoadNeighbor");
  IntVector offset_min, offset_max;
  PSAssert(sr.GetNeighborAccess(offset_min, offset_max));
  SgVariableDeclaration *offset_min_decl =
      rose_util::DeclarePSVectorInt("offset_min", offset_min, scope);
  SgVariableDeclaration *offset_max_decl =
      rose_util::DeclarePSVectorInt("offset_max", offset_max, scope);
  bool diag_needed = sr.IsNeighborAccessDiagonalAccessed();
  SgExprListExp *load_neighbor_args =
      sb::buildExprListExp(grid_var,
                           sb::buildVarRefExp(offset_min_decl),                           
                           sb::buildVarRefExp(offset_max_decl),
                           sb::buildIntVal(diag_needed),
                           reuse, overlap,
                           sb::buildIntVal(is_periodic));
  SgFunctionCallExp *fc = sb::buildFunctionCallExp(load_neighbor_func,
                                                   load_neighbor_args);
  return fc;
}

SgFunctionCallExp *BuildActivateRemoteGrid(SgExpression *grid_var,
                                           bool active) {
  SgFunctionSymbol *fs
      = si::lookupFunctionSymbolInParentScopes("__PSActivateRemoteGrid");
  SgExprListExp *args =
      sb::buildExprListExp(grid_var, sb::buildIntVal(active));
  SgFunctionCallExp *fc = sb::buildFunctionCallExp(fs, args);
  return fc;
}

SgFunctionCallExp *MPIRuntimeBuilder::BuildIsRoot() {
  SgFunctionSymbol *fs
      = si::lookupFunctionSymbolInParentScopes("__PSIsRoot");
  SgFunctionCallExp *fc = sb::buildFunctionCallExp(fs);
  return fc;
}

SgFunctionCallExp *MPIRuntimeBuilder::BuildGetGridByID(SgExpression *id_exp) {
  SgFunctionSymbol *fs
      = si::lookupFunctionSymbolInParentScopes("__PSGetGridByID");
  SgFunctionCallExp *fc =
      sb::buildFunctionCallExp(fs, sb::buildExprListExp(id_exp));
  return fc;
}

SgFunctionCallExp *MPIRuntimeBuilder::BuildDomainSetLocalSize(
    SgExpression *dom) {
  SgFunctionSymbol *fs
      = si::lookupFunctionSymbolInParentScopes("__PSDomainSetLocalSize");
  if (!si::isPointerType(dom->get_type())) {
    dom = sb::buildAddressOfOp(dom);
  }
  SgExprListExp *args = sb::buildExprListExp(dom);
  SgFunctionCallExp *fc = sb::buildFunctionCallExp(fs, args);
  return fc;
}

} // namespace translator
} // namespace physis
