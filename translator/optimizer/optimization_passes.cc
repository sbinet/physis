// Copyright 2011, Tokyo Institute of Technology.
// All rights reserved.
//
// This file is distributed under the license described in
// LICENSE.txt.
//
// Author: Naoya Maruyama (naoya@matsulab.is.titech.ac.jp)

#include "translator/optimizer/optimization_passes.h"
#include "translator/rose_util.h"

namespace si = SageInterface;
namespace sb = SageBuilder;

namespace physis {
namespace translator {
namespace optimizer {
namespace pass {

void null_optimization(
    SgProject *proj,
    physis::translator::TranslationContext *tx,
    physis::translator::RuntimeBuilder *builder) {
  pre_process(proj, tx, __FUNCTION__);
}

} // namespace pass
} // namespace optimizer
} // namespace translator
} // namespace physis

