VariantDir('bin', 'src', duplicate=0)

env = Environment()
env['LIBS'] = ['pthread']
env['LIBPATH'] = "/usr/lib/"
env['CXXFLAGS'] = "-O0 -g -std=c++11 -Wall -Wfatal-errors -pedantic"
env['CPPPATH'] = "include"

 
env.Program('bin/main.cpp')
