from conan import ConanFile
from conan.tools.build import check_min_cppstd, cross_building
from conan.tools.gnu import PkgConfigDeps
from conan.tools.meson import MesonToolchain


class MaiserverConan(ConanFile):
    name = 'maiserver'
    requires = (
        'frida-gum/17.18.0',
        'simple-web-server/0.0.0+git.546895a',
        'protobuf/7.35.0',
        'lz4/1.10.0',
        'nlohmann_json/3.12.0',
        'pugixml/1.16',
        'base64/0.5.2',
        'cpr/1.14.2',
        'date/3.0.5',
    )
    tool_requires = (
        'meson/1.10.2',
        'protobuf/7.35.0',
    )
    default_options = {
        'libcurl/*:with_nghttp2': True,
        'libcurl/*:with_ca_fallback': True,
    }

    settings = 'os', 'compiler', 'build_type', 'arch'

    def validate(self):
        check_min_cppstd(self, 23)

    def generate(self):
        pc = PkgConfigDeps(self)
        pc.generate()
        meson = MesonToolchain(self)
        if cross_building(self):
            meson.binaries['pkg-config'] = 'pkg-config'
        meson.generate()
