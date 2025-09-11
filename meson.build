project('voxel-racing', 'cpp', version : '0.1.0', meson_version : '>=1.0.0')

if host_machine.system() == 'windows'
 override_args = ['cpp_std=vc++latest']
 common_cpp_args =  ['-D_WIN32_WINNT=0x0601']
else
 override_args = ['cpp_std=c++23']
 common_cpp_args = []
endif

common_cpp_args += '-DASIO_STANDALONE'

argparse = dependency('argparse')
websocket = dependency('websocketpp')
asio = dependency('asio')
json = dependency('nlohmann_json')
gtest = dependency('gtest')

dependencies = [
  argparse,
  websocket,
  asio,
  json
]

libvoxelracing = static_library('voxelracing',
  ['src/lib.cpp'],
  dependencies : dependencies,
  include_directories : include_directories('include/voxelracing/', 'src/'),
  cpp_args : common_cpp_args,
  override_options: override_args,
)

server = executable('voxelracing-server',
  ['src/bin/main.cpp'],
  dependencies : dependencies,
  include_directories : include_directories('include/'),
  cpp_args : common_cpp_args,
  override_options: override_args,
  link_with : libvoxelracing,
)

test_basics = executable('test_basics',
  ['src/tests/basics.cpp'],
  dependencies: dependencies + gtest,
  include_directories : include_directories('include/'),
  cpp_args : common_cpp_args,
  override_options: override_args,
  link_with : libvoxelracing,
)

test('basics', test_basics)
