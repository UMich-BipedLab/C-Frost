#ifndef DOCUMENT_H_
#define DOCUMENT_H_

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
            std::vector< std::vector<int> > FuncIndices;
            std::vector<int> nzJacRows;
            std::vector<int> nzJacCols;
            std::vector< std::vector<int> > nzJacIndices;
            int Dimension;
            std::vector<double> LowerBound;
            std::vector<double> UpperBound;
        };
        
        VariableStruct Variable;
        ConstraintStruct Constraint;
        ConstraintStruct Objective;
    };
}

#endif
