#include "states/LeaderboardState.hpp"
#include "states/StateManager.hpp"
#include <iostream>
#include <cmath>
#include <string>

//  Constructor / Destructor
LeaderboardState::LeaderboardState()
    : bgSprite(nullptr), backText(nullptr)
{}

LeaderboardState::~LeaderboardState()
{
    delete bgSprite;   // The destructor must delete the two heap-allocated objects so we don't leak.
    delete backText;
}


// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
//  drawCapsule
//
//  Entire capsule = ONE TriangleFan polygon â†’ no overlapping sub-shapes,
//  uniform alpha, zero seam.
//
//  Vertex count per capsule:
//      1            centroid
//    + (SEGS + 1)   right semicircle
//    + (SEGS + 1)   left  semicircle
//    + 1            closing repeat of first perimeter vertex
//    = 2 * SEGS + 4
//  SEGS = 28  â†’  60 vertices  â†’  array size 64 (safe margin).
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void LeaderboardState::drawCapsule(sf::RenderWindow& window,
                                   float x, float y,
                                   float width, float height,
                                   sf::Color fillColor,
                                   sf::Color outlineColor,
                                   float     outlineThickness)
{
    static const int SEGS      = 28;
    static const int MAX_VERTS = 2 * SEGS + 4;   // 60 â€” fits in 64-slot array

    // â”€â”€ Fills a plain sf::Vertex array for one capsule.
    //    Returns the number of vertices written.
    auto buildCapsule = [&](float bx, float by,
                             float bw, float bh,
                             sf::Color col,
                             sf::Vertex out[]) -> int
    {
        float r    = bh / 2.f;
        float lcx  = bx + r;          // left  semicircle centre X
        float rcx  = bx + bw - r;     // right semicircle centre X
        float midY = by + r;           // shared centre Y

        int idx = 0;

        // Centroid â€” first vertex of TriangleFan
        out[idx++] = {{ bx + bw / 2.f, midY }, col};

        // Right semicircle: -Ï€/2 â†’ +Ï€/2
        for (int i = 0; i <= SEGS; ++i)
        {
            float angle = -3.14159265f / 2.f
                        +  3.14159265f * static_cast<float>(i) / SEGS;
            out[idx++] = {{ rcx + r * std::cos(angle),
                            midY + r * std::sin(angle) }, col};
        }

        // Left semicircle: +Ï€/2 â†’ +3Ï€/2
        for (int i = 0; i <= SEGS; ++i)
        {
            float angle = 3.14159265f / 2.f
                        +  3.14159265f * static_cast<float>(i) / SEGS;
            out[idx++] = {{ lcx + r * std::cos(angle),
                            midY + r * std::sin(angle) }, col};
        }

        // Close the fan â€” repeat the first perimeter vertex
        out[idx++] = out[1];

        return idx;
    };

    sf::Vertex verts[MAX_VERTS];
    int        count = 0;

    // 1. Outline â€” draw a slightly expanded capsule in outlineColor first
    if (outlineThickness > 0.f && outlineColor.a > 0)
    {
        float ot = outlineThickness;
        count = buildCapsule(x  - ot,
                             y  - ot,
                             width  + 2.f * ot,
                             height + 2.f * ot,
                             outlineColor,
                             verts);
        window.draw(verts,
                    static_cast<std::size_t>(count),
                    sf::PrimitiveType::TriangleFan);
    }

    // 2. Fill â€” draw the actual capsule on top
    count = buildCapsule(x, y, width, height, fillColor, verts);
    window.draw(verts,
                static_cast<std::size_t>(count),
                sf::PrimitiveType::TriangleFan);
}


// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
//  onEnter
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void LeaderboardState::onEnter()
{
    // â”€â”€ Background texture + sprite â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    if (!bgTexture.loadFromFile("assets/sprites/leaderboard_bg.png"))
        std::cerr << "BG failed\n";

    // Delete any previous sprite before creating a new one
    delete bgSprite;
    bgSprite = new sf::Sprite(bgTexture);

    auto size = bgTexture.getSize();
    if (size.x != 0 && size.y != 0)
        bgSprite->setScale({ 800.f / static_cast<float>(size.x),
                             600.f / static_cast<float>(size.y) });

    // â”€â”€ Font â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    if (!font.openFromFile("assets/fonts/BubbleBobble-rg3rx.ttf"))
        std::cerr << "Font failed\n";

    // â”€â”€ Back button label â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    delete backText;
    backText = new sf::Text(font);
    backText->setString("BACK");
    backText->setCharacterSize(30);
    backText->setFillColor(sf::Color::White);

    // Centre the text inside the capsule
    sf::FloatRect bounds = backText->getLocalBounds();
    backText->setOrigin({ bounds.position.x + bounds.size.x / 2.f,
                          bounds.position.y + bounds.size.y / 2.f });
    backText->setPosition({ 400.f, 545.f });

    // â”€â”€ Leaderboard data â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    leaderboard.loadFromFile("leaderboard.txt");
}


// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
//  handleEvent
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void LeaderboardState::handleEvent(const sf::Event& event)
{
    sf::FloatRect backBounds({ 310.f, 520.f }, { 180.f, 50.f });

    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>())
    {
        sf::Vector2f mousePos(static_cast<float>(mouse->position.x),
                              static_cast<float>(mouse->position.y));
        if (backBounds.contains(mousePos))
            m_manager->popState();
    }

    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        if (key->code == sf::Keyboard::Key::Escape)
            m_manager->popState();
    }
}


// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
//  update â€” track hover over back button
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void LeaderboardState::update(float /*dt*/)
{
    sf::Vector2i pixelPos = sf::Mouse::getPosition();
sf::Vector2f mousePos(static_cast<float>(pixelPos.x),
                      static_cast<float>(pixelPos.y));

    sf::FloatRect backBounds({ 310.f, 520.f }, { 180.f, 50.f });
    isBackHovered = backBounds.contains(mousePos);
}


// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
//  draw
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void LeaderboardState::draw(sf::RenderWindow& window)
{
    // â”€â”€ Background â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    if (bgSprite)
        window.draw(*bgSprite);

    // â”€â”€ Column X positions â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    const float startX      = 50.f;
    const float colRank     = startX;
    const float colName     = startX + 70.f;
    const float colScore    = startX + 300.f;
    const float colDate     = startX + 520.f;

    const float bubbleX     = 40.f;
    const float bubbleWidth = 720.f;
    const float bubbleH     = 40.f;

    // â”€â”€ Leaderboard rows â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    const LeaderboardEntry* e     = leaderboard.getEntries();
    int                     total = leaderboard.getCount();

    //Mlf:(Headers)
    //  HEADER Y POSITION (above first row)
float headerY = 40.f;

// ðŸ”¹ Rank Header
sf::Text rankHeader(font);
rankHeader.setString("Rank");
rankHeader.setCharacterSize(28);
rankHeader.setFillColor(sf::Color::White);
rankHeader.setStyle(sf::Text::Bold);
rankHeader.setPosition({colRank, headerY});

// ðŸ”¹ Name Header
sf::Text nameHeader(font);
nameHeader.setString("Name");
nameHeader.setCharacterSize(28);
nameHeader.setFillColor(sf::Color::White);
nameHeader.setStyle(sf::Text::Bold);
nameHeader.setPosition({colName, headerY});

// ðŸ”¹ Score Header
sf::Text scoreHeader(font);
scoreHeader.setString("Score");
scoreHeader.setCharacterSize(28);
scoreHeader.setFillColor(sf::Color::White);
scoreHeader.setStyle(sf::Text::Bold);
scoreHeader.setPosition({colScore, headerY});

// ðŸ”¹ Date Header
sf::Text dateHeader(font);
dateHeader.setString("Date");
dateHeader.setCharacterSize(28);
dateHeader.setFillColor(sf::Color::White);
dateHeader.setStyle(sf::Text::Bold);
dateHeader.setPosition({colDate, headerY});

// ðŸ”¹ DRAW HEADERS
window.draw(rankHeader);
window.draw(nameHeader);
window.draw(scoreHeader);
window.draw(dateHeader);

//headers

    for (int i = 0; i < 10; ++i)
    {
        float y = 80.f + i * 45.f;

        // â”€â”€ Gold / Silver / Bronze capsule with black outline â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        if (i < 3)
        {
            sf::Color fillColor;
            if      (i == 0) fillColor = sf::Color(255, 215,   0, 190); // gold
            else if (i == 1) fillColor = sf::Color(192, 192, 192, 190); // silver
            else             fillColor = sf::Color(205, 127,  50, 190); // bronze

            drawCapsule(window,
                        bubbleX, y, bubbleWidth, bubbleH,
                        fillColor,
                        sf::Color(0, 0, 0, 230),  // black outline
                        3.f);
        }

        // â”€â”€ Text colour logic â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        //    Top-3 with real data  â†’ black  (readable on coloured capsule)
        //    Rows 4-10 with data   â†’ white
        //    Any empty placeholder â†’ dim grey
        bool exists = (i < total);

        sf::Color textColor;
        if      (!exists)         textColor = sf::Color(180, 180, 180); // dim grey
        else if (i < 3)           textColor = sf::Color::Black;
        else                      textColor = sf::Color::White;

        // â”€â”€ Build and draw the four text fields â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        std::string rankStr  = std::to_string(i + 1) + ".";
        std::string nameStr  = exists ? e[i].playerName            : "---";
        std::string scoreStr = exists ? std::to_string(e[i].score) : "---";
        std::string dateStr  = exists ? e[i].date                  : "---";

        sf::Text rankText(font);
        rankText.setString(rankStr);
        rankText.setCharacterSize(26);
        rankText.setFillColor(textColor);
        rankText.setPosition({ colRank, y });

        sf::Text nameText(font);
        nameText.setString(nameStr);
        nameText.setCharacterSize(26);
        nameText.setFillColor(textColor);
        nameText.setPosition({ colName, y });

        sf::Text scoreText(font);
        scoreText.setString(scoreStr);
        scoreText.setCharacterSize(26);
        scoreText.setFillColor(textColor);
        scoreText.setPosition({ colScore, y });

        sf::Text dateText(font);
        dateText.setString(dateStr);
        dateText.setCharacterSize(26);
        dateText.setFillColor(textColor);
        dateText.setPosition({ colDate, y });

        window.draw(rankText);
        window.draw(nameText);
        window.draw(scoreText);
        window.draw(dateText);
    }

    // â”€â”€ Back button capsule â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    sf::Color backFill = isBackHovered
                       ? sf::Color( 60,  60,  60, 220)  // dark-grey on hover
                       : sf::Color(  0,   0,   0, 200); // near-black normally

    drawCapsule(window,
                310.f, 520.f, 180.f, 50.f,
                backFill,
                sf::Color(255, 255, 255, 220),  // white outline
                3.f);

    // Back label â€” white normally, warm cream on hover
    if (backText)
    {
        backText->setFillColor(isBackHovered
                               ? sf::Color(255, 255, 180)
                               : sf::Color::White);
        window.draw(*backText);
    }
}