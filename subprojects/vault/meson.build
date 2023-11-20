project(
  'vault',
  'c',
  version: '1.0.0',
  license: 'MIT',
  default_options: [
    'default_library=static'
  ],
)

buildtype = 'release'

install_headers('include/vault.h')
inc = include_directories('include')

vault_sources = [
  'source/platforms/linux/vault_linux.c',
  'source/vault.c',
]

vault_lib = library(
  'vault',
  vault_sources,
  dependencies: dependency('threads'),
  include_directories: inc,
  install: true,
)

vault_dep = declare_dependency(
  include_directories: inc, 
  link_with: vault_lib,
)

pkg = import('pkgconfig')
pkg.generate(vault_lib)
