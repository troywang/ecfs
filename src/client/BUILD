cc_library(
    name = 'ecfs_client',
    srcs = [
    'ecfs_client.cpp',
    ],
    defs = [
    'HAVE_NETINET_IN_H',
    ],

    deps = [
     '//ecfs/src/gen-cpp:ecfs_thrift',
     '//ecfs/src/common:ecfs_common',
     '//thirdparty/fecpp:fecpp',
#    '//thirdparty/glog:glog',
#    '//thirdparty/boost:filesystem',
#    '//thirdparty/boost:system',
#    '//thirdparty/boost:thread',
#    '//thirdparty/boost:date_time',
    ],
    
    extra_cppflags = [
    '-Wno-unused-variable',
    '-Wno-float-equal',
    '-Wno-return-type',
    #'-Wno-vla',
    ], 
)
