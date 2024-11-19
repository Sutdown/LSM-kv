rm -rf build # 删除build目录的所有内容

start=`date +%s`
cmake -S . -B build # 在当前源代码目录构建build
cmake --build build --target all -- -j$(nproc) # 用4个并进任务构建目标
end=`date +%s`
time=`echo $start $end | awk '{print $2-$1}'`

echo "---------------------------------------"
echo "编译完成！编译总耗时(cmake+make)：${time}s"
echo "---------------------------------------"