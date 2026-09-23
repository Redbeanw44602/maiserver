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
supported_archs = ['x86_64', 'aarch64']

current_system = host_machine.system()
current_arch = host_machine.cpu_family()

if not supported_systems.contains(current_system) or not supported_archs.contains(current_arch)
    error('unsupported platform: ' + current_system + '/' + current_arch)
endif

cmake = import('cmake')

simple_web_server_opts = cmake.subproject_options()
simple_web_server_opts.add_cmake_defines(
    {
        # to allow subprojects to locate asio, '/conan' is specified in the build guidelines
        'CMAKE_PREFIX_PATH': meson.project_build_root() + '/conan',
        'USE_STANDALONE_ASIO': true,
        'USE_OPENSSL': false, # we don't need https
        'BUILD_TESTING': false,
    },
)

simple_web_server_subproj = cmake.subproject('simple-web-server', options: simple_web_server_opts)
simple_web_server_dep = simple_web_server_subproj.dependency('simple-web-server')

frida_gum_dep = dependency('frida-gum')
lz4_dep = dependency('liblz4')
nlohmann_json_dep = dependency('nlohmann_json')
pugixml_dep = dependency('pugixml')
base64_dep = dependency('base64')
cpr_dep = dependency('cpr')
date_dep = dependency('date')
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
        simple_web_server_dep,
        lz4_dep,
        nlohmann_json_dep,
        base64_dep,
        pugixml_dep,
        cpr_dep,
        date_dep,
        asio_dep,
        frida_gum_dep,
    ],
)