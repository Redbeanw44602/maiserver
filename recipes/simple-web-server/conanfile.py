import os

from conan import ConanFile
from conan.tools.files import copy
from conan.tools.scm import Git


class SimpleWebServerRecipe(ConanFile):
    name = 'simple-web-server'
    version = '0.0.0+git.546895a'

    package_type = 'header-library'

    def requirements(self):
        self.requires('asio/1.38.2')

    def source(self):
        commit_id = self.conan_data['sources'][self.version]['commit_id']

        git = Git(self)
        git.clone(url='https://gitlab.com/eidheim/Simple-Web-Server.git', target='.')
        git.checkout(commit=commit_id)

    def package(self):
        copy(
            self,
            'LICENSE',
            self.source_folder,
            os.path.join(self.package_folder, 'licenses'),
        )
        copy(
            self,
            '*.hpp',
            self.source_folder,
            os.path.join(self.package_folder, 'include', 'simple-web-server'),
        )

    def package_info(self):
        self.cpp_info.defines.append('USE_STANDALONE_ASIO')

        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
