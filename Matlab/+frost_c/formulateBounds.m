function [obj] = formulateBounds(solver, funcs)
    funcs_str = string();
    for i = 1:length(funcs)
        funcs_str(i) = string(funcs{i});
    end
    
    obj = struct();
    
    % Variable
    [dimVars, lb, ub] = getVarInfo(solver.Nlp);
    if iscolumn(lb)
        lb = lb';
    end
    if iscolumn(ub)
        ub = ub';
    end
    
    obj.Variable = struct();
    obj.Variable.dimVars = dimVars;
    lb(lb == -Inf) = -1e6;
    obj.Variable.lb = lb;
    ub(ub == Inf) = 1e6;
    obj.Variable.ub = ub;
    
    % Constraints
    obj.Constraint.LowerBound = solver.Constraint.LowerBound;
    obj.Constraint.UpperBound = solver.Constraint.UpperBound;
    obj.Constraint.LowerBound(obj.Constraint.LowerBound == -Inf) = -1e6;
    if iscolumn(obj.Constraint.LowerBound)
        obj.Constraint.LowerBound = obj.Constraint.LowerBound';
    end
    obj.Constraint.UpperBound(obj.Constraint.UpperBound == Inf) = 1e6;
    if iscolumn(obj.Constraint.UpperBound)
        obj.Constraint.UpperBound = obj.Constraint.UpperBound';
    end
end
