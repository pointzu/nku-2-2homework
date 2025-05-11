#include "SoundManager.h"

SoundManager* SoundManager::instance() {
    static SoundManager mgr;
    return &mgr;
}

SoundManager::SoundManager(QObject* parent) : QObject(parent) {}

void SoundManager::playEffect(const QString& resourcePath, qreal volume) {
    if (!m_effects.contains(resourcePath)) {
        QSoundEffect* effect = new QSoundEffect(this);
        effect->setSource(QUrl(resourcePath));
        effect->setVolume(volume);
        m_effects[resourcePath] = effect;
    }
    auto effect = m_effects[resourcePath];
    if (effect) {
        effect->setVolume(volume);
        effect->play();
    }
}
