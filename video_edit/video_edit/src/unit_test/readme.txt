gtest单测框架使用

【参考文档】
https://blog.csdn.net/u013366022/article/details/81166643
https://www.cnblogs.com/taukeWang/p/6265458.html
http://blog.sina.com.cn/s/blog_7e4ac8b501018b27.html

【单测示例】

video_edit/video_edit/src/unit_test
video_edit/video_edit/src/unit_test/jni

【测试素材】
video_edit/video_edit/src/unit_test/test_resources
android单测时拷贝素材到/sdcard/video里

【生成测试覆盖率报告步骤】

step1:
进入单测jni目录，执行ndk-build,生成可执行文件vetest

step2:

可执行文件到测试机上
adb push obj/local/armeabi-v7a/vetest /data/local/tmp

step3：
设置生成数据目录、在测试机上运行可执行文件
adb shell GCOV_PREFIX=/data/local/tmp/ /data/local/tmp/vetest

step4:
拷贝生成的中间文件到ndk-build生成的obj目录
adb pull /data/local/tmp/Users/yanjia/workspace/video_edit/video_edit/src/unit_test/obj obj

step5:
生成info文件
lcov -d . -t "vetest" -o 'vetest.info' -b . -c

step6:
过滤
 lcov --remove vetest.info '/Users/yanjia/workspace/video_edit/ext/include/*' '/Users/yanjia/workspace/video_edit/ffmpeg/*' '/Users/yanjia/workspace/video_edit/video_edit/codec/*' '*include*' '*include/bits*' '*include/ext*' '*af_afade.c' '*video_edit/src/unit_test/*' -o vetest3.info
 
step6：
用genhtml生成html测试覆盖率报告
genhtml vetest3.info --no-branch-coverage -t "veLibTest" -o coverage