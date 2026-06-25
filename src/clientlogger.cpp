#include "clientlogger.h"

#include <QDir>
#include <QDateTime>
#include <QDebug>

/**
 * @brief ClientLogger构造函数，用于初始化类内私有属性
 */
ClientLogger::ClientLogger()
    : logFile(nullptr)
    , logStream(nullptr)
    , maxLogSize(10 * 1024 * 1024)
    , currentLogSize(0)
    , currentLogLevel(LogLevel::INFO)
{
}

/**
 * @brief ClientLogger析构函数，用于释放动态分配的资源
 */
ClientLogger::~ClientLogger(){
    if(logStream){
        logStream->flush();  // 将内容强制写入文件，并清空缓冲区
        delete logStream;
        logStream = nullptr;
    }
    if(logFile){
        if(logFile->isOpen()){  // 如果文件打开
            logFile->close();  // 关闭文件
        }
        delete logFile;
        logFile = nullptr;
    }
}

/**
 * @brief 获取ClientLogger单例实例
 * @return ClientLogger实例引用
 */
ClientLogger& ClientLogger::GetInstance(){
    static ClientLogger instance;
    return instance;
}

/**
 * @brief 初始化日志系统
 * @param logDir 日志目录路径
 */
void ClientLogger::InitLogger(const QString& logDir){
    QMutexLocker locker(&mutex);  // 加锁限制共享资源被同时访问
    logDirPath = logDir;
    QDir dir(logDirPath);
    if(!dir.exists()){  // 如果目录不存在
        dir.mkpath(".");  // 创建目录
    }
    QString dateStr = QDateTime::currentDateTime().toString("yyyy-MM-dd");  // 获取当前日期字符串
    logFilePath = logDirPath + "/LiteChat_" + dateStr + ".log";
    if(logFile){
        if(logFile->isOpen()){  // 如果文件打开
            logFile->close();  // 关闭文件
        }
        delete logFile;
        logFile = nullptr;
    }
    if(logStream){
        delete logStream;
        logStream = nullptr;
    }
    logFile = new QFile(logFilePath);
    logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);  // 打开文件，以追加模式写入文本内容
    logStream = new QTextStream(logFile);
    currentLogSize = logFile->size();  // 获取当前日志文件大小
    QString initMsg = QString(
        "[%1] [INFO] [ClientLogger] 日志系统初始化完成，日志文件: %2"
    ).arg(GetTimestamp()).arg(logFilePath);  // 格式化日志消息
    *logStream << initMsg << "\n";  // 写入日志消息
    logStream->flush();  // 将内容强制写入文件，并清空缓冲区
    currentLogSize += initMsg.toUtf8().size() + 1;  // 更新当前日志文件大小
    if(currentLogSize >= maxLogSize){
        RotateLogFile();
    }
}

/**
 * @brief 写入日志
 * @param level 日志级别
 * @param module 模块名称
 * @param message 日志消息
 */
void ClientLogger::WriteLog(LogLevel level, const QString& module, const QString& message){
    if(level < currentLogLevel){
        return;
    }
    QMutexLocker locker(&mutex);
    if(!logStream || !logFile || !logFile->isOpen()){  // 如果日志流/文件不存在/未打开
        return;
    }
    QString filteredMsg = FilterSensitiveInfo(message);
    QString timestamp = GetTimestamp();
    QString levelStr = LogLevelToString(level);
    QString logEntry = QString(
        "[%1] [%2] [%3] %4"
    ).arg(timestamp).arg(levelStr).arg(module).arg(filteredMsg);  // 格式化日志消息
    *logStream << logEntry << "\n";
    logStream->flush();  // 将内容强制写入文件，并清空缓冲区
    currentLogSize += logEntry.toUtf8().size() + 1;  // 更新当前日志文件大小
    if(currentLogSize >= maxLogSize){
        RotateLogFile();
    }
}

/**
 * @brief 设置日志级别
 * @param level 日志级别
 */
void ClientLogger::SetLogLevel(LogLevel level){
    QMutexLocker locker(&mutex);
    currentLogLevel = level;
}

/**
 * @brief 获取当前日志级别
 * @return 当前日志级别
 */
LogLevel ClientLogger::GetLogLevel() const{
    return currentLogLevel;
}

/**
 * @brief 日志文件更迭
 */
void ClientLogger::RotateLogFile(){
    if(!logStream || !logFile){
        return;
    }
    logStream->flush();  // 将内容强制写入文件，并清空缓冲区
    logFile->close();  // 关闭文件
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HHmmss");  // 获取当前时间戳字符串
    QString backupPath = logFilePath + "." + timestamp + ".bak";
    QFile::rename(logFilePath, backupPath);  // 重命名为备份文件
    logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);  // 打开文件，以追加模式写入文本内容
    logStream->setDevice(logFile);  // 重新设置日志流设备
    currentLogSize = 0;
    QString rotateMsg = QString(
        "[%1] [INFO] [ClientLogger] 日志文件更迭完成，备份至: %2"
    ).arg(GetTimestamp()).arg(backupPath);  // 格式化日志消息
    *logStream << rotateMsg << "\n";
    logStream->flush();
    currentLogSize += rotateMsg.toUtf8().size() + 1;  // 更新当前日志文件大小
}

/**
 * @brief 获取当前时间戳字符串
 * @return 格式化的时间戳字符串（yyyy-MM-dd HH:mm:ss.zzz）
 */
QString ClientLogger::GetTimestamp() const{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");  // 获取当前时间戳字符串
    return timestamp;
}

/**
 * @brief 过滤敏感信息（密码字段）
 * @param message 原始消息
 * @return 过滤后的消息
 */
QString ClientLogger::FilterSensitiveInfo(const QString& message) const{
    QString result = message;
    int pwdIdx = result.indexOf("\"password\"");  // 查找密码字段索引
    if(pwdIdx != -1){
        int colonIdx = result.indexOf(":", pwdIdx);  // 查找冒号索引
        if(colonIdx != -1){
            int startQuote = result.indexOf("\"", colonIdx + 1);  // 查找引号索引
            int endQuote = result.indexOf("\"", startQuote + 1);  // 查找引引号索引
            if(startQuote != -1 && endQuote != -1 && endQuote > startQuote){
                result.replace(startQuote + 1, endQuote - startQuote - 1, "***");  // 替换密码字段为 "***"
            }
        }
    }
    return result;
}

/**
 * @brief 将日志级别枚举转换为字符串
 * @param level 日志级别
 * @return 日志级别字符串
 */
QString ClientLogger::LogLevelToString(LogLevel level) const{
    switch(level){
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARN";
        case LogLevel::ERROR:
            return "ERROR";
    }
    return "UNKNOWN";
}