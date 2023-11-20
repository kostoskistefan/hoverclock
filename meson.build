project(
  'hoverclock',
  'c',
  version: '1.0.0',
  license: 'MIT'
)

hoverclock_dependencies = [
  dependency('xcb'),
  dependency('xcb-shape'),
  dependency('fontconfig'),
  dependency('freetype2'),
  dependency('vault', fallback: ['vault', 'vault_dep']),
  dependency('insight', fallback: ['insight', 'insight_dep']),
  dependency('inthreadval', fallback: ['inthreadval', 'inthreadval_dep']),
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
)
