from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan import ConanFile, Version
import os

#OSMI add algo_minis

class Package(ConanFile):
    name = "oxce-plus"
    settings = "os", "build_type", "compiler", "arch"

    revision_mode = "scm"
    scm = {
        "type": "git",
        "url": "auto",
        "revision": "auto",
    }

    def requirements(self):
        pass

    def configure(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self,)

    def generate(self):
        deps = CMakeDeps(self)

        deps.generate()

        toolchain = CMakeToolchain(self)
        toolchain.generate()

    def build(self):
        self.output.info("Building using {}".format(self.settings.compiler))
        cmake = CMake(self)

        if self.should_configure:
            cmake.configure()

        if self.should_build:
            cmake.build()

        # if self.should_test:
        #     cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.install()
