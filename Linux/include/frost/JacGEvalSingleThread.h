#ifndef JACGEVALSINGLETHREAD_H
#define JACGEVALSINGLETHREAD_H

#include "frost/JacGEvalAbstract.h"
#include "rapidjson/document.h"
#include "IpTNLP.hpp"

using namespace Ipopt;

namespace frost {
  class JacGEvalSingleThread : public JacGEvalAbstract {
 public:
    JacGEvalSingleThread(rapidjson::Document &document);
    virtual ~JacGEvalSingleThread();
    virtual bool eval_jac_g(Index n, const Number* x, bool new_x,
                            Index m, Index nele_jac, Index* iRow, Index *jCol,
                            Number* values);

 public:
    rapidjson::Document *document;

 private:
    Number *in;   // temporary input variables
    Number *out;  // temporary function outputs

 public:
    Index n_var; // the number of optimization variables
    Index n_constr; // the number of constraints
    Index nnz_jac_g; // the number of non-zeros in constraint Jacobian
  };
}

#endif
