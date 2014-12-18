cc_library(
    name = 'namenode.a',
    srcs = [
    	'block_manager.cpp',
    	'clean_manager.cpp',
        'namenode_thrift_service.cpp',
        'name_node.cpp',
        'node_manager.cpp',
        'oplog_manager.cpp',
        'recover_manager.cpp'
    ],
    extra_cppflags = [
    '-Wno-unused-variable',
    '-Wno-float-equal',
    '-Wno-return-type',
    #'-Wno-vla',
    ], 
)

cc_binary(
    name = 'ecfs_namenode',
    srcs = [
    'namenode_main.cpp',
    ],
    defs = [
    'HAVE_NETINET_IN_H',
    ],

    deps = [
    ':namenode.a',
    '//ecfs/src/gen-cpp:ecfs_thrift',
    '//ecfs/src/common:ecfs_common',
    ],
    
    extra_cppflags = [
    '-Wno-unused-variable',
    '-Wno-float-equal',
    '-Wno-return-type',
    #'-Wno-vla',
    ], 
)

gen_rule(
    name = 'namenode_deploy',
    cmd  = 'cp -f $SRCS $OUTS',
    deps = [':ecfs_namenode'],
    srcs = ['ecfs_namenode'],
    outs = ['../../../../ecfs/bin/ecfs_namenode'],
)

