from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

src = "../libturboparser/"

setup(cmdclass={'build_ext': build_ext},
    ext_modules=[Extension("turboparser", ["turbo_parser.pyx"], language="c++",
    include_dirs=["../src/semantic_parser", "../src/parser", "../src/entity_recognizer/", "../src/tagger/", "../src/sequence/", "../src/classifier/", "../src/util", "../deps/local/include/"],
    library_dirs=[src, "../deps/local/lib/"], libraries=["turboparser", "gflags", "glog", "ad3"])])
