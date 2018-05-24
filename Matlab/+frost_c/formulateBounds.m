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
    
    obj.Constraint.AuxData = cell(1, solver.Constraint.numFuncs);
    for i = 1:solver.Constraint.numFuncs
        obj.Constraint.AuxData{i} = [];
        for j = 1:length(solver.Constraint.AuxData{i})
            row = solver.Constraint.AuxData{i}{j};
            if iscolumn(row)
                row = row';
            end
            obj.Constraint.AuxData{i} = [obj.Constraint.AuxData{i}, row];
        end
    end
    
    % Objective
    obj.Objective.AuxData = cell(1, solver.Objective.numFuncs);
    for i = 1:solver.Objective.numFuncs
        obj.Objective.AuxData{i} = [];
        for j = 1:length(solver.Objective.AuxData{i})
            row = solver.Objective.AuxData{i}{j};
            if iscolumn(row)
                row = row';
            end
            obj.Objective.AuxData{i} = [obj.Objective.AuxData{i}, row];
        end
    end
end
