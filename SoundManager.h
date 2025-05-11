#pragma once
#include <QObject>      // Qt对象基类
#include <QSoundEffect> // Qt音效类
#include <QMap>         // Qt字典
#include <QString>      // Qt字符串
#include <QPointer>     // Qt智能指针

// 音效管理器单例类，负责全局音效播放
class SoundManager : public QObject {
    Q_OBJECT
public:
    static SoundManager* instance(); // 获取单例实例
    void playEffect(const QString& resourcePath, qreal volume = 1.0); // 播放音效，支持音量
    void playSoundEffect(const QString& resourcePath); // 播放音效（默认音量）
private:
    explicit SoundManager(QObject* parent = nullptr); // 构造函数（私有）
    QMap<QString, QPointer<QSoundEffect>> m_effects; // 音效缓存字典
};
