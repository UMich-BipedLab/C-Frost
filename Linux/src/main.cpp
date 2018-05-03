#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include <iostream>
#include <fstream>
#include <string>
#include "frost/IpoptProblem.h"
#include "frost/functionlist.hh"
#include <cmath>

#include "IpIpoptApplication.hpp"

#define PI 3.141592653589793

using namespace std;
using namespace rapidjson;

void getDocument(rapidjson::Document &document, const std::string &fileName);
void exportSolution(const frost::FROST_SOLVER *solver, const std::string &fileName);


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
  assert(init_document.IsArray());
  double *x0 = new double[init_document.Size()];

  for (unsigned int i=0; i<init_document.Size(); i++)
    x0[i] = init_document[i].GetDouble();

  // Create a new instance of your nlp
  //  (use a SmartPtr, not raw)
  frost::FROST_SOLVER* frost_nlp  = new  frost::FROST_SOLVER(document, x0);
  SmartPtr<TNLP> nlp = frost_nlp;
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
  // app->Options()->SetNumericValue("tol", 1e-7);
  // app->Options()->SetStringValue("mu_strategy", "adaptive");
  // app->Options()->SetStringValue("output_file", "ipopt.out");
  // app->Options()->SetStringValue("linear_solver", "ma57");
  // app->Options()->SetStringValue("hessian_approximation", "limited-memory");
  // app->Options()->SetStringValue("limited_memory_update_type", "bfgs");
  // The following overwrites the default name (ipopt.opt) of the
  // options file
  app->Options()->SetStringValue("option_file_name", "ipopt.opt");

  // Initialize the IpoptApplication and process the options
  ApplicationReturnStatus status;
  status = app->Initialize();
  if (status != Solve_Succeeded) {
    std::cout << std::endl << std::endl << "*** Error during initialization!" << std::endl;
    return (int) status;
  }


  double r = 0.33;
  double theta[16];
  for (int i=0; i<16; i++)
  {
    theta[i] = i*PI/8;
    cout << theta[i] << endl;
  }

  double px, vx, py, vy;
  double x_pos_range = 0.1;
  double x_vel_range = 0.5;
  double y_pos_range = 0.05;
  double y_vel_range = 0.2;



  for (int i = 0; i < 1; i++)
  {
    for (int j = 0; j < 16; j++)
    {
      px = r * cos(theta[j]) * x_pos_range/2 - 0.09;
      vx = r * sin(theta[j]) * x_vel_range/2;
      py = r * cos(theta[i]) * y_pos_range/2;
      vy = r * sin(theta[i]) * y_vel_range/2;

      frost_nlp->set_constr_bound(100, px, px);// x-com position
      frost_nlp->set_constr_bound(101, py, py);  // y-com position
      frost_nlp->set_constr_bound(103, vx, vx);    // x-com velocity
      frost_nlp->set_constr_bound(104, vy, vy);    // x-com velocity


      // cout << fname << endl;
      // // Ask Ipopt to solve the problem
      status = app->OptimizeTNLP(nlp);

      char fname [100];

      if (status == Solve_Succeeded || status == Solved_To_Acceptable_Level
          || status == Feasible_Point_Found || status == Restoration_Failed)
      {
        std::cout << std::endl << std::endl << "*** The problem solved!" << std::endl;
        sprintf(fname, "export/standing_push_px%.3f_py%.3f_vx%0.3f_vy%0.3f.json", px, py, vx, vy);
      }
      else {
        std::cout << std::endl << std::endl << "*** The problem FAILED!" << std::endl;
        sprintf(fname, "export/standing_push_px%.3f_py%.3f_vx%0.3f_vy%0.3f_failed.json", px, py, vx, vy);
      }

      exportSolution(frost_nlp, fname);
    }
  }








  // As the SmartPtrs go out of scope, the reference count
  // will be decremented and the objects will automatically
  // be deleted.

  return (int) status;

}

void getDocument(rapidjson::Document &document, const std::string &fileName)
{
  ifstream in(fileName.c_str());
  std::string json((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
  in.close();

  document.Parse(json.c_str());
}

void exportSolution(const frost::FROST_SOLVER *solver, const std::string &fileName)
{
  // Create a JSON object to fill out the results
  Document doc;
  doc.SetObject();

  Document::AllocatorType& allocator = doc.GetAllocator();

  doc.AddMember("status", solver->status, allocator);
  doc.AddMember("objective", solver->obj_value, allocator);

  Value x(kArrayType);
  Value zl(kArrayType);
  Value zu(kArrayType);
  for (int i=0; i < solver->n_var; i++)
  {
    x.PushBack(solver->x_opt[i], allocator);
    zl.PushBack(solver->z_L[i], allocator);
    zu.PushBack(solver->z_U[i], allocator);
  }

  Value g(kArrayType);
  Value lambda(kArrayType);
  for (int i=0; i< solver->n_constr; i++)
  {
    g.PushBack(solver->g[i], allocator);
    lambda.PushBack(solver->lambda[i], allocator);
  }

  doc.AddMember("x", x, allocator);
  doc.AddMember("zl", zl, allocator);
  doc.AddMember("zu", zu, allocator);
  doc.AddMember("constr", g, allocator);
  doc.AddMember("lambda", lambda, allocator);


  // write to a buffer
  StringBuffer buf;
  Writer<StringBuffer> writer(buf);
  doc.Accept(writer);
  std::string json(buf.GetString(), buf.GetSize());


  // write to a json file
  ofstream out(fileName.c_str());
  out << json;
  if (!out.good())
  {
    throw std::runtime_error ("Can't write the JSON string to the file!");
  }

}
