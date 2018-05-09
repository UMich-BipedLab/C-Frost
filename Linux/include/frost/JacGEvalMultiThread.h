#ifndef JACGEVALMULTITHREAD_H
#define JACGEVALMULTITHREAD_H

#include "frost/JacGEvalAbstract.h"
#include "rapidjson/document.h"
#include "IpTNLP.hpp"

#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;
using namespace Ipopt;

namespace frost
{
  
class JacGEvalMultiThread_worker;
class MyMonitor;

class JacGEvalMultiThread : public JacGEvalAbstract
{
public:
  JacGEvalMultiThread(rapidjson::Document &document);
  virtual ~JacGEvalMultiThread();
  virtual bool eval_jac_g(Index n, const Number *x, bool new_x,
                          Index m, Index nele_jac, Index *iRow, Index *jCol,
                          Number *values);

public:
  rapidjson::Document *document;
  std::mutex lock;
  std::condition_variable signal_done;

public:
  Index n_var;     // the number of optimization variables
  Index n_constr;  // the number of constraints
  Index nnz_jac_g; // the number of non-zeros in constraint Jacobian

private:
  frost::MyMonitor *monitor;
};
}

#endif
