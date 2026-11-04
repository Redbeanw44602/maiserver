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
simple_websocket_server_dep = dependency('simple-websocket-server')
lz4_dep = dependency('liblz4')
nlohmann_json_dep = dependency('nlohmann_json')
pugixml_dep = dependency('pugixml')
tidy_dep = dependency('tidy')
base64_dep = dependency('base64')
cpr_dep = dependency('cpr')
date_dep = dependency('date')
sigslot_dep = dependency('sigslot')
reflect_cpp_dep = dependency('reflect-cpp')

if get_option('debug')
    add_project_arguments('-DMAI_DEBUG', language: ['cpp'])
endif

includes = include_directories('src')
sources = files(
    'src/api/basic.cpp',
    'src/api/streaming.cpp',
    'src/maidebug_helper.cpp',
    'src/maiserver.cpp',
    'src/maiserver_slim.cpp',
    'src/mem/async_wrapper/cloud_proxy.cpp',
    'src/service/aimetoken.cpp',
    'src/service/maimai-dx/home/home.cpp',
    'src/service/maimai-dx/home/rating_target_music.cpp',
    'src/service/maimai-dx/session.cpp',
    'src/service/qrcode.cpp',
    'src/util/html_parser.cpp',
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
        simple_websocket_server_dep,
        lz4_dep,
        nlohmann_json_dep,
        base64_dep,
        pugixml_dep,
        tidy_dep,
        cpr_dep,
        date_dep,
        sigslot_dep,
        reflect_cpp_dep,
    ],
)