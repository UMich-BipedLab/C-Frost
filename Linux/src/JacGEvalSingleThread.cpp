#include "frost/JacGEvalSingleThread.h"
#include "frost/Document.h"
#include "frost/functionlist.hh"
#include "IpTNLP.hpp"

#include <cassert>

using namespace Ipopt;

/** The class constructor function */
frost::JacGEvalSingleThread::JacGEvalSingleThread(frost::Document &document)
{
  int nVar = document.Variable.dimVars;
  int nConst = document.Constraint.numFuncs;
  int nOut = document.Constraint.Dimension;
  int nJOutConst = document.Constraint.nnzJac;
  int nJOutObj = document.Objective.nnzJac;

  this->n_var = nVar;
  this->n_constr = nOut;
  this->nnz_jac_g = nJOutConst;

  this->in = new Number[nJOutConst];
  this->out = new Number[nJOutConst];

  this->document = &document;
}

frost::JacGEvalSingleThread::~JacGEvalSingleThread()
{
  delete[] in;
  delete[] out;
}

bool frost::JacGEvalSingleThread::eval_jac_g(Index n, const Number* x, bool new_x,
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

  for (int i = 0; i < nConst; i++)
  {
    int fIdx = (*document).Constraint.JacFuncs[i] - 1;
    int numDep = (*document).Constraint.DepIndices[i].size();
    for (int j = 0; j < numDep; j++)
    {
      in[j] = x[(*document).Constraint.DepIndices[i][j] - 1];
    }

    int numAux = (*document).Constraint.AuxData[i].size();
    for (int j = 0; j < numAux; j++)
    {
      in[j + numDep] = (*document).Constraint.AuxData[i][j];
    }

    frost::functions[fIdx](out, in);

    int numConst = 0;
    numConst = (*document).Constraint.nzJacIndices[i].size();
    for (int j = 0; j < numConst; j++)
    {
      values[(*document).Constraint.nzJacIndices[i][j] - 1] += out[j];
    }
  }

  return true;
}
