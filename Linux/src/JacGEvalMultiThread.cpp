#include "frost/JacGEvalMultiThread.h"
#include "frost/Document.h"
#include "frost/functionlist.hh"
#include "IpTNLP.hpp"

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <cassert>

using namespace Ipopt;
using namespace std;

namespace frost
{
  class JacGEvalMultiThread_worker
  {
  public:
    JacGEvalMultiThread_worker(int nJOutConst, frost::MyMonitor *monitor, frost::Document *document);
    ~JacGEvalMultiThread_worker();
    void start();
    void Compute();

  private:
    Number *in;  // temporary input variables
    Number *out; // temporary function outputs
    frost::MyMonitor *monitor;
    frost::Document *document;
    std::thread *th;
  };
}

class frost::MyMonitor
{
public:
  MyMonitor(int numThreads, int nJOutConst, frost::Document *document);
  ~MyMonitor();
  void resetConstraints(int num_of_const, const Number* x);
  void setValues(Number* values);
  int getConstraint();
  void updateValues(Number *out, int i);
  void waitTillReady();
  const Number * getX();

private:
  std::mutex lock;
  int next_const_index;
  int num_of_const;
  int num_of_const_done;
  std::condition_variable signal_done;
  std::condition_variable queue_available;
  Number* values;
  const Number *x;
  frost::Document *document;
  std::vector<frost::JacGEvalMultiThread_worker*> thread_obj;
  bool done;
};

frost::JacGEvalMultiThread_worker::JacGEvalMultiThread_worker(int nJOutConst, frost::MyMonitor *monitor, frost::Document *document)
{
  this->in = new Number[nJOutConst];
  this->out = new Number[nJOutConst];
  this->monitor = monitor;
  this->document = document;
}

frost::JacGEvalMultiThread_worker::~JacGEvalMultiThread_worker()
{
  th->join();

  delete []in;
  delete []out;
}

void frost::JacGEvalMultiThread_worker::start()
{
  //std::cout << "Making myself!" << std::endl;
  this->th = new std::thread(&frost::JacGEvalMultiThread_worker::Compute, this);
}

void frost::JacGEvalMultiThread_worker::Compute()
{
  //std::cout << "In thread compute" << std::endl;
  while(true)
  {
    int i = monitor->getConstraint();

    if (i < 0)
      break;
    
    const Number *x = monitor->getX();

    //if ((*document).Constraint.nzJacIndices[i].IsNull())
    //{
    //  monitor->updateValues(out, -1);
    //  continue;
    //}

    int fIdx = (*document).Constraint.JacFuncs[i] - 1;
    unsigned long int numDep = (*document).Constraint.DepIndices[i].size();
    for (unsigned long int j = 0; j < numDep; j++)
    {
      in[j] = x[(*document).Constraint.DepIndices[i][j] - 1];
    }

    //std::cout<<"b"<<std::endl;
    unsigned long int numAux = (*document).Constraint.AuxData[i].size();
    for (unsigned long int j = 0; j < numAux; j++)
    {
      in[j + numDep] = (*document).Constraint.AuxData[i][j];
    }

    frost::functions[fIdx](out, in);

    //std::cout<<"c"<<std::endl;
    monitor->updateValues(out, i);
  }
}

frost::MyMonitor::MyMonitor(int numThreads, int nJOutConst, frost::Document *document)
{
  this->document = document;

  num_of_const_done = 0;
  num_of_const = 0;
  next_const_index = 0;
  done = false;

  for (int i = 0; i < numThreads; i++)
  {
    thread_obj.push_back(new frost::JacGEvalMultiThread_worker(nJOutConst, this, document));
    thread_obj[i]->start();
  }
}

frost::MyMonitor::~MyMonitor()
{
  done = true;
  queue_available.notify_all();

  for (unsigned int i = 0; i < thread_obj.size(); i++)
    delete thread_obj[i];
}

void frost::MyMonitor::resetConstraints(int num_of_const, const Number *x)
{
  std::unique_lock<std::mutex> lk(lock);

  this->num_of_const = num_of_const;
  next_const_index = 0;
  num_of_const_done = 0;

  queue_available.notify_all();

  this->x = x;
}

void frost::MyMonitor::setValues(Number* values)
{
  std::unique_lock<std::mutex> lk(lock);
  
  this->values = values;
}

int frost::MyMonitor::getConstraint()
{
  std::unique_lock<std::mutex> lk(lock);

  while (next_const_index == num_of_const || done)
  {
    queue_available.wait(lk);
  }

  if (done)
    return -1;

  int x = next_const_index;
  next_const_index++;

  return x;
}

void frost::MyMonitor::updateValues(Number *out, int i)
{
  std::unique_lock<std::mutex> lk(lock);

  if (i >= 0)
  {
    unsigned long int numConst = (*document).Constraint.nzJacIndices[i].size();
    for (unsigned long int j = 0; j < numConst; j++)
    {
      values[(*document).Constraint.nzJacIndices[i][j] - 1] += out[j];
    }
  }

  num_of_const_done++;

  //cout<<num_of_const_done<<endl;

  if (num_of_const_done == num_of_const)
  {
    signal_done.notify_all();
  }

}

void frost::MyMonitor::waitTillReady()
{
  std::unique_lock<std::mutex> lk(lock);

  while (num_of_const_done != num_of_const)
  {
    signal_done.wait(lk);
  }
}

const Number * frost::MyMonitor::getX()
{
  std::unique_lock<std::mutex> lk(lock);

  return x;
}

/** The class constructor function */
frost::JacGEvalMultiThread::JacGEvalMultiThread(frost::Document &document, int nThreads)
{
  int nVar = document.Variable.dimVars;
  int nConst = document.Constraint.numFuncs;
  int nOut = document.Constraint.Dimension;
  int nJOutConst = document.Constraint.nnzJac;
  int nJOutObj = document.Objective.nnzJac;

  this->n_var = nVar;
  this->n_constr = nOut;
  this->nnz_jac_g = nJOutConst;

  this->document = &document;

  this->monitor = new frost::MyMonitor(nThreads, nJOutConst, &document);
}

frost::JacGEvalMultiThread::~JacGEvalMultiThread()
{
  delete monitor;
}

bool frost::JacGEvalMultiThread::eval_jac_g(Index n, const Number* x, bool new_x,
                                     Index m, Index nele_jac, Index* iRow, Index *jCol,
                                     Number* values)
{
  int nConst = (*document).Constraint.numFuncs;

  assert(n == n_var);
  assert(m == n_constr);
  assert(nele_jac == nnz_jac_g);

  for (int i = 0; i < nnz_jac_g; i++)
  {
    values[i] = 0;
  }
  
  monitor->setValues(values);
  monitor->resetConstraints(nConst, x);
  monitor->waitTillReady();

  return true;
}
