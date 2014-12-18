cc_library(
    name = 'ecfs_thrift',
    srcs = [
    'common_constants.cpp',
    'common_types.cpp',
    'datanode_constants.cpp',
    'datanode_types.cpp',
    'DatanodeThriftService.cpp',
    'namenode_constants.cpp',
    'namenode_types.cpp',
    'NamenodeThriftService.cpp',
    'recovery_constants.cpp',
    'recovery_types.cpp',
    'RecoveryThriftService.cpp',
    ],
    defs = [
    'HAVE_NETINET_IN_H',
    'HAVE_INTTYPES_H',
    ],
    deps = [
    '//thirdparty/thrift:thrift'
    ],
    extra_cppflags = [
    "-Wno-float-equal",
    "-Wno-sign-compare",
    "-Wno-unused",
    "-Wno-return-type",
    ],
    link_all_symbols = True,
)
