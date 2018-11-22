cc = meson.get_compiler('cpp')
luaDep = dependency('lua', version: '>=5.3')
stickDep = cc.find_library('Stick')

exe = executable('PaperLuaSolTests', 'PaperLuaSolTests.cpp', 
    dependencies: [stickDep, luaDep, dependency('threads')], 
    include_directories: incDirs,
    cpp_args: '-fsanitize=address',
    link_args: '-fsanitize=address')

test('PaperLuaSol Tests', exe, workdir: meson.current_build_dir())