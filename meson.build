project('PaperLuaSol', 'cpp', default_options : ['cpp_std=c++14'])

#remove this rather bogus warning
add_project_arguments('-Wno-missing-braces', language: 'cpp')

incDirs = include_directories('.')

# meseon dependency function to find lua is not very consistent accross platforms yet, so
# we need to do a lot of manual lifting for now
foreach name : ['lua', 'lua5.3', 'lua-5.3', 'lua53']
    luaDep = dependency(name, version: '>=5.3', required: false)
    if luaDep.found()
        break
    endif
endforeach

if not luaDep.found()
    error('Lua could not be found!')
endif

incDirs = include_directories('.')

install_headers('PaperLuaSol/PaperLuaSol.hpp', subdir: 'PaperLuaSol')

cc = meson.get_compiler('cpp')
if get_option('buildSubmodules') == false
    stickDep = cc.find_library('Stick')
    paperDep = cc.find_library('Paper2')
    crunchLuaDep = cc.find_library('CrunchLuaSol')
    paperLuaDeps = [stickDep, crunchLuaDep, dependency('threads')]
else
    stickProj = subproject('Stick')
    crunchLuaProj = subproject('CrunchLuaSol')
    paperProj = subproject('Paper2')
    paperLuaDeps = [stickProj.get_variable('stickDep'), paperProj.get_variable('paperDep'), crunchLuaProj.get_variable('crunchLuaDep')]
endif

if host_machine.system() == 'linux'
    paperLuaDeps += cc.find_library('dl', required : true)
endif

paperLuaDeps += luaDep

paperLua = library('PaperLuaSol', 
        'PaperLuaSol/PaperLuaSol.cpp',
        dependencies: paperLuaDeps, 
        include_directories : incDirs,
        install: true)

paperLuaDep = declare_dependency(link_with : paperLua, 
    dependencies: paperLuaDeps, 
    include_directories: incDirs)

