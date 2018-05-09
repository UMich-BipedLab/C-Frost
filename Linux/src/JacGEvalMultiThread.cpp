#include "frost/JacGEvalMultiThread.h"
#include "rapidjson/document.h"
#include "frost/functionlist.hh"
#include "IpTNLP.hpp"

using namespace Ipopt;
using namespace std;
using namespace rapidjson;

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

  this->in = new Number[nJOutConst];
  this->out = new Number[nJOutConst];

  this->document = &document;
}

frost::JacGEvalMultiThread::~JacGEvalMultiThread()
{
  delete[] in;
  delete[] out;
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

  for (int i = 0; i < nConst; i++)
  {
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
  }

  return true;
}
