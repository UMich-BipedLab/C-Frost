#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "cxxopts.hpp" // Found at https://github.com/jarro2783/cxxopts
#include "IpIpoptApplication.hpp"

#include <iostream>
#include <fstream>
#include <string>

#include "frost/IpoptProblem.h"
#include "frost/JacGEvalSingleThread.h"
#include "frost/JacGEvalMultiThread.h"
#include "frost/functionlist.hh"

using namespace std;
using namespace rapidjson;

void getDocument(rapidjson::Document &document, const std::string &fileName);
void exportOutput(const frost::FROST_SOLVER *solver, const std::string &fileName);
void exportSolution(const frost::FROST_SOLVER *solver, const std::string &fileName);


int main(int argc, const char* argv[])
{
  cxxopts::Options options("FROST optimization", "The FROST optimization based on Ipopt");
  options.add_options()
  ("options", "Ipopt options file", cxxopts::value<std::string>())
  ("initial", "Initial condition file (json)", cxxopts::value<std::string>())
  ("data", "Data file (json)", cxxopts::value<std::string>())
  ("output", "Output file (json)", cxxopts::value<std::string>())
  ("solution", "Solution output file (json)", cxxopts::value<std::string>())
  ("threads", "Number of threads (default=1)", cxxopts::value<int>()->default_value("1"));
  
  auto param = options.parse(argc, argv);
  
  rapidjson::Document document;
  getDocument(document, param["data"].as<std::string>());

  assert(document.IsObject());
  assert(document.HasMember("Constraint"));
  assert(document.HasMember("Objective"));
  assert(document.HasMember("Variable"));
  assert(document.HasMember("Options"));

  rapidjson::Document init_document;
  getDocument(init_document, param["initial"].as<std::string>());
  assert(init_document.IsArray());
  double *x0 = new double[init_document.Size()];

  for (unsigned int i=0; i<init_document.Size(); i++)
    x0[i] = init_document[i].GetDouble();

  // Create a new instance of your nlp
  //  (use a SmartPtr, not raw)
  frost::FROST_SOLVER* frost_nlp;
  if (param["threads"].as<int>() == 1)
    frost_nlp = new  frost::FROST_SOLVER(document, x0, new frost::JacGEvalSingleThread(document));
  else
    frost_nlp = new  frost::FROST_SOLVER(document, x0, new frost::JacGEvalMultiThread(document, param["threads"].as<int>()));
  
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
  app->Options()->SetStringValue("option_file_name", param["options"].as<std::string>());

  // Initialize the IpoptApplication and process the options
  ApplicationReturnStatus status;
  status = app->Initialize();
  if (status != Solve_Succeeded) {
    std::cout << std::endl << std::endl << "*** Error during initialization!" << std::endl;
    return (int) status;
  }

  // cout << fname << endl;
  // // Ask Ipopt to solve the problem
  status = app->OptimizeTNLP(nlp);
  
  if (status == Solve_Succeeded || status == Solved_To_Acceptable_Level
    || status == Feasible_Point_Found || status == Restoration_Failed)
  {
    std::cout << std::endl << std::endl << "*** The problem solved!" << std::endl;
  }
  else {
    std::cout << std::endl << std::endl << "*** The problem FAILED!" << std::endl;
  }

  if (param.count("output") >= 1)
  {
    exportOutput(frost_nlp, param["output"].as<std::string>());
  }

  if (param.count("solution") >= 1)
  {
    exportSolution(frost_nlp, param["solution"].as<std::string>());
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

void exportOutput(const frost::FROST_SOLVER *solver, const std::string &fileName)
{
  // Create a JSON object to fill out the results
  Document doc;
  Document::AllocatorType& allocator = doc.GetAllocator();

  doc.SetArray();
  for (int i=0; i < solver->n_var; i++)
  {
    doc.PushBack(solver->x_opt[i], allocator);
  }

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
