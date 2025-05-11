#pragma once
#include <QObject>
#include <QSoundEffect>
#include <QMap>
#include <QString>
#include <QPointer>

class SoundManager : public QObject {
    Q_OBJECT
public:
    static SoundManager* instance();
    void playEffect(const QString& resourcePath, qreal volume = 1.0);
    void playSoundEffect(const QString& resourcePath);
private:
    explicit SoundManager(QObject* parent = nullptr);
    QMap<QString, QPointer<QSoundEffect>> m_effects;
};
