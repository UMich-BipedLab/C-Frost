#include "frost/Document.h"
#include "frost/functionlist.hh"
#include "frost/IpoptProblem.h"
#include "IpTNLP.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstring>
#include <cassert>

using namespace Ipopt;
using namespace std;

/** The class constructor function */
frost::FROST_SOLVER::FROST_SOLVER(frost::Document &document, const double *x0, frost::JacGEvalAbstract *jacGEval)
{
  int nVar = document.Variable.dimVars;
  int nConst = document.Constraint.numFuncs;
  int nOut = document.Constraint.Dimension;
  int nJOutConst = document.Constraint.nnzJac;
  int nJOutObj = document.Objective.nnzJac;

  this->in = new Number[nJOutConst];
  this->out = new Number[nJOutConst];
  this->x0 = new Number[nVar];

  for (int i = 0; i < nVar; i++)
  {
    this->x0[i] = x0[i];
  }

  this->document = &document;
  this->jacGEval = jacGEval;

  this->n_var = nVar;
  this->n_constr = nOut;
  this->nnz_jac_g = nJOutConst;

  this->x_l = new Number[nVar];
  this->x_u = new Number[nVar];
  for (int i = 0; i < nVar; i++)
  {
    this->x_l[i] = document.Variable.lb[i];
    this->x_u[i] = document.Variable.ub[i];
  }

  this->g_l = new Number[nOut];
  this->g_u = new Number[nOut];
  for (int i = 0; i < nOut; i++)
  {
    this->g_l[i] = document.Constraint.LowerBound[i];
    this->g_u[i] = document.Constraint.UpperBound[i];
  }

  // outputs
  this->x_opt = new Number[nVar];
  this->z_L   = new Number[nVar];
  this->z_U   = new Number[nVar];
  this->lambda = new Number[nOut];
  this->g      = new Number[nOut];
}

frost::FROST_SOLVER::~FROST_SOLVER()
{
  delete[] in;
  delete[] out;
  delete[] x0;
  delete[] x_l;
  delete[] x_u;
  delete[] g_l;
  delete[] g_u;

  delete[] x_opt;
  delete[] z_L;
  delete[] z_U;
  delete[] lambda;
  delete[] g;

  delete jacGEval;
}

bool frost::FROST_SOLVER::get_nlp_info(Index &n, Index &m, Index &nnz_jac_g,
                                       Index &nnz_h_lag, IndexStyleEnum &index_style)
{
  n = this->n_var;

  m = this->n_constr;

  nnz_jac_g = this->nnz_jac_g;

  index_style = TNLP::C_STYLE;

  return true;
}

bool frost::FROST_SOLVER::set_var_bound(Index idx, Number x_l, Number x_u)
{
  this->x_l[idx] = x_l;
  this->x_u[idx] = x_u;

  return true;
}

bool frost::FROST_SOLVER::set_constr_bound(Index idx, Number g_l, Number g_u)
{
  this->g_l[idx] = g_l;
  this->g_u[idx] = g_u;

  return true;
}

// returns the variable bounds
bool frost::FROST_SOLVER::get_bounds_info(Index n, Number *x_l, Number *x_u,
                                          Index m, Number *g_l, Number *g_u)
{

  assert(n == n_var);
  assert(m == n_constr);

  memcpy(x_l, this->x_l, sizeof(Number)*n_var);
  memcpy(x_u, this->x_u, sizeof(Number)*n_var);

  memcpy(g_l, this->g_l, sizeof(Number)*n_constr);
  memcpy(g_u, this->g_u, sizeof(Number)*n_constr);
  return true;
}

// returns the initial point for the problem
bool frost::FROST_SOLVER::get_starting_point(Index n, bool init_x, Number *x,
                                             bool init_z, Number *z_L, Number *z_U,
                                             Index m, bool init_lambda,
                                             Number *lambda)
{
  // Here, we assume we only have starting values for x, if you code
  // your own NLP, you can provide starting values for the dual variables
  // if you wish
  assert(init_x == true);
  assert(init_z == false);
  assert(init_lambda == false);

  memcpy(x, this->x0, sizeof(Number)*n_var);

  return true;
}



void frost::FROST_SOLVER::finalize_solution(SolverReturn status,
                                            Index n, const Number* x, const Number* z_L, const Number* z_U,
                                            Index m, const Number* g, const Number* lambda,
                                            Number obj_value,
                                            const IpoptData* ip_data,
                                            IpoptCalculatedQuantities* ip_cq)
{
  // here is where we would store the solution to variables, or write to a file, etc
  // so we could use the solution.
  assert(n == n_var);
  assert(m == n_constr);

  this->status = status;
  memcpy(this->x_opt, x, sizeof(Number)*n_var);
  memcpy(this->z_L, z_L, sizeof(Number)*n_var);
  memcpy(this->z_U, z_U, sizeof(Number)*n_var);

  memcpy(this->lambda, lambda, sizeof(Number)*n_constr);
  memcpy(this->g, g, sizeof(Number)*n_constr);

  this->obj_value = obj_value;

  std::cout << std::endl << std::endl << "Objective value" << std::endl;
  std::cout << "f(x*) = " << obj_value << std::endl;

}


