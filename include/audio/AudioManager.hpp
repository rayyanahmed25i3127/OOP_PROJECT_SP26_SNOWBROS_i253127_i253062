#pragma once

#include <SFML/Audio.hpp>
#include <string>

/**
 * @brief Global audio controller — Singleton pattern.
 *
 * Handles background music across the entire game. Because it's a Singleton,
 * any state can access it via AudioManager::get() without needing a reference
 * passed through constructors.
 *
 * Music continuity: since AudioManager lives outside any state, transitioning
 * Menu -> Leaderboard -> Pause keeps the menu music playing seamlessly. Only
 * PlayState calls playGameMusic() to switch tracks.
 *
 * Satisfies "Design Pattern" requirement from spec section 11.1.
 *
 * Uses sf::Music (streams from disk) rather than sf::Sound (loads fully into
 * memory) because music files are large and streaming is the standard choice.
 */
class AudioManager {
public:
    // Access the single global instance (lazy-initialized on first call).
    static AudioManager& get();

    // Disable copy/move — singletons must not be duplicated.
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    // Music tracks
    void playMenuMusic();    // Menu, Leaderboard, Pause — all share this
    void playGameMusic();    // PlayState only
    void stopMusic();

    // Volume control (0.0 to 100.0 in SFML convention)
    void setMusicVolume(float volume);

private:
    // Private constructor — nobody can create one from outside.
    AudioManager();

    // sf::Music is NOT copyable, so we use raw pointers + new/delete.
    // We hold two separate Music objects — one for menu, one for gameplay —
    // so switching between them is instant (no re-loading from disk).
    sf::Music* m_menuMusic;
    sf::Music* m_gameMusic;

    // Track whether each track successfully loaded, so we don't call
    // setLooping/setVolume/play on unloaded Music objects (which asserts on Windows).
    bool m_menuLoaded;
    bool m_gameLoaded;

    // Track which one is currently playing, so we don't restart it
    // if the same music is requested again.
    enum class CurrentTrack { None, Menu, Game };
    CurrentTrack m_current;

    float m_volume;

    // Helper — tries to load a file into the given Music*; logs on failure.
    // Returns true if loaded successfully.
    bool tryOpen(sf::Music* music, const std::string& filepath);

public:
    // Destructor must be public for the static instance to be destroyed cleanly
    ~AudioManager();
};