#include "audio/AudioManager.hpp"
#include <iostream>

AudioManager& AudioManager::get() {
    static AudioManager instance;
    return instance;
}

AudioManager::AudioManager() {
    m_menuMusic   = new sf::Music();
    m_normalMusic = new sf::Music();
    m_bossMusic   = new sf::Music();

    m_menuLoaded   = tryOpen(m_menuMusic,   "assets/sounds/Kyrie.ogg");
    m_normalLoaded = tryOpen(m_normalMusic, "assets/sounds/pixel_play.ogg");
    m_bossLoaded   = tryOpen(m_bossMusic,   "assets/sounds/goated.wav");

    if (m_menuLoaded)   { m_menuMusic->setLooping(true);   m_menuMusic->setVolume(m_musicVolume); }
    if (m_normalLoaded) { m_normalMusic->setLooping(true); m_normalMusic->setVolume(m_musicVolume); }
    if (m_bossLoaded)   { m_bossMusic->setLooping(true);   m_bossMusic->setVolume(m_musicVolume); }

    // SFX — short files, load fully into buffer
    if (m_attackBuf.loadFromFile("assets/sounds/attack.ogg")) {
        m_attackLoaded = true;
        m_attackSound  = new sf::Sound(m_attackBuf);
        m_attackSound->setLooping(true);
        m_attackSound->setVolume(m_sfxVolume);
    } else {
        std::cerr << "[AudioManager] Could not load attack.ogg\n";
        m_attackSound = new sf::Sound(m_attackBuf); // safe empty sound
    }

    if (m_deathBuf.loadFromFile("assets/sounds/death.ogg")) {
        m_deathLoaded = true;
        m_deathSound  = new sf::Sound(m_deathBuf);
        m_deathSound->setLooping(false);
        m_deathSound->setVolume(m_sfxVolume);
    } else {
        std::cerr << "[AudioManager] Could not load death.ogg\n";
        m_deathSound = new sf::Sound(m_deathBuf);
    }
}

AudioManager::~AudioManager() {
    stopAllMusic();
    if (m_attackSound) { m_attackSound->stop(); delete m_attackSound; }
    if (m_deathSound)  { m_deathSound->stop();  delete m_deathSound;  }
    delete m_menuMusic;
    delete m_normalMusic;
    delete m_bossMusic;
}

bool AudioManager::tryOpen(sf::Music* music, const std::string& path) {
    if (!music->openFromFile(path)) {
        std::cerr << "[AudioManager] Could not open: " << path << "\n";
        return false;
    }
    return true;
}

void AudioManager::stopAllMusic() {
    if (m_menuLoaded   && m_menuMusic)   m_menuMusic->stop();
    if (m_normalLoaded && m_normalMusic) m_normalMusic->stop();
    if (m_bossLoaded   && m_bossMusic)   m_bossMusic->stop();
}

void AudioManager::playMenuMusic() {
    if (m_currentTrack == Track::Menu) return;
    stopAllMusic();
    if (m_menuLoaded && m_menuMusic) m_menuMusic->play();
    m_currentTrack = Track::Menu;
}

void AudioManager::playNormalLevelMusic() {
    if (m_currentTrack == Track::Normal) return;
    stopAllMusic();
    if (m_normalLoaded && m_normalMusic) m_normalMusic->play();
    m_currentTrack = Track::Normal;
}

void AudioManager::playBossLevelMusic() {
    if (m_currentTrack == Track::Boss) return;
    stopAllMusic();
    if (m_bossLoaded && m_bossMusic) m_bossMusic->play();
    m_currentTrack = Track::Boss;
}

void AudioManager::stopMusic() {
    stopAllMusic();
    m_currentTrack = Track::None;
}

void AudioManager::setMusicVolume(float v) {
    m_musicVolume = v;
    if (m_menuLoaded)   m_menuMusic->setVolume(v);
    if (m_normalLoaded) m_normalMusic->setVolume(v);
    if (m_bossLoaded)   m_bossMusic->setVolume(v);
}

// ── SFX ───────────────────────────────────────────────────────────────────

void AudioManager::setAttackPlaying(bool playing) {
    if (!m_attackLoaded || !m_attackSound) return;
    if (playing && !m_attackPlaying) {
        m_attackSound->play();
        m_attackPlaying = true;
    } else if (!playing && m_attackPlaying) {
        m_attackSound->stop();
        m_attackPlaying = false;
    }
}

void AudioManager::playDeathSound() {
    if (!m_deathLoaded || !m_deathSound) return;
    m_deathSound->stop();   // restart if already playing
    m_deathSound->play();
}

void AudioManager::setSFXVolume(float v) {
    m_sfxVolume = v;
    if (m_attackSound) m_attackSound->setVolume(v);
    if (m_deathSound)  m_deathSound->setVolume(v);
}