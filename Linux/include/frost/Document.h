#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include "rapidjson/document.h"

#include <vector>
#include <iostream>

namespace frost
{
    struct Document {
        struct VariableStruct {
            int dimVars;
            std::vector<double> lb;
            std::vector<double> ub;
        };
        struct ConstraintStruct {
            int numFuncs;
            int nnzJac;
            std::vector<int> Funcs;
            std::vector<int> JacFuncs;
            std::vector< std::vector<int> > DepIndices;
            std::vector< std::vector<double> > AuxData;
            std::vector< std::vector<bool> > AuxDataIsNull;
            std::vector< std::vector<int> > FuncIndices;
            std::vector<int> nzJacRows;
            std::vector<int> nzJacCols;
            std::vector< std::vector<int> > nzJacIndices;
            int Dimension;
            std::vector<double> LowerBound;
            std::vector<double> UpperBound;
        };
        
        VariableStruct Varaible;
        ConstraintStruct Constraint;
        ConstraintStruct Objective;
    };

    void copyFrostDocumentFromRapidJsonDocument(rapidjson::Document &rapidDocument, frost::Document &document)
    {
        assert(rapidDocument.IsObject());
        assert(rapidDocument.HasMember("Constraint"));
        assert(rapidDocument.HasMember("Objective"));
        assert(rapidDocument.HasMember("Variable"));
        assert(rapidDocument.HasMember("Options"));

        // Copying Variable data
        document.Varaible.dimVars = rapidDocument["Variable"]["dimVars"].GetInt();
        for (unsigned int i = 0; i < rapidDocument["Variable"]["lb"].Size(); i++)
            document.Varaible.lb.push_back(rapidDocument["Variable"]["lb"][i].GetDouble());
        for (unsigned int i = 0; i < rapidDocument["Variable"]["ub"].Size(); i++)
            document.Varaible.ub.push_back(rapidDocument["Variable"]["ub"][i].GetDouble());

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
            std::vector<double> v1;
            std::vector<bool> v2;
            document.Constraint.AuxData.push_back(v1);
            document.Constraint.AuxDataIsNull.push_back(v2);
            if (rapidDocument["Constraint"]["AuxData"][i].IsArray())
                for (unsigned int j = 0; j < rapidDocument["Constraint"]["AuxData"][i].Size(); j++)
                {
                    document.Constraint.AuxData[i].push_back(rapidDocument["Constraint"]["AuxData"][i][j].GetDouble());
                    document.Constraint.AuxDataIsNull[i].push_back(false);
                }
            else if (rapidDocument["Constraint"]["AuxData"][i].IsNull() == false)
            {
                document.Constraint.AuxData[i].push_back(rapidDocument["Constraint"]["AuxData"][i].GetDouble());
                document.Constraint.AuxDataIsNull[i].push_back(false);
            }
            else
            {
                document.Constraint.AuxData[i].push_back(0);
                document.Constraint.AuxDataIsNull[i].push_back(true);
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
        for (unsigned int i = 0; i < rapidDocument["Constraint"]["LowerBound"].Size(); i++)
            document.Constraint.LowerBound.push_back(rapidDocument["Constraint"]["LowerBound"][i].GetDouble());
        for (unsigned int i = 0; i < rapidDocument["Constraint"]["UpperBound"].Size(); i++)
            document.Constraint.UpperBound.push_back(rapidDocument["Constraint"]["UpperBound"][i].GetDouble());

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
            std::vector<double> v1;
            std::vector<bool> v2;
            document.Objective.AuxData.push_back(v1);
            document.Objective.AuxDataIsNull.push_back(v2);
            if (rapidDocument["Objective"]["AuxData"][i].IsArray())
                for (unsigned int j = 0; j < rapidDocument["Objective"]["AuxData"][i].Size(); j++)
                {
                    document.Objective.AuxData[i].push_back(rapidDocument["Objective"]["AuxData"][i][j].GetDouble());
                    document.Objective.AuxDataIsNull[i].push_back(false);
                }
            else if (rapidDocument["Objective"]["AuxData"][i].IsNull() == false)
            {
                document.Objective.AuxData[i].push_back(rapidDocument["Objective"]["AuxData"][i].GetDouble());
                document.Objective.AuxDataIsNull[i].push_back(false);
            }
            else
            {
                document.Objective.AuxData[i].push_back(0);
                document.Objective.AuxDataIsNull[i].push_back(true);
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
}

#endif
