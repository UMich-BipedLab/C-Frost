function createBoundsFile(solver, funcs, data_path, name)
    if ~exist('name', 'var')
        name = 'bounds';
    end

    obj = frost_c.formulateBounds(solver, funcs);
    
    if ~exist(data_path, 'dir')
        mkdir(data_path);
    end
    
    % Uses RapidJSON
    savejson('', obj, fullfile(data_path, [name, '.json']));
end