bool frost::FROST_SOLVER::eval_g(Index n, const Number* x, bool new_x, Index m, Number* g)
{

  int nConst = (*document).Constraint.numFuncs;

  assert(n == n_var);
  assert(m == n_constr);

  for (int i = 0; i < n_constr; i++)
    g[i] = 0;

  for (int i = 0; i < nConst; i++)
  {
    int fIdx = (*document).Constraint.Funcs[i] - 1;
    unsigned long int numDep = (*document).Constraint.DepIndices[i].size();
    for (unsigned long int j = 0; j < numDep; j++)
    {
      in[j] = x[(*document).Constraint.DepIndices[i][j] - 1];
    }

    unsigned long int numAux = (*document).Constraint.AuxData[i].size();
    for (unsigned long int j = 0; j < numAux; j++)
    {
      in[j + numDep] = (*document).Constraint.AuxData[i][j];
    }

    frost::functions[fIdx](out, in);

    unsigned long int numConst = (*document).Constraint.FuncIndices[i].size();
    for (unsigned long int j = 0; j < numConst; j++)
    {
      g[(*document).Constraint.FuncIndices[i][j] - 1] += out[j];
    }
  }

  return true;
}

bool frost::FROST_SOLVER::eval_f(Index n, const Number* x, bool new_x, Number& obj_value)
{
  int nConst = (*document).Objective.numFuncs;

  assert(n == n_var);

  obj_value = 0;

  for (int i = 0; i < nConst; i++)
  {
    int fIdx = (*document).Objective.Funcs[i] - 1;
    unsigned long int numDep = 0;
    numDep = (*document).Objective.DepIndices[i].size();
    for (unsigned long int j = 0; j < numDep; j++)
    {
      in[j] = x[(*document).Objective.DepIndices[i][j] - 1];
    }

    unsigned long int numAux = (*document).Objective.AuxData[i].size();
    for (unsigned long int j = 0; j < numAux; j++)
    {
      in[j + numDep] = (*document).Objective.AuxData[i][j];
    }

    frost::functions[fIdx](out, in);

    unsigned long int numConst = 0;
    numConst = (*document).Objective.FuncIndices[i].size();
    for (unsigned long int j = 0; j < numConst; j++)
    {
      obj_value += out[j];
    }
  }


  return true;
}

bool frost::FROST_SOLVER::eval_jac_g(Index n, const Number* x, bool new_x,
                                     Index m, Index nele_jac, Index* iRow, Index *jCol,
                                     Number* values)
{
  if (values == NULL)
  {
    for (int i = 0; i < nnz_jac_g; i++)
    {
      iRow[i] = (*document).Constraint.nzJacRows[i] - 1;
      jCol[i] = (*document).Constraint.nzJacCols[i] - 1;
    }

    return true;
  }
  else
  {
    return jacGEval->eval_jac_g(n, x, new_x, m, nele_jac, iRow, jCol, values);
  }
}

bool frost::FROST_SOLVER::eval_grad_f(Index n, const Number* x, bool new_x, Number* grad_f)
{
  int n_var = (*document).Variable.dimVars;
  int nConst = (*document).Objective.numFuncs;
  int nJOut = (*document).Objective.nnzJac;

  assert(n == n_var);

  for (int i = 0; i < n_var; i++)
  {
    grad_f[i] = 0;
  }

  for (int i = 0; i < nConst; i++)
  {
    int fIdx = (*document).Objective.JacFuncs[i] - 1;
    unsigned long int numDep = 0;
    numDep = (*document).Objective.DepIndices[i].size();
    for (unsigned long int j = 0; j < numDep; j++)
    {
      in[j] = x[(*document).Objective.DepIndices[i][j] - 1];
    }

    unsigned long int numAux = (*document).Objective.AuxData[i].size();
    for (unsigned long int j = 0; j < numAux; j++)
    {
      in[j + numDep] = (*document).Objective.AuxData[i][j];
    }

    frost::functions[fIdx](out, in);

    unsigned long int numConst = (*document).Objective.nzJacIndices[i].size();
    for (unsigned long int j = 0; j < numConst; j++)
    {
      int flIdx = (*document).Objective.nzJacIndices[i][j] - 1;
      int index = (*document).Objective.nzJacCols[flIdx] - 1;
      grad_f[index] += out[j];
    }
  }


  return true;
}


//return the structure or values of the hessian
bool frost::FROST_SOLVER::eval_h(Index n, const Number* x, bool new_x,
                                 Number obj_factor, Index m, const Number* lambda,
                                 bool new_lambda, Index nele_hess, Index* iRow,
                                 Index* jCol, Number* values)
{
  return true;
}
