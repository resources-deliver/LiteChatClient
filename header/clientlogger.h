#ifndef CLIENTLOGGER_H
#define CLIENTLOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>

/**
 * @brief 日志级别枚举
 */
enum class LogLevel{DEBUG, INFO, WARNING, ERROR,};

/**
 * @brief 客户端日志记录器类，负责日志写入、轮转和敏感信息过滤
 */
class ClientLogger{
public:
    static ClientLogger& GetInstance();
    void InitLogger(const QString& logDir);
    void WriteLog(LogLevel level, const QString& module, const QString& message);
    void SetLogLevel(LogLevel level);
    LogLevel GetLogLevel() const;

private:
    ClientLogger();
    ~ClientLogger();
    ClientLogger(const ClientLogger&) = delete;
    ClientLogger& operator=(const ClientLogger&) = delete;
    void RotateLogFile();
    QString GetTimestamp() const;
    QString FilterSensitiveInfo(const QString& message) const;
    QString LogLevelToString(LogLevel level) const;

private:
    QString logDirPath;  // 日志目录路径
    QString logFilePath;  // 日志文件路径
    QFile* logFile;  // 日志文件对象指针
    QTextStream* logStream;  // 日志文本流指针
    qint64 maxLogSize;  // 日志文件大小阈值（字节）
    qint64 currentLogSize;  // 当前日志文件大小
    LogLevel currentLogLevel;  // 当前日志级别
    QMutex mutex;  // 互斥锁，保证线程安全
};

#endif // CLIENTLOGGER_H