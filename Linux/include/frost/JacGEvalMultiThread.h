#ifndef JACGEVALSINGLETHREAD_H
#define JACGEVALSINGLETHREAD_H

#include "frost/JacGEvalAbstract.h"
#include "rapidjson/document.h"
#include "IpTNLP.hpp"

#include <queue>
#include <mutex>
#include <thread>

using namespace std;
using namespace Ipopt;

namespace frost {
  class JacGEvalMultiThread_worker {

  };

  class JacGEvalMultiThread_workerQueue {
  public:
    JacGEvalMultiThread_threadInstance* getWorker()
    {
      lock.lock();
      if (q.length() == 0)
      {
        
      }

      JacGEvalMultiThread_threadInstance *worker = q.front();
      q.pop();
      lock.unlock();

      return worker;
    }

    void returnWorker(JacGEvalMultiThread_threadInstance *worker)
    {
      lock.lock();
      q.push(worker);
      lock.unlock();
    }
  private:
    mutex lock;
    queue<JacGEvalMultiThread_threadInstance*> q;
  };

  class JacGEvalMultiThread : public JacGEvalAbstract {
  public:
    JacGEvalMultiThread(rapidjson::Document &document);
    virtual ~JacGEvalMultiThread();
    virtual bool eval_jac_g(Index n, const Number* x, bool new_x,
                            Index m, Index nele_jac, Index* iRow, Index *jCol,
                            Number* values);

  public:
    rapidjson::Document *document;

  private:
    Number *in;   // temporary input variables
    Number *out;  // temporary function outputs
    mutex valueLock;

  public:
    Index n_var; // the number of optimization variables
    Index n_constr; // the number of constraints
    Index nnz_jac_g; // the number of non-zeros in constraint Jacobian
  };
}

#endif
