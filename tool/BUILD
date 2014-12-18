cc_binary(
    name = 'ecfs_format',
    srcs = 'ecfs_format.cpp',
    defs = [
    'HAVE_NETINET_IN_H',
    ],
    deps = [
    '//ecfs/src/common:ecfs_common',
    ],
    extra_cppflags = [
    '-Wno-unused-variable',
    '-Wno-float-equal',
    '-Wno-return-type',
    '-Wno-vla',
    ], 
)
gen_rule(
    name = 'format_deploy',
    cmd  = 'cp -f $SRCS $OUTS',
    deps = [':ecfs_format'],
    srcs = ['ecfs_format'],
    outs = ['../../../../ecfs/bin/ecfs_format'],
)

cc_binary(
    name = 'ecfs_put',
    srcs = 'ecfs_put.cpp',
    defs = [
    'HAVE_NETINET_IN_H',
    ],
    deps = [
    '//ecfs/src/common:ecfs_common',
    '//ecfs/src/client:ecfs_client',
    ],
    extra_cppflags = [
    '-Wno-unused-variable',
    '-Wno-float-equal',
    '-Wno-return-type',
    '-Wno-vla',
    ], 
)
gen_rule(
    name = 'put_deploy',
    cmd  = 'cp -f $SRCS $OUTS',
    deps = [':ecfs_put'],
    srcs = ['ecfs_put'],
    outs = ['../../../../ecfs/bin/ecfs_put'],
)