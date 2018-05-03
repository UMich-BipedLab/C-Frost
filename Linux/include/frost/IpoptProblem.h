#ifndef IPOPTPROBLEM_H
#define IPOPTPROBLEM_H

#include "rapidjson/document.h"
#include "IpTNLP.hpp"

using namespace Ipopt;

namespace frost {
  class FROST_SOLVER : public TNLP {
 public:
    FROST_SOLVER(rapidjson::Document &document, const double *x0);
    virtual ~FROST_SOLVER();

    virtual bool get_nlp_info(Index& n, Index& m, Index& nnz_jac_g,
                              Index& nnz_h_lag, IndexStyleEnum& index_style);

    virtual bool get_bounds_info(Index n, Number* x_l, Number* x_u,
                                 Index m, Number* g_l, Number* g_u);

    virtual bool get_starting_point(Index n, bool init_x, Number* x,
                                    bool init_z, Number* z_L, Number* z_U,
                                    Index m, bool init_lambda,
                                    Number* lambda);

    virtual bool eval_f(Index n, const Number* x, bool new_x, Number& obj_value);

    virtual bool eval_grad_f(Index n, const Number* x, bool new_x, Number* grad_f);

    virtual bool eval_g(Index n, const Number* x, bool new_x, Index m, Number* g);

    virtual bool eval_jac_g(Index n, const Number* x, bool new_x,
                            Index m, Index nele_jac, Index* iRow, Index *jCol,
                            Number* values);

    virtual bool eval_h(Index n, const Number* x, bool new_x,
                        Number obj_factor, Index m, const Number* lambda,
                        bool new_lambda, Index nele_hess, Index* iRow,
                        Index* jCol, Number* values);

    virtual void finalize_solution(SolverReturn status,
                                   Index n, const Number* x, const Number* z_L, const Number* z_U,
                                   Index m, const Number* g, const Number* lambda,
                                   Number obj_value,
                                   const IpoptData* ip_data,
                                   IpoptCalculatedQuantities* ip_cq);


    virtual bool set_var_bound(Index idx, Number x_l, Number x_u);
    virtual bool set_constr_bound(Index idx, Number g_l, Number g_u);





 public:
    rapidjson::Document *document;


 private:
    Number *in;   // temporary input variables
    Number *out;  // temporary function outputs
    Number *x0;   // the starting point (initial guess) for the NLP

    Number *x_l;  // the lower bound of the variables
    Number *x_u;  // the upper bound of the variables
    Number *g_l;  // the lower bound of the constraints
    Number *g_u;  // the upper bound of the constraints

 public:
    Index n_var; // the number of optimization variables
    Index n_constr; // the number of constraints
    Index nnz_jac_g; // the number of non-zeros in constraint Jacobian

 public:
    SolverReturn status; // the NLP status
    Number *x_opt;    // the optimal solution
    Number *z_L;      // the lower bound of z
    Number *z_U;      // the upper bound of z
    Number *lambda;   // the Lagrangian multiplier lambda
    Number obj_value; // the objective function value
    Number *g;        // the constraints evaluation

  };
}

#endif
