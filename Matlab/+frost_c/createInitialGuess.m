function createInitialGuess(solver, data_path, x0, name)
    if ~exist('x0', 'var') || isempty(x0)
        x0 = getInitialGuess(solver.Nlp, solver.Options.initialguess);
    end
    
    if ~exist('name', 'var')
        name = 'init';
    end
    
    if iscolumn(x0)
        x0 = x0';
    end
    
    if ~exist(data_path, 'dir')
        mkdir(data_path);
    end
    
    % Uses RapidJSON
    savejson('', x0, fullfile(data_path, [name, '.json']));
end
