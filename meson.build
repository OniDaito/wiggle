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
libcee = dependency('cee')
glfw = dependency('glfw3')
postgres = dependency('libpqxx')
nlopt = dependency('nlopt')

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
  'src/lib/pipe.cpp',
  ],
  dependencies : [libcee, imagine, glfw],
  include_directories : include_dirs,
  link_args : '-lpthread',
)

# Single pair
executable('single',
          'src/single.cpp',
          install : false,
          include_directories : include_dirs,
          dependencies : [libcee, imagine],
          link_with : wiggle,
          link_args : '-Wl,-lpthread')

# Find the best worm model
executable('basemodel',
          'src/solver.cpp',
          'src/basemodel.cpp',
          install : false,
          include_directories : include_dirs,
          dependencies : [libcee, postgres, nlopt],
          link_with : wiggle,
          link_args : '-Wl,-lpthread')

# Wiggle
executable('wiggle',
          'src/wiggle.cpp',
          install : false,
          include_directories : include_dirs,
          dependencies : [libcee, imagine],
          link_with : wiggle,
          link_args : '-Wl,-lpthread')

# count
executable('count',
          'src/count.cpp',
          install : false,
          include_directories : include_dirs,
          dependencies : [libcee, imagine],
          link_with : wiggle,
          link_args : '-Wl,-lpthread')

# stack
executable('stack',
          'src/stack.cpp',
          install : false,
          include_directories : include_dirs,
          dependencies : [libcee, imagine],
          link_with : wiggle,
          link_args : '-Wl,-lpthread')

# mask
executable('mask',
          'src/mask.cpp',
          install : false,
          include_directories : include_dirs,
          dependencies : [libcee, imagine],
          link_with : wiggle,
          link_args : '-Wl,-lpthread')

# db
executable('db',
          'src/db.cpp',
          install : false,
          include_directories : include_dirs,
          dependencies : [libcee, postgres],
          link_with : wiggle,
          link_args : '-Wl,-lpthread')

# Tests
test_basic = executable('test_basic',
  'src/test/basic.cpp',
  include_directories : include_dirs,
  dependencies : [libcee, imagine],
  link_with : wiggle)

test_roi = executable('test_roi',
  'src/test/roi.cpp',
  include_directories : include_dirs,
  dependencies : [libcee, imagine],
  link_with : wiggle)

test_rotaug = executable('test_rotaug',
  'src/test/rotaug.cpp',
  include_directories : include_dirs,
  dependencies : [libcee, imagine],
  link_with : wiggle)

test_deconv = executable('test_deconv',
  'src/test/deconv.cpp',
  include_directories : include_dirs,
  dependencies : [libcee, imagine],
  link_with : wiggle)

test_deconv_stack = executable('test_deconv_stack',
  'src/test/deconv_stack.cpp',
  include_directories : include_dirs,
  dependencies : [libcee, imagine],
  link_with : wiggle)

test_basemodel = executable('test_basemodel',
  'src/solver.cpp',
  'src/test/basemodel.cpp',
  include_directories : include_dirs,
  dependencies : [libcee, postgres, nlopt],
  link_with : wiggle)

test('Basic Test', test_basic)
#test('ROI Test', test_roi)