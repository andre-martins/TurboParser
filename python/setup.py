from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext
from Cython.Build import cythonize
import os

if os.name == 'nt':
    ext_modules=[Extension("turboparser",
    ["turboparser.pyx"],
    language="c++",
    extra_compile_args=["/Zi", "/Od", "/DGOOGLE_GLOG_DLL_DECL=", "/DGFLAGS_DLL_DECL="],
    extra_link_args=['/DEBUG'],
    include_dirs=["..\\src\\util",
    "..\\src\\classifier",
    "..\\src\\sequence",
    "..\\src\\entity_recognizer",
    "..\\src\\morphological_tagger",
    "..\\src\\parser",
    "..\\src\\tagger",
    "..\\src\\semantic_parser",
    "..\\src\\coreference_resolver",
    "..\\deps\\AD3-2.0.2\\ad3",
    "..\\deps\\AD3-2.0.2",
    "..\\deps\\glog-0.3.2\\src\\windows",
    "..\\deps\\gflags-2.0\\src\\windows",
    "..\\deps\\eigen-eigen-c58038c56923",
    "..\\deps\\googletest\\src"],
    library_dirs=["..\\vsprojects\\x64\\Release",    
    "..\\deps\\glog-0.3.2\\x64\\Release",            
    "..\\deps\\gflags-2.0\\x64\\Release",            
    "..\\deps\\AD3-2.0.2\\vsprojects\\x64\\Release", 
    "..\\deps\\googletest\\msvc\\x64\\Release"],
    extra_objects=["libturboparser.lib", 
    "AD3_140mdx64.lib",                 
    "libgflags_140mdx64.lib",           
    "libglog_static_140mdx64.lib",      
    "gtest-md_140mdx64.lib"])]
    setup(cmdclass={'build_ext': build_ext},
    ext_modules = cythonize(ext_modules, gdb_debug=True)
    )
else:
    ext_modules=[Extension("turboparser", 
    ["turbo_parser.pyx"],
    language="c++",
    extra_compile_args=["-std=c++0x"],
    include_dirs=["../src/morphological_tagger", 
    "../src/coreference_resolver", 
    "../src/semantic_parser", 
    "../src/parser", 
    "../src/entity_recognizer/", 
    "../src/tagger/", 
    "../src/sequence/", 
    "../src/classifier/", 
    "../src/util", 
    "../deps/local/include/"],
    library_dirs=["../libturboparser/", 
    "../deps/local/lib/"],
    libraries=["turboparser",
    "gflags", 
    "glog", 
    "ad3"])]
    setup(cmdclass={'build_ext': build_ext},
    ext_modules = ext_modules)
