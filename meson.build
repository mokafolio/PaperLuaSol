project('PaperLuaSol', 'cpp', default_options : ['cpp_std=c++14'])

#remove this rather bogus warning
add_project_arguments('-Wno-missing-braces', language: 'cpp')

incDirs = include_directories('.')

if meson.is_subproject() == false
    install_headers('PaperLuaSol/PaperLuaSol.hpp', subdir: 'PaperLuaSol')
endif

paperLuaDep = declare_dependency(include_directories: incDirs)
