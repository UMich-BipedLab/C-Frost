#include "rapidjson/document.h"
#include <iostream>
#include <fstream>
#include <string>
#include "frost/IpoptProblem.h"
#include "frost/functionlist.hh"
#include <cmath>

#include "IpIpoptApplication.hpp"

using namespace std;

void getDocument(rapidjson::Document &document, const std::string &fileName);

void testIpoptConstraint(frost::FROST_SOLVER &solver);
void testIpoptOptimization(frost::FROST_SOLVER &solver);
void testIpoptJacobian(frost::FROST_SOLVER &solver);
//void testIpoptGradient(frost::FROST_SOLVER &solver);

int main()
{
  rapidjson::Document document;
  getDocument(document, "res/data.json");

  assert(document.IsObject());
  assert(document.HasMember("Constraint"));
  assert(document.HasMember("Objective"));
  assert(document.HasMember("Variable"));
  assert(document.HasMember("Options"));

  rapidjson::Document init_document;
  getDocument(init_document, "res/init.json");
  assert(document.IsArray());
  double *x0 = new [document.Size()];

  // Create a new instance of your nlp
  //  (use a SmartPtr, not raw)
  //frost::FROST_SOLVER solver(document);

  // Create a new instance of your nlp
  //  (use a SmartPtr, not raw)
  SmartPtr<TNLP> mynlp = new  frost::FROST_SOLVER(document, x0);
  delete []x0;

  // Create a new instance of IpoptApplication
  //  (use a SmartPtr, not raw)
  // We are using the factory, since this allows us to compile this
  // example with an Ipopt Windows DLL
  SmartPtr<IpoptApplication> app = IpoptApplicationFactory();
  app->RethrowNonIpoptException(true);

  // Change some options
  // Note: The following choices are only examples, they might not be
  //       suitable for your optimization problem.
  app->Options()->SetNumericValue("tol", 1e-7);
  app->Options()->SetNumericValue("max_iter", 1000);
  app->Options()->SetStringValue("mu_strategy", "adaptive");
  app->Options()->SetStringValue("output_file", "ipopt.out");
  app->Options()->SetStringValue("linear_solver", "ma57");
  app->Options()->SetStringValue("hessian_approximation", "limited-memory");
  app->Options()->SetStringValue("limited_memory_update_type", "bfgs");
  // The following overwrites the default name (ipopt.opt) of the
  // options file
  // app->Options()->SetStringValue("option_file_name", "hs071.opt");

  // Initialize the IpoptApplication and process the options
  ApplicationReturnStatus status;
  status = app->Initialize();
  if (status != Solve_Succeeded) {
    std::cout << std::endl << std::endl << "*** Error during initialization!" << std::endl;
    return (int) status;
  }

  // Ask Ipopt to solve the problem
  status = app->OptimizeTNLP(mynlp);

  if (status == Solve_Succeeded) {
    std::cout << std::endl << std::endl << "*** The problem solved!" << std::endl;
  }
  else {
    std::cout << std::endl << std::endl << "*** The problem FAILED!" << std::endl;
  }

  // As the SmartPtrs go out of scope, the reference count
  // will be decremented and the objects will automatically
  // be deleted.

  return (int) status;

  //testIpoptConstraint(solver);
  //testIpoptOptimization(solver);
  //testIpoptJacobian(solver);
  //testIpoptGradient(solver);

  return 0;
}

void getDocument(rapidjson::Document &document, const std::string &fileName)
{
  ifstream in(fileName.c_str());
  std::string json((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
  in.close();

  document.Parse(json.c_str());
}

void testIpoptConstraint(frost::FROST_SOLVER &solver)
{
  rapidjson::Document testConst;
  getDocument(testConst, "res/ipopt_constraint_test.json");

  rapidjson::Document *document = solver.document;

  int nVar = (*document)["Variable"]["dimVars"].GetInt();
  int nConst = (*document)["Constraint"]["numFuncs"].GetInt();
  int nOut = (*document)["Constraint"]["Dimension"].GetInt();

  double c[2000];
  double x[2000];

  for (int i = 0; i < nVar; i++)
    {
      x[i] = testConst["x"][i].GetDouble();
    }

  solver.eval_g(nVar, x, false, nOut, c);

  double error = 0;
  for (int i = 0; i < nConst; i++)
    {
      error += pow(c[i] - testConst["C"][i].GetDouble(), 2);
    }

  cout << error << endl;
}

void testIpoptOptimization(frost::FROST_SOLVER &solver)
{
  rapidjson::Document testConst;
  getDocument(testConst, "res/ipopt_constraint_test.json");

  rapidjson::Document *document = solver.document;

  int nVar = (*document)["Variable"]["dimVars"].GetInt();


  double o;
  double x[2000];

  for (int i = 0; i < nVar; i++)
    {
      x[i] = testConst["x"][i].GetDouble();
    }

  solver.eval_f(nVar, x, false, o);

  cout << o << endl;
}

void testIpoptJacobian(frost::FROST_SOLVER &solver)
{
  rapidjson::Document testConst;
  getDocument(testConst, "res/ipopt_jacobian_test.json");

  rapidjson::Document *document = solver.document;

  int nVar = (*document)["Variable"]["dimVars"].GetInt();
  int nConst = (*document)["Constraint"]["numFuncs"].GetInt();
  int nOut = (*document)["Constraint"]["Dimension"].GetInt();
  int nJOut = (*document)["Constraint"]["nnzJac"].GetInt();

  double x[2000];
  double *Jval;

  Jval = new double[nJOut];

  for (int i = 0; i < nVar; i++)
    {
      x[i] = testConst["x"][i].GetDouble();
    }

  solver.eval_jac_g(nVar, x, false, nOut, nJOut, NULL, NULL, Jval);


  double errorVal = 0;
  for (int i = 0; i < nJOut; i++)
    {
      errorVal += pow(Jval[i] - testConst["J_val"][i].GetDouble(), 2);
    }

  cout << errorVal << endl;

  delete[] Jval;
}


// void testIpoptGradient(frost::FROST_SOLVER &solver)
// {
//   rapidjson::Document testConst;
//   getDocument(testConst, "res/ipopt_gradient_test.json");

//   int nVar = document["Variable"]["dimVars"].GetInt();
//   int nJOut = document["Objective"]["nnzJac"].GetInt();

//   double x[2000];
//   double *Jgrad;

//   Jgrad = new double[nJOut];

//   for (int i = 0; i < nVar; i++)
//     {
//       x[i] = testConst["x"][i].GetDouble();
//     }

//   frost::IpoptGradient(Jgrad, x, document);

//   double errorVal = 0;
//   for (int i = 0; i < nJOut; i++)
//     {
//       errorVal += pow(Jgrad[i] - testConst["J_val"][i].GetDouble(), 2);
//     }

//   cout << errorVal << endl;

//   delete[] Jgrad;
// }
