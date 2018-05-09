#include "rapidjson/document.h"
#include "frost/functionlist.hh"
#include "frost/IpoptProblem.h"
#include "IpTNLP.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>

using namespace Ipopt;
using namespace std;
using namespace rapidjson;

/** The class constructor function */
frost::FROST_SOLVER::FROST_SOLVER(rapidjson::Document &document, const double *x0, frost::JacGEvalAbstract *jacGEval)
{
  int nVar = document["Variable"]["dimVars"].GetInt();
  int nConst = document["Constraint"]["numFuncs"].GetInt();
  int nOut = document["Constraint"]["Dimension"].GetInt();
  int nJOutConst = document["Constraint"]["nnzJac"].GetInt();
  int nJOutObj = document["Objective"]["nnzJac"].GetInt();

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
    this->x_l[i] = document["Variable"]["lb"][i].GetDouble();
    this->x_u[i] = document["Variable"]["ub"][i].GetDouble();
  }

  this->g_l = new Number[nOut];
  this->g_u = new Number[nOut];
  for (int i = 0; i < nOut; i++)
  {
    this->g_l[i] = document["Constraint"]["LowerBound"][i].GetDouble();
    this->g_u[i] = document["Constraint"]["UpperBound"][i].GetDouble();
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

  int nConst = (*document)["Constraint"]["numFuncs"].GetInt();

  assert(n == n_var);
  assert(m == n_constr);

  for (int i = 0; i < n_constr; i++)
    g[i] = 0;

  for (int i = 0; i < nConst; i++)
  {
    int fIdx = (*document)["Constraint"]["Funcs"][i].GetInt() - 1;
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
    if ((*document)["Constraint"]["FuncIndices"][i].IsArray())
    {
      numConst = (*document)["Constraint"]["FuncIndices"][i].Size();
      for (int j = 0; j < numConst; j++)
      {
        g[(*document)["Constraint"]["FuncIndices"][i][j].GetInt() - 1] += out[j];
      }
    }
    else if ((*document)["Constraint"]["FuncIndices"][i].IsNull() == false)
    {
      numConst = 1;
      g[(*document)["Constraint"]["FuncIndices"][i].GetInt() - 1] += out[0];
    }
  }

  return true;
}

bool frost::FROST_SOLVER::eval_f(Index n, const Number* x, bool new_x, Number& obj_value)
{
  int nConst = (*document)["Objective"]["numFuncs"].GetInt();

  assert(n == n_var);

  obj_value = 0;

  for (int i = 0; i < nConst; i++)
  {
    int fIdx = (*document)["Objective"]["Funcs"][i].GetInt() - 1;
    int numDep = 0;
    if ((*document)["Objective"]["DepIndices"][i].IsArray())
    {
      numDep = (*document)["Objective"]["DepIndices"][i].Size();
      for (int j = 0; j < numDep; j++)
      {
        in[j] = x[(*document)["Objective"]["DepIndices"][i][j].GetInt() - 1];
      }
    }
    else if ((*document)["Objective"]["DepIndices"][i].IsNull() == false)
    {
      numDep = 1;
      in[0] = x[(*document)["Objective"]["DepIndices"][i].GetInt() - 1];
    }

    int numAux = 0;
    if ((*document)["Objective"]["AuxData"][i].IsArray())
    {
      numAux = (*document)["Objective"]["AuxData"][i].Size();
      for (int j = 0; j < numAux; j++)
      {
        in[j + numDep] = (*document)["Objective"]["AuxData"][i][j].GetDouble();
      }
    }
    else if ((*document)["Objective"]["AuxData"][i].IsNull() == false)
    {
      numAux = 1;
      in[0 + numDep] = (*document)["Objective"]["AuxData"][i].GetDouble();
    }

    frost::functions[fIdx](out, in);

    int numConst = 0;
    if ((*document)["Objective"]["FuncIndices"][i].IsArray())
    {
      numConst = (*document)["Objective"]["FuncIndices"][i].Size();
      for (int j = 0; j < numConst; j++)
      {
        obj_value += out[j];
      }
    }
    else if ((*document)["Objective"]["FuncIndices"][i].IsNull() == false)
    {
      numConst = 1;
      obj_value += out[0];
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
      iRow[i] = (*document)["Constraint"]["nzJacRows"][i].GetInt() - 1;
      jCol[i] = (*document)["Constraint"]["nzJacCols"][i].GetInt() - 1;
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
  int n_var = (*document)["Variable"]["dimVars"].GetInt();
  int nConst = (*document)["Objective"]["numFuncs"].GetInt();
  int nJOut = (*document)["Objective"]["nnzJac"].GetInt();

  assert(n == n_var);

  for (int i = 0; i < n_var; i++)
  {
    grad_f[i] = 0;
  }

  for (int i = 0; i < nConst; i++)
  {
    if ((*document)["Objective"]["nzJacIndices"][i].IsNull())
      continue;

    int fIdx = (*document)["Objective"]["JacFuncs"][i].GetInt() - 1;
    int numDep = 0;
    if ((*document)["Objective"]["DepIndices"][i].IsArray())
    {
      numDep = (*document)["Objective"]["DepIndices"][i].Size();
      for (int j = 0; j < numDep; j++)
      {
        in[j] = x[(*document)["Objective"]["DepIndices"][i][j].GetInt() - 1];
      }
    }
    else if ((*document)["Objective"]["DepIndices"][i].IsNull() == false)
    {
      numDep = 1;
      in[0] = x[(*document)["Objective"]["DepIndices"][i].GetInt() - 1];
    }

    int numAux = 0;
    if ((*document)["Objective"]["AuxData"][i].IsArray())
    {
      numAux = (*document)["Objective"]["AuxData"][i].Size();
      for (int j = 0; j < numAux; j++)
      {
        in[j + numDep] = (*document)["Objective"]["AuxData"][i][j].GetDouble();
      }
    }
    else if ((*document)["Objective"]["AuxData"][i].IsNull() == false)
    {
      numAux = 1;
      in[0 + numDep] = (*document)["Objective"]["AuxData"][i].GetDouble();
    }

    frost::functions[fIdx](out, in);

    int numConst = 0;
    if ((*document)["Objective"]["nzJacIndices"][i].IsArray())
    {
      numConst = (*document)["Objective"]["nzJacIndices"][i].Size();
      for (int j = 0; j < numConst; j++)
      {
        int flIdx = (*document)["Objective"]["nzJacIndices"][i][j].GetInt() - 1;
        int index = (*document)["Objective"]["nzJacCols"][flIdx].GetInt() - 1;
        grad_f[index] += out[j];

      }
    }
    else if ((*document)["Objective"]["FuncIndices"][i].IsNull() == false)
    {
      numConst = 1;
      int flIdx = (*document)["Objective"]["nzJacIndices"][i].GetInt() - 1;
      int index = (*document)["Objective"]["nzJacCols"][flIdx].GetInt() - 1;
      grad_f[index] += out[0];
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
