function [obj] = formulateProblemStructure(solver, funcs)
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
    
    Js = sparse2(obj.Constraint.nzJacRows, obj.Constraint.nzJacCols, ...
        ones(obj.Constraint.nnzJac,1), obj.Constraint.Dimension,...
        dimVars, obj.Constraint.nnzJac);
    
    [iRows, jCols] = find(Js);
    
    for i=1:obj.Constraint.numFuncs
        if ~isempty(obj.Constraint.nzJacIndices{i})     
            
            % get the sparsity pattern (i.e., the indices of non-zero elements)
            % of the Jacobian of the current function
            
            jac_pattern = feval(obj.Constraint.JacStructFuncs{i},0);
            % retrieve the indices of dependent variables
            dep_indices = obj.Constraint.DepIndices{i};
            func_indics = obj.Constraint.FuncIndices{i};
            
            if iscolumn(func_indics(jac_pattern(:,1)))
                f_list = func_indics(jac_pattern(:,1));
            else
                f_list = func_indics(jac_pattern(:,1))';
            end
            
            if iscolumn(dep_indices(jac_pattern(:,2)))
                x_list = dep_indices(jac_pattern(:,2));
            else
                x_list = dep_indices(jac_pattern(:,2))';
            end
            
            idx = arrayfun(@(x,y)intersect(find(iRows==x),find(jCols==y)),...
               f_list , x_list);
            
            obj.Constraint.nzJacIndices{i} = idx';
%             %| @note The JacPattern gives the indices of non-zero Jacobian
%             % elements of the current function.
%             func_struct.nzJacRows(jac_index) = func_indics(jac_pattern.Rows);
%             
%             func_struct.nzJacCols(jac_index) = dep_indices(jac_pattern.Cols);
        end
    end
    obj.Constraint.nzJacRows = iRows';
    obj.Constraint.nzJacCols = jCols';
    obj.Constraint.nnzJac = length(iRows);
    obj.Constraint = rmfield(obj.Constraint, 'JacStructFuncs');
    %     if iscolumn(obj.Constraint.nzJacRows)
    %         obj.Constraint.nzJacRows = obj.Constraint.nzJacRows';
    %     end
    %     if iscolumn(obj.Constraint.nzJacCols)
    %         obj.Constraint.nzJacCols = obj.Constraint.nzJacCols';
    %     end
    if iscolumn(obj.Constraint.nzJacIndices)
        obj.Constraint.nzJacIndices = obj.Constraint.nzJacIndices';
    end
    obj.Constraint.LowerBound(obj.Constraint.LowerBound == -Inf) = -1e6;
    if iscolumn(obj.Constraint.LowerBound)
        obj.Constraint.LowerBound = obj.Constraint.LowerBound';
    end
    obj.Constraint.UpperBound(obj.Constraint.UpperBound == Inf) = 1e6;
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
        obj.Objective.AuxData{i} = [];
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
    obj.Objective = rmfield(obj.Objective, 'LowerBound');
    obj.Objective = rmfield(obj.Objective, 'UpperBound');
    
    
    
    % Options
    obj.Options = solver.Options;
end
