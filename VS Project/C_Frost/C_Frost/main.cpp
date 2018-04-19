#include "rapidjson/document.h"
#include <iostream>
#include <fstream>
#include <string>
#include "frost/IpoptProblem.h"
#include "frost/functionlist.hh"
#include <cmath>

using namespace std;

void getDocument(rapidjson::Document &document, const std::string &fileName);

void testIpoptConstraint(const rapidjson::Document &document);
void testIpoptOptimization(const rapidjson::Document &document);
void testIpoptJacobian(const rapidjson::Document &document);
void testIpoptGradient(const rapidjson::Document &document);

int main()
{
	rapidjson::Document document;
	getDocument(document, "res/data.json");

	assert(document.IsObject());
	assert(document.HasMember("Constraint"));
	assert(document.HasMember("Objective"));
	assert(document.HasMember("Variable"));
	assert(document.HasMember("Options"));

	testIpoptConstraint(document);
	testIpoptOptimization(document);
	testIpoptJacobian(document);
	testIpoptGradient(document);

	return 0;
}

void getDocument(rapidjson::Document &document, const std::string &fileName)
{
	ifstream in(fileName);
	std::string json((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
	in.close();

	document.Parse(json.c_str());
}

void testIpoptConstraint(const rapidjson::Document &document)
{
	rapidjson::Document testConst;
	getDocument(testConst, "res/ipopt_constraint_test.json");

	int nVar = document["Variable"]["dimVars"].GetInt();
	int nConst = document["Constraint"]["numFuncs"].GetInt();

	double c[2000];
	double x[2000];

	for (int i = 0; i < nVar; i++)
	{
		x[i] = testConst["x"][i].GetDouble();
	}

	frost::IpoptConstraint(c, x, document);

	double error = 0;
	for (int i = 0; i < nConst; i++)
	{
		error += pow(c[i] - testConst["C"][i].GetDouble(), 2);
	}

	cout << error << endl;
}

void testIpoptOptimization(const rapidjson::Document &document)
{
	rapidjson::Document testConst;
	getDocument(testConst, "res/ipopt_constraint_test.json");

	int nVar = document["Variable"]["dimVars"].GetInt();
	int nConst = document["Constraint"]["numFuncs"].GetInt();

	double o;
	double x[2000];

	for (int i = 0; i < nVar; i++)
	{
		x[i] = testConst["x"][i].GetDouble();
	}

	frost::IpoptObjective(o, x, document);

	cout << o << endl;
}

void testIpoptJacobian(const rapidjson::Document &document)
{
	rapidjson::Document testConst;
	getDocument(testConst, "res/ipopt_jacobian_test.json");

	int nVar = document["Variable"]["dimVars"].GetInt();
	int nJOut = document["Constraint"]["nnzJac"].GetInt();

	double x[2000];
	double *Jval;

	Jval = new double[nJOut];

	for (int i = 0; i < nVar; i++)
	{
		x[i] = testConst["x"][i].GetDouble();
	}

	frost::IpoptJacobian(Jval, x, document);

	double errorVal = 0;
	for (int i = 0; i < nJOut; i++)
	{
		errorVal += pow(Jval[i] - testConst["J_val"][i].GetDouble(), 2);
	}

	cout << errorVal << endl;

	delete[] Jval;
}


void testIpoptGradient(const rapidjson::Document &document)
{
	rapidjson::Document testConst;
	getDocument(testConst, "res/ipopt_gradient_test.json");

	int nVar = document["Variable"]["dimVars"].GetInt();
	int nJOut = document["Objective"]["nnzJac"].GetInt();

	double x[2000];
	double *Jgrad;

	Jgrad = new double[nJOut];

	for (int i = 0; i < nVar; i++)
	{
		x[i] = testConst["x"][i].GetDouble();
	}

	frost::IpoptGradient(Jgrad, x, document);

	double errorVal = 0;
	for (int i = 0; i < nJOut; i++)
	{
		errorVal += pow(Jgrad[i] - testConst["J_val"][i].GetDouble(), 2);
	}

	cout << errorVal << endl;

	delete[] Jgrad;
}
