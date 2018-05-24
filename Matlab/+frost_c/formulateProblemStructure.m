function [obj] = formulateProblemStructure(solver, funcs)
    funcs_str = string();
    for i = 1:length(funcs)
        funcs_str(i) = string(funcs{i});
    end
    
    obj = struct();
    
    % Variable
    [dimVars, ~, ~] = getVarInfo(solver.Nlp);
    
    obj.Variable = struct();
    obj.Variable.dimVars = dimVars;
    
    % Constraints
    obj.Constraint = solver.Constraint;
    
    obj.Constraint.Funcs = zeros(1, solver.Constraint.numFuncs);
    for i = 1:solver.Constraint.numFuncs
        %[Lia, Locb] = ismember(solver.Constraint.Funcs{i}, funcs);
        Locb = binarySearch(1, length(funcs_str), solver.Constraint.Funcs{i});
        obj.Constraint.Funcs(i) = Locb;
    end
    
    obj.Constraint.JacFuncs = zeros(1, solver.Constraint.numFuncs);
    for i = 1:solver.Constraint.numFuncs
        %[Lia, Locb] = ismember(solver.Constraint.JacFuncs{i}, funcs);
        Locb = binarySearch(1, length(funcs_str), solver.Constraint.JacFuncs{i});
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
    
    obj.Constraint = rmfield(obj.Constraint, 'AuxData');
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
    obj.Constraint = rmfield(obj.Constraint, 'LowerBound');
    obj.Constraint = rmfield(obj.Constraint, 'UpperBound');
    obj.Constraint = rmfield(obj.Constraint, 'JacStructFuncs');
    if iscolumn(obj.Constraint.nzJacIndices)
        obj.Constraint.nzJacIndices = obj.Constraint.nzJacIndices';
    end
    
    
    % Objective
    obj.Objective = solver.Objective;
    
    obj.Objective.Funcs = zeros(1, solver.Objective.numFuncs);
    for i = 1:solver.Objective.numFuncs
        %[Lia, Locb] = ismember(solver.Objective.Funcs{i}, funcs);
        Locb = binarySearch(1, length(funcs_str), solver.Objective.Funcs{i});
        obj.Objective.Funcs(i) = Locb;
    end
    
    obj.Objective.JacFuncs = zeros(1, solver.Objective.numFuncs);
    for i = 1:solver.Objective.numFuncs
        %[Lia, Locb] = ismember(solver.Objective.JacFuncs{i}, funcs);
        Locb = binarySearch(1, length(funcs_str), solver.Objective.JacFuncs{i});
        obj.Objective.JacFuncs(i) = Locb;
    end
    
    obj.Objective.DepIndices = cell(1, solver.Objective.numFuncs);
    for i = 1:solver.Objective.numFuncs
        obj.Objective.DepIndices{i} = [];
        for j = 1:length(solver.Objective.DepIndices{i})
            row = solver.Objective.DepIndices{i}{j};
            if iscolumn(row)
                row = row';
            end
            obj.Objective.DepIndices{i} = [obj.Objective.DepIndices{i}, row];
        end
    end
    
    obj.Objective = rmfield(obj.Objective, 'AuxData');
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
    obj.Objective = rmfield(obj.Objective, 'JacStructFuncs');
    
    
    
    % Options
    obj.Options = solver.Options;
    
    function result = binarySearch(a, b, word)
        if a > b
            result = 0;
        elseif a == b
            if word == funcs_str(a)
                result = a;
            else
                result = 0;
            end
        else
            mid = floor((a + b) / 2);
            
            if word == funcs_str(mid)
                result = mid;
            elseif word < funcs_str(mid)
                result = binarySearch(a, mid - 1, word);
            else
                result = binarySearch(mid + 1, b, word);
            end
        end
    end
end
