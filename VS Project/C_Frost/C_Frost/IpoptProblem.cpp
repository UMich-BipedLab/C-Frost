#include "rapidjson/document.h"
#include "frost/functionlist.hh"
#include "frost/IpoptProblem.h"

double out[100000];
double in[100000];

void frost::IpoptConstraint(double *c, const double *x, const rapidjson::Document &document) {
	int nConst = document["Constraint"]["numFuncs"].GetInt();
	int nOut = document["Constraint"]["Dimension"].GetInt();

	for (int i = 0; i < nOut; i++)
		c[i] = 0;

	for (int i = 0; i < nConst; i++)
	{
		int fIdx = document["Constraint"]["Funcs"][i].GetInt() - 1;
		int numDep = 0;
		if (document["Constraint"]["DepIndices"][i].IsArray())
		{
			numDep = document["Constraint"]["DepIndices"][i].Size();
			for (int j = 0; j < numDep; j++)
			{
				in[j] = x[document["Constraint"]["DepIndices"][i][j].GetInt() - 1];
			}
		}
		else if (document["Constraint"]["DepIndices"][i].IsNull() == false)
		{
			numDep = 1;
			in[0] = x[document["Constraint"]["DepIndices"][i].GetInt() - 1];
		}

		int numAux = 0;
		if (document["Constraint"]["AuxData"][i].IsArray())
		{
			numAux = document["Constraint"]["AuxData"][i].Size();
			for (int j = 0; j < numAux; j++)
			{
				in[j + numDep] = document["Constraint"]["AuxData"][i][j].GetDouble();
			}
		}
		else if (document["Constraint"]["AuxData"][i].IsNull() == false)
		{
			numAux = 1;
			in[0 + numDep] = document["Constraint"]["AuxData"][i].GetDouble();
		}

		frost::functions[fIdx](out, in);

		int numConst = 0;
		if (document["Constraint"]["FuncIndices"][i].IsArray())
		{
			numConst = document["Constraint"]["FuncIndices"][i].Size();
			for (int j = 0; j < numConst; j++)
			{
				c[document["Constraint"]["FuncIndices"][i][j].GetInt() - 1] += out[j];
			}
		}
		else if (document["Constraint"]["FuncIndices"][i].IsNull() == false)
		{
			numConst = 1;
			c[document["Constraint"]["FuncIndices"][i].GetInt() - 1] += out[0];
		}
	}
}

void frost::IpoptObjective(double &o, const double *x, const rapidjson::Document &document) {
	int nConst = document["Objective"]["numFuncs"].GetInt();
	int nOut = document["Objective"]["Dimension"].GetInt();

	o = 0;

	for (int i = 0; i < nConst; i++)
	{
		int fIdx = document["Objective"]["Funcs"][i].GetInt() - 1;
		int numDep = 0;
		if (document["Objective"]["DepIndices"][i].IsArray())
		{
			numDep = document["Objective"]["DepIndices"][i].Size();
			for (int j = 0; j < numDep; j++)
			{
				in[j] = x[document["Objective"]["DepIndices"][i][j].GetInt() - 1];
			}
		}
		else if (document["Objective"]["DepIndices"][i].IsNull() == false)
		{
			numDep = 1;
			in[0] = x[document["Objective"]["DepIndices"][i].GetInt() - 1];
		}

		int numAux = 0;
		if (document["Objective"]["AuxData"][i].IsArray())
		{
			numAux = document["Objective"]["AuxData"][i].Size();
			for (int j = 0; j < numAux; j++)
			{
				in[j + numDep] = document["Objective"]["AuxData"][i][j].GetDouble();
			}
		}
		else if (document["Objective"]["AuxData"][i].IsNull() == false)
		{
			numAux = 1;
			in[0 + numDep] = document["Objective"]["AuxData"][i].GetDouble();
		}

		frost::functions[fIdx](out, in);

		int numConst = 0;
		if (document["Objective"]["FuncIndices"][i].IsArray())
		{
			numConst = document["Objective"]["FuncIndices"][i].Size();
			for (int j = 0; j < numConst; j++)
			{
				o += out[j];
			}
		}
		else if (document["Objective"]["FuncIndices"][i].IsNull() == false)
		{
			numConst = 1;
			o += out[0];
		}
	}
}

