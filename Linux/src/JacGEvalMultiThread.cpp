#include "frost/JacGEvalMultiThread.h"
#include "rapidjson/document.h"
#include "frost/functionlist.hh"
#include "IpTNLP.hpp"

#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>

using namespace Ipopt;
using namespace std;
using namespace rapidjson;


class frost::JacGEvalMultiThread_worker
{
public:
  JacGEvalMultiThread_worker(int nJOutConst, frost::MyMonitor *monitor, rapidjson::Document *document)
  {
    this->in = new Number[nJOutConst];
    this->out = new Number[nJOutConst];
    this->monitor = monitor;
    this->document = document;
  }

  ~JacGEvalMultiThread_worker()
  {
    delete []in;
    delete []out;
  }

  void Compute()
  {
    while(true)
    {
      int i = monitor->getConstraint();

      if (i < 0)
        return;

      if ((*document)["Constraint"]["nzJacIndices"][i].IsNull())
        continue;

      int fIdx = (*document)["Constraint"]["JacFuncs"][i].GetInt() - 1;
      int numDep = 0;
      if ((*document)["Constraint"]["DepIndices"][i].IsArray())
      {
        numDep = (*document)["Constraint"]["DepIndices"][i].Size();
        for (int j = 0; j < numDep; j++)
        {
          in[j] = x[(*document)["Constraint"]["DepIndices"][i][j].GetInt() - 1];
        }
      }
      else if ((*document)["Constraint"]["DepIndices"][i].IsNull() == false)
      {
        numDep = 1;
        in[0] = x[(*document)["Constraint"]["DepIndices"][i].GetInt() - 1];
      }

      int numAux = 0;
      if ((*document)["Constraint"]["AuxData"][i].IsArray())
      {
        numAux = (*document)["Constraint"]["AuxData"][i].Size();
        for (int j = 0; j < numAux; j++)
        {
          in[j + numDep] = (*document)["Constraint"]["AuxData"][i][j].GetDouble();
        }
      }
      else if ((*document)["Constraint"]["AuxData"][i].IsNull() == false)
      {
        numAux = 1;
        in[0 + numDep] = (*document)["Constraint"]["AuxData"][i].GetDouble();
      }

      frost::functions[fIdx](out, in);

      monitor->updateValues(out);
    }
  }

private:
  Number *in;  // temporary input variables
  Number *out; // temporary function outputs
  frost::MyMonitor *monitor;
  rapidjson::Document *document;
};

class frost::MyMonitor
{
public:
  MyMonitor(int numThreads, int nJOutConst, rapidjson::Document *document)
  {
    this->document = document;

    num_of_const_done = 0;
    num_of_const = 0;
    next_const_index = 0;
    done = false;

    for (int i = 0; i < numThreads; i++)
    {
      thread_obj.push_back(new frost::JacGEvalMultiThread_worker(nJOutConst, this, document));
      threads.push_back(new std::thread(&(thread_obj[i]->Compute())));
    }
  }

  ~MyMonitor()
  {
    done = true;
    queue_available.notify_all();

    for (int i = 0; i < threads.size(); i++)
    {
      threads[i]->join();
    }

    for (int i = 0; i < threads.size(); i++)
    {
      delete threads[i];
      delete thread_obj[i];
    }
  }

  void resetConstraints(int num_of_const)
  {
    std::unique_lock<std::mutex> lk(lock);

    this->num_of_const = num_of_const;
    next_const_index = 0;
    num_of_const_done = 0;

    queue_available.notify_all();
  }

  void setValues(Number* values)
  {
    std::unique_lock<std::mutex> lk(lock);
    
    this->values = values;
  }

  int getConstraint()
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

  void updateValues(Number *out)
  {
    std::unique_lock<std::mutex> lk(lock);

    int numConst = 0;
    if ((*document)["Constraint"]["nzJacIndices"][i].IsArray())
    {
      numConst = (*document)["Constraint"]["nzJacIndices"][i].Size();
      for (int j = 0; j < numConst; j++)
      {
        values[(*document)["Constraint"]["nzJacIndices"][i][j].GetInt() - 1] += out[j];
      }
    }
    else if ((*document)["Constraint"]["FuncIndices"][i].IsNull() == false)
    {
      numConst = 1;
      values[(*document)["Constraint"]["nzJacIndices"][i].GetInt() - 1] += out[0];
    }

    num_of_const_done++;

    if (num_of_const_done == num_of_const)
    {
      signal_done.notify_all();
    }
  }

  void waitTillReady()
  {
    std::unique_lock<std::mutex> lk(lock);

    while (num_of_const_done != num_of_const)
    {
      signal_done.wait(lk);
    }
  }

private:
  std::mutex lock;
  int next_const_index;
  int num_of_const;
  int num_of_const_done;
  std::condition_variable signal_done;
  std::condition_variable queue_available;
  Number* values;
  rapidjson::Document *document;
  std::vector<std::thread*> threads;
  std::vector<frost::JacGEvalMultiThread_worker*> thread_obj;
  bool done;
};

/** The class constructor function */
frost::JacGEvalMultiThread::JacGEvalMultiThread(rapidjson::Document &document)
{
  int nVar = document["Variable"]["dimVars"].GetInt();
  int nConst = document["Constraint"]["numFuncs"].GetInt();
  int nOut = document["Constraint"]["Dimension"].GetInt();
  int nJOutConst = document["Constraint"]["nnzJac"].GetInt();
  int nJOutObj = document["Objective"]["nnzJac"].GetInt();

  this->n_var = nVar;
  this->n_constr = nOut;
  this->nnz_jac_g = nJOutConst;

  this->document = &document;

  this->monitor = new frost::MyMonitor(numThreads, nJOutConst, &document);
}

frost::JacGEvalMultiThread::~JacGEvalMultiThread()
{
  delete monitor;
}

bool frost::JacGEvalMultiThread::eval_jac_g(Index n, const Number* x, bool new_x,
                                     Index m, Index nele_jac, Index* iRow, Index *jCol,
                                     Number* values)
{
  int nConst = (*document)["Constraint"]["numFuncs"].GetInt();

  assert(n == n_var);
  assert(m == n_constr);
  assert(nele_jac == nnz_jac_g);

  for (int i = 0; i < nnz_jac_g; i++)
  {
    values[i] = 0;
  }
  
  monitor->setValues(values);
  monitor->resetConstraints(n_constr);
  monitor->waitTillReady();

  return true;
}
