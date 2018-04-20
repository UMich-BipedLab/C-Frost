#include "rapidjson/document.h"
#include "frost/functionlist.hh"
#include "frost/IpoptProblem.h"
#include "IpTNLP.hpp"

using namespace Ipopt;

double in[200000];
double out[200000];

frost::FROST_SOLVER::FROST_SOLVER(const rapidjson::Document &document) : document(document)
{
	int nVar = document["Variable"]["dimVars"].GetInt();
	int nConst = document["Constraint"]["numFuncs"].GetInt();
	int nOut = document["Constraint"]["Dimension"].GetInt();
	int nJOutConst = document["Constraint"]["nnzJac"].GetInt();
	int nJOutObj = document["Objective"]["nnzJac"].GetInt();

	in = new double[nJOutConst];
	out = new double[nJOutConst];
}

frost::FROST_SOLVER::~FROST_SOLVER()
{
	delete []in;
	delete []out
}

bool frost::FROST_SOLVER::eval_g(Index n, const Number* x, bool new_x, Index m, Number* g)
{
	int nVar = document["Variable"]["dimVars"].GetInt();
	int nConst = document["Constraint"]["numFuncs"].GetInt();
	int nOut = document["Constraint"]["Dimension"].GetInt();

	assert(n == nVar);
	assert(m == nOut);

	for (int i = 0; i < nOut; i++)
		g[i] = 0;
        
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
				g[document["Constraint"]["FuncIndices"][i][j].GetInt() - 1] += out[j];
			}
		}
		else if (document["Constraint"]["FuncIndices"][i].IsNull() == false)
		{
			numConst = 1;
			g[document["Constraint"]["FuncIndices"][i].GetInt() - 1] += out[0];
		}
	}

	return true;
}

bool frost::FROST_SOLVER::eval_f(Index n, const Number* x, bool new_x, Number& obj_value)
{
	int nVar = document["Variable"]["dimVars"].GetInt();
	int nConst = document["Objective"]["numFuncs"].GetInt();
	int nOut = document["Objective"]["Dimension"].GetInt();

	assert(n == nVar);

	obj_value = 0;

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
				obj_value += out[j];
			}
		}
		else if (document["Objective"]["FuncIndices"][i].IsNull() == false)
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
	int nVar = document["Variable"]["dimVars"].GetInt();
	int nConst = document["Constraint"]["numFuncs"].GetInt();
	int nJOut = document["Constraint"]["nnzJac"].GetInt();

	assert(n == nVar);
	assert(m == nOut);
	assert(n_ele_jac == nJOut);

	if (values == NULL)
	{
		for (int i = 0; i < nJOut; i++)
		{
			iRow[i] = document["Constraint"]["nzJacRows"][i].GetInt() - 1;
			jCol[i] = document["Constraint"]["nzJacCols"][i].GetInt() - 1;
		}

		return true;
	}

	for (int i = 0; i < nJOut; i++)
	{
		values[i] = 0;
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
				values[document["Constraint"]["nzJacIndices"][i][j].GetInt() - 1] += out[j];
			}
		}
		else if (document["Constraint"]["FuncIndices"][i].IsNull() == false)
		{
			numConst = 1;
			values[document["Constraint"]["nzJacIndices"][i].GetInt() - 1] += out[0];
		}
	}
}

bool frost::FROST_SOLVER::eval_grad_f(Index n, const Number* x, bool new_x, Number* grad_f)
{
	int nVar = document["Variable"]["dimVars"].GetInt();
	int nConst = document["Objective"]["numFuncs"].GetInt();
	int nJOut = document["Objective"]["nnzJac"].GetInt();

	assert(n == nVar);

	for (int i = 0; i < nVar; i++)
	{
		grad_f[i] = 0;
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
				int fIdx = document["Objective"]["nzJacIndices"][i][j].GetInt() - 1;
				int index = document["Objective"]["nzJacCols"][i][fIdx].GetInt() - 1;
				grad_f[index] += out[j];

			}
		}
		else if (document["Objective"]["FuncIndices"][i].IsNull() == false)
		{
			numConst = 1;
			int fIdx = document["Objective"]["nzJacIndices"][i].GetInt() - 1;
			int index = document["Objective"]["nzJacCols"][i][fIdx].GetInt() - 1;
			grad_f[index] += out[0];
		}
	}

	return true;
}




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
