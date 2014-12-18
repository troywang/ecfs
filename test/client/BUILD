cc_test(
    name = 'unittest_client',
    srcs = [
    'ecfs_client_test.cpp',
    ],
    defs = [
    'HAVE_NETINET_IN_H'
    ],
    deps = [
    '//ecfs/test/core:gtest-main',
    '//ecfs/test/common:unittest_common',
    '//ecfs/src/common:ecfs_common',
    '//ecfs/src/client:ecfs_client',
    '//ecfs/src/datanode:ecfs_datanode',
    '//ecfs/src/namenode:ecfs_namenode',
    '//ecfs/src/recovery:ecfs_recovery',
    '//ecfs/src/gen-cpp:ecfs_thrift',
    ],
)