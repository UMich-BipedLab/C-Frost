function [rootPath] = getRootPath()
    pathstr = mfilename('fullpath');
    [pathstr, ~, ~] = fileparts([pathstr, '.m']);
    [rootPath, ~, ~] = fileparts(pathstr);
end
