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

cmake = import('cmake')

cmake_silent_warning = cmake.subproject_options()
cmake_silent_warning.append_compile_args('c', '-w')
cmake_silent_warning.append_compile_args('cpp', '-w')

dobby_subproj = cmake.subproject('dobby', options: cmake_silent_warning)
simple_web_server_subproj = cmake.subproject('simple-web-server')
base64_subproj = cmake.subproject('tobiaslocker_base64')

dobby_dep = dobby_subproj.dependency('dobby_static')
simple_web_server_dep = simple_web_server_subproj.dependency('simple-web-server')
base64_dep = base64_subproj.dependency('base64')
protobuf_dep = dependency('protobuf')
lz4_dep = dependency('liblz4')
nlohmann_json_dep = dependency('nlohmann_json')
pugixml_dep = dependency('pugixml')
cpr_dep = dependency('cpr')
openssl_dep = dependency('openssl') # cascaded by simple-web-server

protoc = find_program('protoc')
proto_gen = generator(
    protoc,
    output: ['@BASENAME@.pb.cc', '@BASENAME@.pb.h'],
    arguments: [
        '--proto_path=@CURRENT_SOURCE_DIR@',
        '--cpp_out=@BUILD_DIR@',
        '@INPUT@',
    ],
)

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
    sources + proto_sources,
    include_directories: includes,
    dependencies: [
        dobby_dep,
        simple_web_server_dep,
        protobuf_dep,
        lz4_dep,
        nlohmann_json_dep,
        base64_dep,
        pugixml_dep,
        cpr_dep,
        openssl_dep,
    ],
)