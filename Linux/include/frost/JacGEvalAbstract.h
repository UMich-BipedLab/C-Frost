#ifndef JACGEVALABSTRACT_H
#define JACGEVALABSTRACT_H

#include "IpTNLP.hpp"

using namespace Ipopt;

namespace frost {
  class JacGEvalAbstract {
 public:
    virtual bool eval_jac_g(Index n, const Number* x, bool new_x,
                            Index m, Index nele_jac, Index* iRow, Index *jCol,
                            Number* values) = 0;
    virtual ~JacGEvalAbstract() {}
  };
}

#endif
