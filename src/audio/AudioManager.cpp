#include "audio/AudioManager.hpp"
#include <iostream>

AudioManager& AudioManager::get() {
    // Meyers Singleton — thread-safe since C++11, no manual cleanup needed.
    // The instance is constructed on first call, destroyed at program exit.
    static AudioManager instance;
    return instance;
}

AudioManager::AudioManager()
    : m_menuMusic(nullptr)
    , m_gameMusic(nullptr)
    , m_menuLoaded(false)
    , m_gameLoaded(false)
    , m_current(CurrentTrack::None)
    , m_volume(50.f)
{
    m_menuMusic = new sf::Music();
    m_gameMusic = new sf::Music();

    // Track whether each file loaded successfully.
    // Calling setLooping/setVolume on an unloaded Music asserts in SFML 3 on Windows,
    // so we only configure tracks that actually loaded.
    m_menuLoaded = tryOpen(m_menuMusic, "assets/sounds/menu_music.ogg");
    m_gameLoaded = tryOpen(m_gameMusic, "assets/sounds/game_music.ogg");

    if (m_menuLoaded) {
        m_menuMusic->setLooping(true);
        m_menuMusic->setVolume(m_volume);
    }
    if (m_gameLoaded) {
        m_gameMusic->setLooping(true);
        m_gameMusic->setVolume(m_volume);
    }
}

AudioManager::~AudioManager() {
    if (m_menuMusic) {
        m_menuMusic->stop();
        delete m_menuMusic;
        m_menuMusic = nullptr;
    }
    if (m_gameMusic) {
        m_gameMusic->stop();
        delete m_gameMusic;
        m_gameMusic = nullptr;
    }
}

bool AudioManager::tryOpen(sf::Music* music, const std::string& filepath) {
    // SFML 3: openFromFile (was openFromFile in SFML 2 too, but different
    // return handling). Returns bool directly in SFML 3.
    if (!music->openFromFile(filepath)) {
        std::cerr << "[AudioManager] Could not open: " << filepath
                  << " (placeholder — drop the real file here later)\n";
        return false;
    }
    return true;
}

void AudioManager::playMenuMusic() {
    if (m_current == CurrentTrack::Menu) return;

    if (m_current == CurrentTrack::Game && m_gameLoaded && m_gameMusic) {
        m_gameMusic->stop();
    }

    if (m_menuLoaded && m_menuMusic
        && m_menuMusic->getStatus() != sf::Music::Status::Playing) {
        m_menuMusic->play();
    }
    m_current = CurrentTrack::Menu;
}

void AudioManager::playGameMusic() {
    if (m_current == CurrentTrack::Game) return;

    if (m_current == CurrentTrack::Menu && m_menuLoaded && m_menuMusic) {
        m_menuMusic->stop();
    }

    if (m_gameLoaded && m_gameMusic
        && m_gameMusic->getStatus() != sf::Music::Status::Playing) {
        m_gameMusic->play();
    }
    m_current = CurrentTrack::Game;
}

void AudioManager::stopMusic() {
    if (m_menuLoaded && m_menuMusic) m_menuMusic->stop();
    if (m_gameLoaded && m_gameMusic) m_gameMusic->stop();
    m_current = CurrentTrack::None;
}

void AudioManager::setMusicVolume(float volume) {
    m_volume = volume;
    if (m_menuLoaded && m_menuMusic) m_menuMusic->setVolume(volume);
    if (m_gameLoaded && m_gameMusic) m_gameMusic->setVolume(volume);
}