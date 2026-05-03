#pragma once
#include <SFML/Audio.hpp>
#include <string>

//controlls all the sound ( music + sfx like levels ounds and hit sound effects etc)
class AudioManager {
public:
    static AudioManager& get();
    AudioManager(const AudioManager&)            = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    ~AudioManager();

    //music
    void playMenuMusic();
    void playNormalLevelMusic();
    void playBossLevelMusic();
    void stopMusic();
    void setMusicVolume(float v);

//sfx
    void setAttackPlaying(bool playing);
    void playDeathSound();
    void setSFXVolume(float v);

private:
    AudioManager();
    bool tryOpen(sf::Music* m, const std::string& path);
    void stopAllMusic();

    sf::Music* m_menuMusic   = nullptr;
    sf::Music* m_normalMusic = nullptr;
    sf::Music* m_bossMusic   = nullptr;
    bool m_menuLoaded   = false;
    bool m_normalLoaded = false;
    bool m_bossLoaded   = false;

    enum class Track { None, Menu, Normal, Boss };
    Track m_currentTrack = Track::None;
    float m_musicVolume  = 50.f;

    sf::SoundBuffer m_attackBuf;
    sf::Sound*      m_attackSound  = nullptr;
    bool            m_attackLoaded  = false;
    bool            m_attackPlaying = false;

    sf::SoundBuffer m_deathBuf;
    sf::Sound*      m_deathSound = nullptr;
    bool            m_deathLoaded = false;

    float m_sfxVolume = 70.f;
};