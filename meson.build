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

frida_gum_dep = dependency('frida-gum')
simple_web_server_dep = dependency('simple-web-server')
lz4_dep = dependency('liblz4')
nlohmann_json_dep = dependency('nlohmann_json')
pugixml_dep = dependency('pugixml')
base64_dep = dependency('base64')
cpr_dep = dependency('cpr')
date_dep = dependency('date')

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
        frida_gum_dep,
        simple_web_server_dep,
        lz4_dep,
        nlohmann_json_dep,
        base64_dep,
        pugixml_dep,
        cpr_dep,
        date_dep,
    ],
)