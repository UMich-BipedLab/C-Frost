function createDataFile(solver, funcs, data_path)
    obj = frost_c.formulateProblemStructure(solver, funcs);
    
    if ~exist(data_path, 'dir')
        mkdir(data_path);
    end
    
    % Uses RapidJSON
    savejson('', obj, fullfile(data_path,'data.json'));
end
