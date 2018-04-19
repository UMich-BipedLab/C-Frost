function [obj] = generateProblemStructure(solver, funcs, nums)
    obj = struct();
    
    % Constraints
    obj.Constraint = solver.Constraint;
    
    obj.Constraint.Funcs = zeros(1, solver.Constraint.numFuncs);
    for i = 1:solver.Constraint.numFuncs
        [Lia, Locb] = ismember(solver.Constraint.Funcs{i}, funcs);
        obj.Constraint.Funcs(i) = Locb;
    end
    
    obj.Constraint.JacFuncs = zeros(1, solver.Constraint.numFuncs);
    for i = 1:solver.Constraint.numFuncs
        [Lia, Locb] = ismember(solver.Constraint.JacFuncs{i}, funcs);
        obj.Constraint.JacFuncs(i) = Locb;
    end
    
    obj.Constraint.DepIndices = cell(1, solver.Constraint.numFuncs);
    for i = 1:solver.Constraint.numFuncs
        obj.Constraint.DepIndices{i} = [];
        for j = 1:length(solver.Constraint.DepIndices{i})
            row = solver.Constraint.DepIndices{i}{j};
            if iscolumn(row)
                row = row';
            end
            obj.Constraint.DepIndices{i} = [obj.Constraint.DepIndices{i}, row];
        end
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
    
    obj.Constraint = rmfield(obj.Constraint, 'Names');
    
    if iscolumn(obj.Constraint.FuncIndices)
        obj.Constraint.FuncIndices = obj.Constraint.FuncIndices';
    end
    if iscolumn(obj.Constraint.nzJacRows)
        obj.Constraint.nzJacRows = obj.Constraint.nzJacRows';
    end
    if iscolumn(obj.Constraint.nzJacCols)
        obj.Constraint.nzJacCols = obj.Constraint.nzJacCols';
    end
    if iscolumn(obj.Constraint.nzJacIndices)
        obj.Constraint.nzJacIndices = obj.Constraint.nzJacIndices';
    end
    if iscolumn(obj.Constraint.LowerBound)
        obj.Constraint.LowerBound = obj.Constraint.LowerBound';
    end
    if iscolumn(obj.Constraint.UpperBound)
        obj.Constraint.UpperBound = obj.Constraint.UpperBound';
    end
    
    % Objective
    obj.Objective = solver.Objective;
    
    obj.Objective.Funcs = zeros(1, solver.Objective.numFuncs);
    for i = 1:solver.Objective.numFuncs
        [Lia, Locb] = ismember(solver.Objective.Funcs{i}, funcs);
        obj.Objective.Funcs(i) = Locb;
    end
    
    obj.Objective.JacFuncs = zeros(1, solver.Objective.numFuncs);
    for i = 1:solver.Objective.numFuncs
        [Lia, Locb] = ismember(solver.Objective.JacFuncs{i}, funcs);
        obj.Objective.JacFuncs(i) = Locb;
    end
    
    obj.Objective.DepIndices = cell(1, solver.Objective.numFuncs);
    for i = 1:solver.Objective.numFuncs
        obj.Objective.DepIndices{i} = zeros(0, 1);
        for j = 1:length(solver.Objective.DepIndices{i})
            row = solver.Objective.DepIndices{i}{j};
            if iscolumn(row)
                row = row';
            end
            obj.Objective.DepIndices{i} = [obj.Objective.DepIndices{i}, row];
        end
    end
    
    obj.Objective.AuxData = cell(1, solver.Objective.numFuncs);
    for i = 1:solver.Objective.numFuncs
        obj.Objective.AuxData{i} = zeros(0, 1);
        for j = 1:length(solver.Objective.AuxData{i})
            row = solver.Objective.AuxData{i}{j};
            if iscolumn(row)
                row = row';
            end
            obj.Objective.AuxData{i} = [obj.Objective.AuxData{i}, row];
        end
    end
    
    obj.Objective = rmfield(obj.Objective, 'Names');
    
    if iscolumn(obj.Objective.FuncIndices)
        obj.Objective.FuncIndices = obj.Objective.FuncIndices';
    end
    if iscolumn(obj.Objective.nzJacRows)
        obj.Objective.nzJacRows = obj.Objective.nzJacRows';
    end
    if iscolumn(obj.Objective.nzJacCols)
        obj.Objective.nzJacCols = obj.Objective.nzJacCols';
    end
    if iscolumn(obj.Objective.nzJacIndices)
        obj.Objective.nzJacIndices = obj.Objective.nzJacIndices';
    end
    if iscolumn(obj.Objective.LowerBound)
        obj.Objective.LowerBound = obj.Objective.LowerBound';
    end
    if iscolumn(obj.Objective.UpperBound)
        obj.Objective.UpperBound = obj.Objective.UpperBound';
    end
    
    % Variable
    [dimVars, lb, ub] = getVarInfo(solver.Nlp);
    
    obj.Variable = struct();
    obj.Variable.dimVars = dimVars;
    obj.Variable.lb = lb;
    obj.Variable.ub = ub;
    
    % Options
    obj.Options = solver.Options;
end
