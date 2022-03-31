# Project details
project('wiggle', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++17'])

# Debug or release
if get_option('buildtype') == 'debug'
  add_global_arguments('-DDEBUG', language : 'cpp')
endif

if get_option('buildtype') == 'release'
  add_global_arguments('-O3', language : 'cpp')
endif

project_dir  = meson.current_source_dir() # MESON_SOURCE_ROOT
compiler = meson.get_compiler('cpp')

imagine = dependency('imagine')
libsee = dependency('libsee')
cfits = dependency('cfitsio')
glfw = dependency('glfw3')

# The include directories for the dependencies inside this project
include_dirs = [
  include_directories('include'),
  include_directories('external/glm'),
]

external_libs = join_paths(project_dir, './external/')

wiggle = library('wiggle', sources : [
  'src/lib/image.cpp',
  'src/lib/data.cpp',
  'src/lib/roi.cpp',
  'src/lib/rots.cpp',
  ],
  dependencies : [libsee, imagine, cfits, glfw],
  include_directories : include_dirs,
  link_args : '-lpthread',
)


# dataset
executable('dataset',
          'src/dataset.cpp',
          install : false,
          include_directories : include_dirs,
          dependencies : [libsee, imagine, cfits],
          link_with : wiggle)

# multiset
executable('multiset',
          'src/multiset.cpp',
          install : false,
          include_directories : include_dirs,
          dependencies : [libsee, imagine, cfits],
          link_with : [wiggle],
          link_args : '-Wl,-lstdc++fs,-lpthread')

# volume
executable('volume',
          'src/volume.cpp',
          install : false,
          include_directories : include_dirs,
          dependencies : [libsee, imagine, cfits, glfw],
          link_with : wiggle)

# flatten
executable('flatten',
          'src/flatten.cpp',
          install : false,
          include_directories : include_dirs,
          dependencies : [libsee, imagine, cfits],
          link_with : wiggle)

executable('fluro',
          'src/fluro.cpp',
          install : false,
          include_directories : include_dirs,
          dependencies : [libsee, imagine],
          link_with : wiggle)

# Tests
test_basic = executable('test_basic',
  'src/test/basic.cpp',
  include_directories : include_dirs,
  dependencies : [libsee, imagine, cfits],
  link_with : wiggle)

test_roi = executable('test_roi',
  'src/test/roi.cpp',
  include_directories : include_dirs,
  dependencies : [libsee, imagine, cfits],
  link_with : wiggle)

test_rotaug = executable('test_rotaug',
  'src/test/rotaug.cpp',
  include_directories : include_dirs,
  dependencies : [libsee, imagine, cfits],
  link_with : wiggle)

test_deconv = executable('test_deconv',
  'src/test/deconv.cpp',
  include_directories : include_dirs,
  dependencies : [libsee, imagine, cfits],
  link_with : wiggle)

test_deconv_stack = executable('test_deconv_stack',
  'src/test/deconv_stack.cpp',
  include_directories : include_dirs,
  dependencies : [libsee, imagine, cfits],
  link_with : wiggle)

test('Basic Test', test_basic)
#test('ROI Test', test_roi)