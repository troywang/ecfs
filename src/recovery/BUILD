cc_library(
    name = 'recovery.a',
    srcs = [
    	'recovery_node.cpp',
        'recovery_thrift_service.cpp',
    ],
    extra_cppflags = [
    '-Wno-unused-variable',
    '-Wno-float-equal',
    '-Wno-return-type',
    #'-Wno-vla',
    ], 
)

cc_binary(
    name = 'ecfs_recovery',
    srcs = [
    'recovery_main.cpp',
    ],
    defs = [
    'HAVE_NETINET_IN_H',
    ],

    deps = [
    ':recovery.a',
    '//ecfs/src/gen-cpp:ecfs_thrift',
    '//ecfs/src/common:ecfs_common',
    '//thirdparty/fecpp:fecpp',
    ],
    
    extra_cppflags = [
    '-Wno-unused-variable',
    '-Wno-float-equal',
    '-Wno-return-type',
    #'-Wno-vla',
    ], 
)

gen_rule(
    name = 'recovery_deploy',
    cmd  = 'cp -f $SRCS $OUTS',
    deps = [':ecfs_recovery'],
    srcs = ['ecfs_recovery'],
    outs = ['../../../../ecfs/bin/ecfs_recovery'],
)
