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

dependencies = [
  argparse,
  websocket,
  asio,
  json
]

server = executable('voxelracing-server',
  ['src/main.cpp'],
  dependencies : dependencies,
  include_directories : include_directories('src/'),
  cpp_args : common_cpp_args,
  override_options: override_args,
)
