// CMakeEncrypt.cpp: 定义应用程序的入口点。
//

#include <vector>
#include <string>
#include "com_me_study_javaCore_jni_NativeEncryptUtils.h"
#include <curl/curl.h>

using namespace std;


// 定义常量值用于按位异或
const jbyte XOR_VALUE = 19;


size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

//该函数头复制于 com_me_study_javaCore_jni_NativeEncryptUtils.h
JNIEXPORT jbyteArray JNICALL Java_com_me_study_javaCore_jni_NativeEncryptUtils_decrypt
(JNIEnv *env, jclass clazz, jbyteArray inputArray) {
	CURL* curl;
	CURLcode res;
	std::string readBuffer;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://gitee.com/uahioacnao/tvs/raw/master/key.txt");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			printf("HTTPS 异常");
			return nullptr; // 返回 null
		}
		else {
			// 使用 printf 打印 readBuffer 的内容
			printf("%s\n", readBuffer.c_str());
		}
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();

	// 获取输入字节数组的长度
	jsize length = env->GetArrayLength(inputArray);
	if (length <= 0) {
		return nullptr; // 如果输入数组长度为0，返回null
	}

	// 创建一个新的字节数组用于存储结果
	jbyteArray resultArray = env->NewByteArray(length);
	if (resultArray == nullptr) {
		return nullptr; // 内存分配失败时返回 null
	}

	// 获取输入字节数组的内容
	jbyte *inputBytes = env->GetByteArrayElements(inputArray, nullptr);
	if (inputBytes == nullptr) {
		env->DeleteLocalRef(resultArray); // 清理分配的结果数组
		return nullptr; // 获取失败时返回 null
	}

	// 使用 std::vector 来存储结果，避免手动内存管理
	vector<jbyte> resultBytes(length);

	// 进行按位异或操作
	for (jsize i = 0; i < length; i++) {
		resultBytes[i] = inputBytes[i] ^ XOR_VALUE;
	}

	// 将结果写入到新创建的字节数组中
	env->SetByteArrayRegion(resultArray, 0, length, resultBytes.data());

	// 释放资源
	env->ReleaseByteArrayElements(inputArray, inputBytes, JNI_ABORT);

	// 打印调试信息
	printf("this is C++ print\n");

	// 返回结果数组
	return resultArray;
}
