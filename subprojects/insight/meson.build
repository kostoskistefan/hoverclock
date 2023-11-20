project(
  'insight',
  'c',
  version: '1.0.0',
  license: 'MIT',
  default_options: [
    'default_library=static'
  ],
)

buildtype = 'release'

install_headers('include/insight.h')
inc = include_directories('include')

insight_lib = library(
  'insight',
  'source/insight.c',
  include_directories: inc,
  install: true,
)

insight_dep = declare_dependency(
  include_directories: inc, 
  link_with: insight_lib,
)

pkg = import('pkgconfig')
pkg.generate(insight_lib)
