project(
  'hoverclock',
  'c',
  version: '1.0.0',
  license: 'MIT',
  default_options: [
    'warning_level=3',
    'buildtype=release',
  ],
)

hoverclock_dependencies = [
  dependency('xcb'),
  dependency('xcb-shape'),
  dependency('fontconfig'),
  dependency('freetype2'),
  subproject('vault').get_variable('vault_dep'),
  subproject('insight').get_variable('insight_dep'),
  subproject('inthreadval').get_variable('inthreadval_dep'),
]

hoverclock_sources = [
  'source/display_server.c',
  'source/settings.c',
  'source/surface.c',
  'source/window.c',
  'source/label.c',
  'source/color.c',
  'source/main.c',
]

hoverclock_executable = executable(
  'hoverclock',
  hoverclock_sources,
  dependencies: hoverclock_dependencies,
  install: true,
  install_dir: '/usr/bin',
)
