
cc_library(
    name = 'datanode.a',
    srcs = [
    'chunk_manager.cpp',
    'datanode_thrift_service.cpp',
    'data_node.cpp',
    'disk_manager.cpp',
    'heartbeat_manager.cpp',
    ],
    defs = [
    'HAVE_NETINET_IN_H',
    ],

    deps = [
    '//ecfs/src/gen-cpp:ecfs_thrift',
    '//ecfs/src/common:ecfs_common',
    '//thirdparty/fecpp:fecpp',
    ],
    
    extra_cppflags = [
    '-Wno-unused-variable',
    '-Wno-float-equal',
    '-Wno-return-type',
    '-Wno-vla',
    ], 
)
cc_binary(
    name = 'ecfs_datanode',
    srcs = 'datanode_main.cpp',
    deps = [
    ':datanode.a',
    ],
    defs = [
    'HAVE_NETINET_IN_H',
    ],
    extra_cppflags = [
    '-Wno-unused-variable',
    '-Wno-float-equal',
    '-Wno-return-type',
    '-Wno-vla',
    ], 
)
gen_rule(
    name = 'datanode_deploy',
    cmd  = 'cp -f $SRCS $OUTS',
    deps = [':ecfs_datanode'],
    srcs = ['ecfs_datanode'],
    outs = ['../../../../ecfs/bin/ecfs_datanode'],
)
