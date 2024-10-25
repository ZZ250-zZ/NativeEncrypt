// CMakeEncrypt.cpp: 定义应用程序的入口点。
//

#include <vector>
#include <string>
#include "com_me_study_javaCore_jni_NativeEncryptUtils.h"
#include <curl/curl.h>
#include <fstream>
#include <stdexcept>
#include <sstream>  // for std::istringstream
#include <iomanip>  // for std::get_time
#include <ctime>

#if defined(_WIN32) || defined(_WIN64)
    #include <direct.h>  // Windows下使用_getcwd
    #define getcwd _getcwd
    #define PATH_MAX _MAX_PATH
#else
    #include <unistd.h>  // POSIX API
    #include <limits.h> // PATH_MAX
#endif

using namespace std;


// 定义常量值用于按位异或
const jbyte XOR_VALUE = 19;

// 读取文件首行内容
std::string readFile(const std::string& filename) {
    // 获取当前目录的完整路径
    char currentPath[PATH_MAX];
    if (getcwd(currentPath, sizeof(currentPath)) != nullptr) {
        // printf("Current directory: %s\n", currentPath);
    } else {
        throw std::runtime_error("无法确定当前目录");
    }

    // 打开并读取指定文件的内容
    std::string fullPath = std::string(currentPath) + "/" + filename;
    std::ifstream file(fullPath);
    if (!file) {
        printf("文件不存在：%s\n", fullPath.c_str());
        throw std::runtime_error("文件不存在：" + fullPath);
    }

    std::string line;
    if (!std::getline(file, line)) {
        throw std::runtime_error(fullPath + " is empty");
    }
    file.close();

    return line;
}

// 从 curl 中获取响应
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

// GET 请求获取远程内容
std::string curlGet(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            throw std::runtime_error("HTTPS error");
        }
        else {
            // 使用 printf 打印 readBuffer 的内容
            //printf("get：%s\n", readBuffer.c_str());
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return readBuffer;
}

// 把 c++异常转换成 java Rutime
void throwException(JNIEnv* env, const char* className, const char* message) {
    // 捕获标准异常并转换为 JNI RuntimeException
    jclass runtimeExceptionClass = env->FindClass(className);
    if (runtimeExceptionClass != nullptr) {
        env->ThrowNew(runtimeExceptionClass, message);
    } else {
        // 找不到 RuntimeException 类，输出错误信息
        printf("%s\n", "Unable to find RuntimeException class");
    }
}

// 解析日期字符串为 struct tm
struct tm parseDate(const std::string& dateStr) {
    struct tm tmStruct = {};
#if defined(_WIN32) || defined(_WIN64)
    std::istringstream ss(dateStr);
    ss >> std::get_time(&tmStruct, "%Y-%m-%d");
    if (ss.fail()) {
        throw std::runtime_error("can't parse date string");
    }
#else
    strptime(dateStr.c_str(), "%Y-%m-%d %H:%M:%S", &tmStruct);
#endif
    return tmStruct;
}


// 比较两个日期
bool isDateBefore(const std::string& date1, const std::string& date2) {
    struct tm tm1 = parseDate(date1);
    struct tm tm2 = parseDate(date2);

    time_t time1 = mktime(&tm1);
    time_t time2 = mktime(&tm2);

    return time1 <= time2;
}

// 获取当前系统日期的函数
std::string getCurrentDate() {
    time_t now = time(0);
    struct tm tstruct;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tstruct, &now);  // Windows 使用 localtime_s
#else
    localtime_r(&now, &tstruct);  // Linux/Unix 使用 localtime_r
#endif
    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return std::string(buf);
}

//该函数头复制于 com_me_study_javaCore_jni_NativeEncryptUtils.h
JNIEXPORT jbyteArray JNICALL Java_com_me_study_javaCore_jni_NativeEncryptUtils_decrypt
(JNIEnv *env, jclass clazz, jbyteArray inputArray) {
    try {
        std::string currentDate = getCurrentDate();
        //printf("currentDate：%s\n", currentDate.c_str());
        struct tm tm2 = parseDate(currentDate);
        time_t time2 = mktime(&tm2);

        std::string url = "https://gitee.com/uahioacnao/jni-key/raw/master/" + readFile("key.conf") + ".txt";
        std::string remoteDate = curlGet(url);
        // 比较时间
        if (isDateBefore(remoteDate, currentDate)) {
            //throw std::runtime_error("您的授权已过期，请联系开发者");
        }
        
        //printf("remoteDate：%s\n", remoteDate.c_str());
        struct tm tm1 = parseDate(remoteDate);
        time_t time1 = mktime(&tm1);
        if (time1 < time2) {
            throw std::runtime_error("Your license has expired");
        } else if (time1 == time2) {
            printf("明天就要过期啦，请及时联系开发者\n");
        }


        // 获取输入字节数组的长度
        jsize length = env->GetArrayLength(inputArray);
        if (length <= 0) {
            // 如果输入数组长度为0，返回null
            throw std::runtime_error("Decryption failed");
        }

        // 创建一个新的字节数组用于存储结果
        jbyteArray resultArray = env->NewByteArray(length);
        if (resultArray == nullptr) {
            // 内存分配失败时返回 null
            throw std::runtime_error("Decryption failed");
        }

        // 获取输入字节数组的内容
        jbyte *inputBytes = env->GetByteArrayElements(inputArray, nullptr);
        if (inputBytes == nullptr) {
            env->DeleteLocalRef(resultArray); // 清理分配的结果数组
            // 获取失败时返回 null
            throw std::runtime_error("Decryption failed");
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

        // 返回结果数组
        return resultArray;
    } catch (const std::exception& e) {
        // 捕获并抛出Java异常
        throwException(env, "java/lang/RuntimeException", e.what());
    }
}
