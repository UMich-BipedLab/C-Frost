function [funcs, nums] = getAllFuncs(solver)
    import java.util.TreeMap
    
    map = TreeMap;
    for i = 1:solver.Constraint.numFuncs
        num = length(solver.Constraint.DepIndices{i}) + length(solver.Constraint.AuxData{i});
        map.put(solver.Constraint.Funcs{i}, num);
        map.put(solver.Constraint.JacFuncs{i}, num);
    end
    for i = 1:solver.Objective.numFuncs
        num = length(solver.Objective.DepIndices{i}) + length(solver.Objective.AuxData{i});
        map.put(solver.Objective.Funcs{i}, num);
        map.put(solver.Objective.JacFuncs{i}, num);
    end
    
    set = map.keySet();
    itr = set.iterator;
    
    funcs = {};
    nums = [];
    i = 0;
    while itr.hasNext
        i = i + 1;
        funcs{i} = itr.next;
        nums(i) = map.get(funcs{i});
    end
end
