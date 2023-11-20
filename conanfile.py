from conan import ConanFile
from conan.tools.cmake import CMake

class Conan(ConanFile):
    name = "StochasticPackageQuery"
    version = "1.0.0"

    # Define your package's settings and options if necessary
    settings = "os", "compiler", "build_type", "arch"

    # Define your package requirements
    def requirements(self):
        self.requires("boost/1.83.0")
        self.requires("highs/1.6.0")
        self.requires("libpq/15.4")
        self.requires("fmt/10.1.1")
        self.requires("xtensor/0.24.7")
        # self.requires("quantlib/1.30")

    # Define your generators
    generators = "CMakeDeps", "CMakeToolchain"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()