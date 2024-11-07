rm -rf build # 删除build目录的所有内容
cmake -S . -B build # 在当前源代码目录构建build
cmake --build build --target all -- -j$(nproc) # 用4个并进任务构建目标

echo "编译完成！"
echo ""