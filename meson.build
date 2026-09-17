project(
    'maiserver',
    'cpp',
    version: '0.0.1',
    license: 'GPL-3.0-only',
    default_options: [
        'cpp_std=c++23',
        'warning_level=3',
        'default_library=static',
    ],
)

supported_systems = ['linux']
supported_archs = ['x86_64']

current_system = host_machine.system()
current_arch = host_machine.cpu_family()

if not supported_systems.contains(current_system) or not supported_archs.contains(current_arch)
    error('unsupported platform: ' + current_system + '/' + current_arch)
endif

cmake = import('cmake')

cmake_silent_warning = cmake.subproject_options()
cmake_silent_warning.append_compile_args('c', '-w')
cmake_silent_warning.append_compile_args('cpp', '-w')

simple_web_server_opts = cmake.subproject_options()
simple_web_server_opts.add_cmake_defines(
    {
        'CMAKE_PREFIX_PATH': meson.project_build_root() + '/conan',
        'USE_STANDALONE_ASIO': true,
        'USE_OPENSSL': false,
    },
)

dobby_subproj = cmake.subproject('dobby', options: cmake_silent_warning)
simple_web_server_subproj = cmake.subproject('simple-web-server', options: simple_web_server_opts)
base64_subproj = cmake.subproject('tobiaslocker_base64')

dobby_dep = dobby_subproj.dependency('dobby_static')
simple_web_server_dep = simple_web_server_subproj.dependency('simple-web-server')
base64_dep = base64_subproj.dependency('base64')
lz4_dep = dependency('liblz4')
nlohmann_json_dep = dependency('nlohmann_json')
pugixml_dep = dependency('pugixml')
cpr_dep = dependency('cpr')
asio_dep = dependency('asio') # cascaded by simple-web-server

if get_option('debug')
    add_project_arguments('-DMAI_DEBUG', language: ['cpp'])
endif

includes = include_directories('src')
sources = files(
    'src/maiserver.cpp',
    'src/service/aimetoken.cpp',
    'src/service/qrcode.cpp',
)

subdir('src/proto')

shared_library(
    'maiserver',
    sources,
    include_directories: includes,
    dependencies: [
        micromsg_dep,
        dobby_dep,
        simple_web_server_dep,
        lz4_dep,
        nlohmann_json_dep,
        base64_dep,
        pugixml_dep,
        cpr_dep,
        asio_dep,
    ],
)