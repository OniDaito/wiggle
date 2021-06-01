# Project details
project('wiggle', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++17'])

# Debug or release
if get_option('buildtype') == 'debug'
  add_global_arguments('-DDEBUG', language : 'cpp')
endif

masamune = dependency('masamune')

# The include directories for the dependencies inside this project
include_dirs = [
  include_directories('include'),
]


# Solange
executable('solange',
          'src/solange.cc',
          install : false,
          include_directories : include_dirs,
          dependencies : [masamune])

# flatten
executable('flatten',
          'src/flatten.cc',
          install : false,
          include_directories : include_dirs,
          dependencies : [masamune])