void frost::IpoptJacobian(double *Jval, const double *x, const rapidjson::Document &document) {
	int nConst = document["Constraint"]["numFuncs"].GetInt();
	int nJOut = document["Constraint"]["nnzJac"].GetInt();

	for (int i = 0; i < nJOut; i++)
	{
		Jval[i] = 0;
	}

	for (int i = 0; i < nConst; i++)
	{
		if (document["Constraint"]["nzJacIndices"][i].IsNull())
			continue;

		int fIdx = document["Constraint"]["JacFuncs"][i].GetInt() - 1;
		int numDep = 0;
		if (document["Constraint"]["DepIndices"][i].IsArray())
		{
			numDep = document["Constraint"]["DepIndices"][i].Size();
			for (int j = 0; j < numDep; j++)
			{
				in[j] = x[document["Constraint"]["DepIndices"][i][j].GetInt() - 1];
			}
		}
		else if (document["Constraint"]["DepIndices"][i].IsNull() == false)
		{
			numDep = 1;
			in[0] = x[document["Constraint"]["DepIndices"][i].GetInt() - 1];
		}

		int numAux = 0;
		if (document["Constraint"]["AuxData"][i].IsArray())
		{
			numAux = document["Constraint"]["AuxData"][i].Size();
			for (int j = 0; j < numAux; j++)
			{
				in[j + numDep] = document["Constraint"]["AuxData"][i][j].GetDouble();
			}
		}
		else if (document["Constraint"]["AuxData"][i].IsNull() == false)
		{
			numAux = 1;
			in[0 + numDep] = document["Constraint"]["AuxData"][i].GetDouble();
		}

		frost::functions[fIdx](out, in);

		int numConst = 0;
		if (document["Constraint"]["nzJacIndices"][i].IsArray())
		{
			numConst = document["Constraint"]["nzJacIndices"][i].Size();
			for (int j = 0; j < numConst; j++)
			{
				Jval[document["Constraint"]["nzJacIndices"][i][j].GetInt() - 1] += out[j];
			}
		}
		else if (document["Constraint"]["FuncIndices"][i].IsNull() == false)
		{
			numConst = 1;
			Jval[document["Constraint"]["nzJacIndices"][i].GetInt() - 1] += out[0];
		}
	}
}

void frost::IpoptGradient(double *Jgrad, const double *x, const rapidjson::Document &document) {
	int nConst = document["Objective"]["numFuncs"].GetInt();
	int nJOut = document["Objective"]["nnzJac"].GetInt();

	for (int i = 0; i < nJOut; i++)
	{
		Jgrad[i] = 0;
	}

	for (int i = 0; i < nConst; i++)
	{
		if (document["Objective"]["nzJacIndices"][i].IsNull())
			continue;

		int fIdx = document["Objective"]["JacFuncs"][i].GetInt() - 1;
		int numDep = 0;
		if (document["Objective"]["DepIndices"][i].IsArray())
		{
			numDep = document["Objective"]["DepIndices"][i].Size();
			for (int j = 0; j < numDep; j++)
			{
				in[j] = x[document["Objective"]["DepIndices"][i][j].GetInt() - 1];
			}
		}
		else if (document["Objective"]["DepIndices"][i].IsNull() == false)
		{
			numDep = 1;
			in[0] = x[document["Objective"]["DepIndices"][i].GetInt() - 1];
		}

		int numAux = 0;
		if (document["Objective"]["AuxData"][i].IsArray())
		{
			numAux = document["Objective"]["AuxData"][i].Size();
			for (int j = 0; j < numAux; j++)
			{
				in[j + numDep] = document["Objective"]["AuxData"][i][j].GetDouble();
			}
		}
		else if (document["Objective"]["AuxData"][i].IsNull() == false)
		{
			numAux = 1;
			in[0 + numDep] = document["Objective"]["AuxData"][i].GetDouble();
		}

		frost::functions[fIdx](out, in);

		int numConst = 0;
		if (document["Objective"]["nzJacIndices"][i].IsArray())
		{
			numConst = document["Objective"]["nzJacIndices"][i].Size();
			for (int j = 0; j < numConst; j++)
			{
				Jgrad[document["Objective"]["nzJacIndices"][i][j].GetInt() - 1] += out[j];
			}
		}
		else if (document["Objective"]["FuncIndices"][i].IsNull() == false)
		{
			numConst = 1;
			Jgrad[document["Objective"]["nzJacIndices"][i].GetInt() - 1] += out[0];
		}
	}
}
