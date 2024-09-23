JNI 中的 native 代码部分。

只需要修改并 push javah 生成的头文件、C++ 实现代码，Github Actions 会自动编译 dll 和 so 库。