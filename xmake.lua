add_rules('mode.debug', 'mode.release')

add_requires('dobby')
add_requires('protobuf-cpp')
add_requires('simple-web-server', {configs = {standalone_asio = true}})
add_requires('lz4')
add_requires('nlohmann_json')
add_requires('tobiaslocker_base64')
add_requires('pugixml')
add_requires('cpr')

add_repositories('local-repo repo')

set_allowedplats('linux')
set_allowedarchs('x86_64')

set_warnings('all', 'extra')
set_languages('c++23')

set_toolchains('clang')
-- set_runtimes('c++_static')

if is_mode('debug') then 
    add_defines('MAI_DEBUG')
end 

target('maiserver')
    set_kind('shared')
    add_rules('protobuf.cpp')
    add_files('src/**.cpp')
    add_files('src/proto/**.proto')
    add_includedirs('src')
    add_packages('dobby',
                 'protobuf-cpp',
                 'simple-web-server',
                 'lz4',
                 'nlohmann_json',
                 'tobiaslocker_base64',
                 'pugixml',
                 'cpr')

