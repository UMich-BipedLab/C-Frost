#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "cxxopts.hpp" // Found at https://github.com/jarro2783/cxxopts
#include "IpIpoptApplication.hpp"

#include <iostream>
#include <fstream>
#include <string>

#include "frost/Document.h"
#include "frost/IpoptProblem.h"
#include "frost/JacGEvalSingleThread.h"
#include "frost/JacGEvalMultiThread.h"
#include "frost/functionlist.hh"

using namespace std;
using namespace rapidjson;

void getDocument(rapidjson::Document &document, const std::string &fileName);
void updateData(rapidjson::Document &rapidDocument, frost::Document &document);
void updateBounds(rapidjson::Document &rapidDocument, frost::Document &document);
void copyFrostDocumentFromRapidJsonDocument(rapidjson::Document &rapidDocument, frost::Document &document);
void exportOutput(const frost::FROST_SOLVER *solver, const std::string &fileName);
void exportSolution(const frost::FROST_SOLVER *solver, const std::string &fileName);

int main(int argc, const char* argv[])
{
  cxxopts::Options options("FROST optimization", "The FROST optimization based on Ipopt");
  options.add_options()
  ("options", "Ipopt options file", cxxopts::value<std::string>())
  ("initial", "Initial condition file (json)", cxxopts::value<std::string>())
  ("data", "Data file (json)", cxxopts::value<std::string>())
  ("bounds", "Bounds file (json)", cxxopts::value<std::string>())
  ("output", "Output file (json)", cxxopts::value<std::string>())
  ("solution", "Solution output file (json)", cxxopts::value<std::string>())
  ("threads", "Number of threads (default=1)", cxxopts::value<int>()->default_value("1"));
  
  auto param = options.parse(argc, argv);
  
  rapidjson::Document document;
  getDocument(document, param["data"].as<std::string>());
  rapidjson::Document bounds;
  getDocument(bounds, param["bounds"].as<std::string>());

  frost::Document new_document;
  updateData(document, new_document);
  updateBounds(bounds, new_document);

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
    frost_nlp = new  frost::FROST_SOLVER(new_document, x0, new frost::JacGEvalSingleThread(new_document));
  else
    frost_nlp = new  frost::FROST_SOLVER(new_document, x0, new frost::JacGEvalMultiThread(new_document, param["threads"].as<int>()));
  
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

void updateData(rapidjson::Document &rapidDocument, frost::Document &document)
{
    assert(rapidDocument.IsObject());
    assert(rapidDocument.HasMember("Constraint"));
    assert(rapidDocument.HasMember("Objective"));
    assert(rapidDocument.HasMember("Variable"));
    assert(rapidDocument.HasMember("Options"));

    // Copying Variable data
    document.Variable.dimVars = rapidDocument["Variable"]["dimVars"].GetInt();

    // Copying Constraint data
    document.Constraint.numFuncs = rapidDocument["Constraint"]["numFuncs"].GetInt();
    document.Constraint.nnzJac = rapidDocument["Constraint"]["nnzJac"].GetInt();
    for (unsigned int i = 0; i < rapidDocument["Constraint"]["Funcs"].Size(); i++)
        document.Constraint.Funcs.push_back(rapidDocument["Constraint"]["Funcs"][i].GetInt());
    for (unsigned int i = 0; i < rapidDocument["Constraint"]["JacFuncs"].Size(); i++)
        document.Constraint.JacFuncs.push_back(rapidDocument["Constraint"]["JacFuncs"][i].GetInt());
    for (unsigned int i = 0; i < rapidDocument["Constraint"]["DepIndices"].Size(); i++)
    {
        std::vector<int> v;
        document.Constraint.DepIndices.push_back(v);
        if (rapidDocument["Constraint"]["DepIndices"][i].IsArray())
            for (unsigned int j = 0; j < rapidDocument["Constraint"]["DepIndices"][i].Size(); j++)
                document.Constraint.DepIndices[i].push_back(rapidDocument["Constraint"]["DepIndices"][i][j].GetInt());
        else if (rapidDocument["Constraint"]["DepIndices"][i].IsNull() == false)
            document.Constraint.DepIndices[i].push_back(rapidDocument["Constraint"]["DepIndices"][i].GetInt());
        else
            throw "null found";
    }
    for (unsigned int i = 0; i < rapidDocument["Constraint"]["AuxData"].Size(); i++)
    {
        std::vector<double> v;
        document.Constraint.AuxData.push_back(v);
        if (rapidDocument["Constraint"]["AuxData"][i].IsArray())
        {
            for (unsigned int j = 0; j < rapidDocument["Constraint"]["AuxData"][i].Size(); j++)
                document.Constraint.AuxData[i].push_back(rapidDocument["Constraint"]["AuxData"][i][j].GetDouble());
        }
        else if (rapidDocument["Constraint"]["AuxData"][i].IsNull() == false)
        {
            document.Constraint.AuxData[i].push_back(rapidDocument["Constraint"]["AuxData"][i].GetDouble());
        }
    }
    for (unsigned int i = 0; i < rapidDocument["Constraint"]["FuncIndices"].Size(); i++)
    {
        std::vector<int> v;
        document.Constraint.FuncIndices.push_back(v);
        if (rapidDocument["Constraint"]["FuncIndices"][i].IsArray())
            for (unsigned int j = 0; j < rapidDocument["Constraint"]["FuncIndices"][i].Size(); j++)
                document.Constraint.FuncIndices[i].push_back(rapidDocument["Constraint"]["FuncIndices"][i][j].GetInt());
        else
            document.Constraint.FuncIndices[i].push_back(rapidDocument["Constraint"]["FuncIndices"][i].GetInt());
    }
    for (unsigned int i = 0; i < rapidDocument["Constraint"]["nzJacRows"].Size(); i++)
        document.Constraint.nzJacRows.push_back(rapidDocument["Constraint"]["nzJacRows"][i].GetInt());
    for (unsigned int i = 0; i < rapidDocument["Constraint"]["nzJacCols"].Size(); i++)
        document.Constraint.nzJacCols.push_back(rapidDocument["Constraint"]["nzJacCols"][i].GetInt());
    for (unsigned int i = 0; i < rapidDocument["Constraint"]["nzJacIndices"].Size(); i++)
    {
        std::vector<int> v;
        document.Constraint.nzJacIndices.push_back(v);
        if (rapidDocument["Constraint"]["nzJacIndices"][i].IsArray())
            for (unsigned int j = 0; j < rapidDocument["Constraint"]["nzJacIndices"][i].Size(); j++)
                document.Constraint.nzJacIndices[i].push_back(rapidDocument["Constraint"]["nzJacIndices"][i][j].GetInt());
        else
            document.Constraint.nzJacIndices[i].push_back(rapidDocument["Constraint"]["nzJacIndices"][i].GetInt());
    }
    document.Constraint.Dimension = rapidDocument["Constraint"]["Dimension"].GetInt();

    // Copying Objective data
    document.Objective.numFuncs = rapidDocument["Objective"]["numFuncs"].GetInt();
    document.Objective.nnzJac = rapidDocument["Objective"]["nnzJac"].GetInt();
    for (unsigned int i = 0; i < rapidDocument["Objective"]["Funcs"].Size(); i++)
        document.Objective.Funcs.push_back(rapidDocument["Objective"]["Funcs"][i].GetInt());
    for (unsigned int i = 0; i < rapidDocument["Objective"]["JacFuncs"].Size(); i++)
        document.Objective.JacFuncs.push_back(rapidDocument["Objective"]["JacFuncs"][i].GetInt());
    for (unsigned int i = 0; i < rapidDocument["Objective"]["DepIndices"].Size(); i++)
    {
        std::vector<int> v;
        document.Objective.DepIndices.push_back(v);
        if (rapidDocument["Objective"]["DepIndices"][i].IsArray())
            for (unsigned int j = 0; j < rapidDocument["Objective"]["DepIndices"][i].Size(); j++)
                document.Objective.DepIndices[i].push_back(rapidDocument["Objective"]["DepIndices"][i][j].GetInt());
        else
            document.Objective.DepIndices[i].push_back(rapidDocument["Objective"]["DepIndices"][i].GetInt());
    }
    for (unsigned int i = 0; i < rapidDocument["Objective"]["AuxData"].Size(); i++)
    {
        std::vector<double> v;
        document.Objective.AuxData.push_back(v);
        if (rapidDocument["Objective"]["AuxData"][i].IsArray())
        {
            for (unsigned int j = 0; j < rapidDocument["Objective"]["AuxData"][i].Size(); j++)
                document.Objective.AuxData[i].push_back(rapidDocument["Objective"]["AuxData"][i][j].GetDouble());
        }
        else if (rapidDocument["Objective"]["AuxData"][i].IsNull() == false)
        {
            document.Objective.AuxData[i].push_back(rapidDocument["Objective"]["AuxData"][i].GetDouble());
        }
    }
    for (unsigned int i = 0; i < rapidDocument["Objective"]["FuncIndices"].Size(); i++)
    {
        std::vector<int> v;
        document.Objective.FuncIndices.push_back(v);
        if (rapidDocument["Objective"]["FuncIndices"][i].IsArray())
            for (unsigned int j = 0; j < rapidDocument["Objective"]["FuncIndices"][i].Size(); j++)
                document.Objective.FuncIndices[i].push_back(rapidDocument["Objective"]["FuncIndices"][i][j].GetInt());
        else
            document.Objective.FuncIndices[i].push_back(rapidDocument["Objective"]["FuncIndices"][i].GetInt());
    }
    for (unsigned int i = 0; i < rapidDocument["Objective"]["nzJacRows"].Size(); i++)
        document.Objective.nzJacRows.push_back(rapidDocument["Objective"]["nzJacRows"][i].GetInt());
    for (unsigned int i = 0; i < rapidDocument["Objective"]["nzJacCols"].Size(); i++)
        document.Objective.nzJacCols.push_back(rapidDocument["Objective"]["nzJacCols"][i].GetInt());
    for (unsigned int i = 0; i < rapidDocument["Objective"]["nzJacIndices"].Size(); i++)
    {
        std::vector<int> v;
        document.Objective.nzJacIndices.push_back(v);
        if (rapidDocument["Objective"]["nzJacIndices"][i].IsArray())
            for (unsigned int j = 0; j < rapidDocument["Objective"]["nzJacIndices"][i].Size(); j++)
                document.Objective.nzJacIndices[i].push_back(rapidDocument["Objective"]["nzJacIndices"][i][j].GetInt());
        else
            document.Objective.nzJacIndices[i].push_back(rapidDocument["Objective"]["nzJacIndices"][i].GetInt());
    }
    document.Objective.Dimension = rapidDocument["Objective"]["Dimension"].GetInt();
}


void updateBounds(rapidjson::Document &rapidDocument, frost::Document &document)
{
    assert(rapidDocument.IsObject());
    assert(rapidDocument.HasMember("Constraint"));
    assert(rapidDocument.HasMember("Variable"));

    // Copying Variable data
    for (unsigned int i = 0; i < rapidDocument["Variable"]["lb"].Size(); i++)
        document.Variable.lb.push_back(rapidDocument["Variable"]["lb"][i].GetDouble());
    for (unsigned int i = 0; i < rapidDocument["Variable"]["ub"].Size(); i++)
        document.Variable.ub.push_back(rapidDocument["Variable"]["ub"][i].GetDouble());

    // Copying Constraint data
    for (unsigned int i = 0; i < rapidDocument["Constraint"]["LowerBound"].Size(); i++)
        document.Constraint.LowerBound.push_back(rapidDocument["Constraint"]["LowerBound"][i].GetDouble());
    for (unsigned int i = 0; i < rapidDocument["Constraint"]["UpperBound"].Size(); i++)
        document.Constraint.UpperBound.push_back(rapidDocument["Constraint"]["UpperBound"][i].GetDouble());
}
