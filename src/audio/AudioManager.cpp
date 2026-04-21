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
    , m_current(CurrentTrack::None)
    , m_volume(50.f)
{
    // Allocate both Music objects up front. We keep them alive for the
    // whole program — no re-loading when switching tracks.
    m_menuMusic = new sf::Music();
    m_gameMusic = new sf::Music();

    // Try to load the placeholder files. If they don't exist yet,
    // tryOpen() logs a warning and we continue without music.
    // Once the user drops real files in assets/sounds/, this just works.
    tryOpen(m_menuMusic, "assets/sounds/menu_music.ogg");
    tryOpen(m_gameMusic, "assets/sounds/game_music.ogg");

    // Configure both for looping playback
    m_menuMusic->setLooping(true);
    m_gameMusic->setLooping(true);
    m_menuMusic->setVolume(m_volume);
    m_gameMusic->setVolume(m_volume);
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
    // If menu music is already playing, don't restart it — that's the whole
    // point of this class. Prevents the song from cutting out every time
    // you transition Menu -> Leaderboard -> Menu.
    if (m_current == CurrentTrack::Menu) {
        return;
    }

    // Stop whatever's currently playing
    if (m_current == CurrentTrack::Game && m_gameMusic) {
        m_gameMusic->stop();
    }

    if (m_menuMusic && m_menuMusic->getStatus() != sf::Music::Status::Playing) {
        m_menuMusic->play();  // no-op if the file wasn't loaded
    }
    m_current = CurrentTrack::Menu;
}

void AudioManager::playGameMusic() {
    if (m_current == CurrentTrack::Game) {
        return;
    }

    if (m_current == CurrentTrack::Menu && m_menuMusic) {
        m_menuMusic->stop();
    }

    if (m_gameMusic && m_gameMusic->getStatus() != sf::Music::Status::Playing) {
        m_gameMusic->play();
    }
    m_current = CurrentTrack::Game;
}

void AudioManager::stopMusic() {
    if (m_menuMusic) m_menuMusic->stop();
    if (m_gameMusic) m_gameMusic->stop();
    m_current = CurrentTrack::None;
}

void AudioManager::setMusicVolume(float volume) {
    m_volume = volume;
    if (m_menuMusic) m_menuMusic->setVolume(volume);
    if (m_gameMusic) m_gameMusic->setVolume(volume);
}