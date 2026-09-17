protobuf_dep = dependency('protobuf')
protoc = find_program('protoc')

proto_files = ['micromsg.proto']
proto_gen_cs = []
proto_gen_hs = []

# https://github.com/mesonbuild/meson/issues/4943
foreach proto : proto_files
    gen = custom_target(
        proto + '_gen',
        output: ['@BASENAME@.pb.cc', '@BASENAME@.pb.h'],
        input: proto,
        command: [
            protoc,
            '--proto_path=@CURRENT_SOURCE_DIR@',
            '--cpp_out=@OUTDIR@',
            '@INPUT@',
        ],
    )
    proto_gen_cs += gen[0]
    proto_gen_hs += gen[1]
endforeach

proto_inc = include_directories('.', is_system: true)

proto_lib = static_library(
    'proto_lib',
    proto_gen_cs,
    dependencies: protobuf_dep,
    include_directories: proto_inc,
    override_options: ['warning_level=0'],
)

micromsg_dep = declare_dependency(
    sources: proto_gen_hs,
    include_directories: proto_inc,
    link_with: proto_lib,
    dependencies: protobuf_dep,
)