project(
  'inthreadval',
  'c',
  version: '1.0.0',
  license: 'MIT',
  default_options: [
    'default_library=static'
  ],
)

buildtype = 'release'

install_headers('include/inthreadval.h')
inc = include_directories('include')

inthreadval_lib = library(
  'inthreadval',
  'source/inthreadval.c',
  dependencies: dependency('threads'),
  include_directories: inc,
  install: true,
)

inthreadval_dep = declare_dependency(
  include_directories: inc, 
  link_with: inthreadval_lib,
)

pkg = import('pkgconfig')
pkg.generate(inthreadval_lib)
