from conan import ConanFile
from conan.tools.files import get, copy
from conan.errors import ConanInvalidConfiguration
import os


class FridaGumRecipe(ConanFile):
    name = 'frida-gum'
    version = '17.18.0'

    settings = 'os', 'arch'

    def validate(self):
        if self.settings.os not in ['Linux']:
            raise ConanInvalidConfiguration('unsupported os')
        if self.settings.arch not in ['x86_64', 'armv8']:
            raise ConanInvalidConfiguration('unsupported arch')

    def build(self):
        _os = {'Linux': 'linux'}[str(self.settings.os)]
        _arch = {'x86_64': 'x86_64', 'armv8': 'arm64'}[str(self.settings.arch)]
        _version = self.version

        get(
            self,
            f'https://github.com/frida/frida/releases/download/{_version}/frida-gum-devkit-{_version}-{_os}-{_arch}.tar.xz',
        )

    def package(self):
        copy(self, '*.h', self.build_folder, os.path.join(self.package_folder, 'include'))
        copy(self, '*.a', self.build_folder, os.path.join(self.package_folder, 'lib'))

    def package_info(self):
        self.cpp_info.libs = ['frida-gum']
