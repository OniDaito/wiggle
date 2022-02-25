# Project details
project('wiggle', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++17'])

# Debug or release
if get_option('buildtype') == 'debug'
  add_global_arguments('-DDEBUG', language : 'cpp')
endif

masamune = dependency('masamune')
cfits = dependency('cfitsio')
glfw = dependency('glfw3')

# The include directories for the dependencies inside this project
include_dirs = [
  include_directories('include'),
]


# dataset
executable('dataset',
          'src/image.cpp',
          'src/data.cpp',
          'src/dataset.cpp',
          install : false,
          include_directories : include_dirs,
          dependencies : [masamune, cfits])

# multiset
executable('multiset',
          'src/roi.cpp',
          'src/image.cpp',
          'src/data.cpp',
          'src/multiset.cpp',
          install : false,
          include_directories : include_dirs,
          dependencies : [masamune, cfits])

# volume
executable('volume',
          'src/roi.cpp',
          'src/image.cpp',
          'src/data.cpp',
          'src/rots.cpp',
          'src/volume.cpp',
          install : false,
          include_directories : include_dirs,
          dependencies : [masamune, cfits, glfw])

# flatten
executable('flatten',
          'src/flatten.cpp',
          install : false,
          include_directories : include_dirs,
          dependencies : [masamune, cfits])

# stack
executable('stack',
          'src/stack.cpp',
          install : false,
          include_directories : include_dirs,
          dependencies : [masamune, cfits])

executable('fluro',
          'src/fluro.cpp',
          install : false,
          include_directories : include_dirs,
          dependencies : [masamune])

# Tests
test_basic = executable('test_basic',
  'src/test/basic.cpp',
  include_directories : include_dirs,
  dependencies : [masamune, cfits])

test_roi = executable('test_roi',
  'src/image.cpp',
  'src/data.cpp',
  'src/roi.cpp',
  'src/test/roi.cpp',
  include_directories : include_dirs,
  dependencies : [masamune, cfits])

test_rotaug = executable('test_rotaug',
  'src/roi.cpp',
  'src/image.cpp',
  'src/data.cpp',
  'src/rots.cpp',
  'src/rots.cpp',
  'src/test/rotaug.cpp',
  include_directories : include_dirs,
  dependencies : [masamune, cfits])

test('Basic Test', test_basic)
#test('ROI Test', test_roi)