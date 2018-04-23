function createInitialGuess(solver, data_path)
    x0 = getInitialGuess(solver.Nlp, solver.Options.initialguess);
    if iscolumn(x0)
        x0 = x0';
    end
    
    if ~exist(data_path, 'dir')
        mkdir(data_path);
    end
    
    % Uses RapidJSON
    savejson('', x0, fullfile(data_path,'init.json'));
end
