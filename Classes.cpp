#include <algorithm> // used for std::swap()
#include "Classes.h"
#include "loc_eng.h"
#include "Main_Func.h"

//// Member Functions

// Card

Card::Card(CARD_COLOUR colour, CARD_RANK rank)
    : m_colour{ colour }, m_rank{ rank }
{}
Card::Card(int colour, int rank)
    : m_colour{ static_cast<CARD_COLOUR>(colour) }, m_rank{ static_cast<CARD_RANK>(rank) }
{}

std::string Card::getColourText() const
{
    switch (m_colour)
    {
    case CARD_COLOUR::YELLOW:
        return "Yellow";
    case CARD_COLOUR::RED:
        return "Red";
    case CARD_COLOUR::GREEN:
        return "Green";
    case CARD_COLOUR::BLUE:
        return "Blue";
    case CARD_COLOUR::SPECIAL:
        return "All-colour";
    default:
        return "???";
    }
}

std::string Card::getRankText() const
{
    switch (m_rank)
    {
    case CARD_RANK::ONE:
        return "1";
    case CARD_RANK::TWO:
        return "2";
    case CARD_RANK::THREE:
        return "3";
    case CARD_RANK::FOUR:
        return "4";
    case CARD_RANK::FIVE:
        return "5";
    case CARD_RANK::SIX:
        return "6";
    case CARD_RANK::SEVEN:
        return "7";
    case CARD_RANK::EIGHT:
        return "8";
    case CARD_RANK::NINE:
        return "9";
    case CARD_RANK::ZERO:
        return "0";
    case CARD_RANK::SKIP:
        return "Skip";
    case CARD_RANK::REVERSE:
        return "Reverse";
    case CARD_RANK::PLUS_2:
        return "+2";
    case CARD_RANK::PLUS_4:
        return "+4";
    case CARD_RANK::WILD:
        return "Wild";
    default:
        return "???";
    }
}

void Card::print() const
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); // Does something that makes the colours work
    switch (m_colour)
    {
    case CARD_COLOUR::YELLOW:
        SetConsoleTextAttribute(hConsole, 6);
        break;
    case CARD_COLOUR::RED:
        SetConsoleTextAttribute(hConsole, 12);
        break;
    case CARD_COLOUR::GREEN:
        SetConsoleTextAttribute(hConsole, 10);
        break;
    case CARD_COLOUR::BLUE:
        SetConsoleTextAttribute(hConsole, 3);
        break;
    case CARD_COLOUR::SPECIAL:
        SetConsoleTextAttribute(hConsole, 13);
        break;
    }
    std::cout << getColourText() << ' ' << getRankText() << '\n';
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Set the colour back to white
}

void Card::setCard(CARD_COLOUR colour, CARD_RANK rank)
{
    m_colour = colour;
    m_rank = rank;
}

void Card::setCard(int colour, int rank)
{
    m_colour = static_cast<CARD_COLOUR>(colour);
    m_rank = static_cast<CARD_RANK>(rank);
}

// Player

// Can't define default parameters twice, that's why i can't define the default constructor here

Player::Player(int id, const std::string& name, AI_PERSONAS personality, int vectorLength = 30, int score = 0, int turnsBlocked = 0)
    : m_id{ id }, m_name{ name }, m_personality{ personality }, m_playerCards(vectorLength), m_score{ score }, m_turnsBlocked{ turnsBlocked }
{
    m_playerCards.resize(0); // Capacity stays 30, size changed to 0
}

Player::Player(int id, const std::string& name, int personality, int vectorLength = 30, int score = 0, int turnsBlocked = 0)
    : m_id{ id }, m_name{ name }, m_personality{ static_cast<AI_PERSONAS>(personality) }, m_playerCards(vectorLength), m_score{ score }, m_turnsBlocked{ turnsBlocked }
{
    m_playerCards.resize(0); // Capacity stays 30, size changed to 0
}

void Player::sortCards()
{
    switch (getSortingMethod())
    {
    case 1: // Sort by colour
        std::sort(m_playerCards.begin(), m_playerCards.end(),
            [](const Card& a, const Card& b) -> bool
            {
                if (a.getColour() < b.getColour())
                    return true;
                return false;
            });
        break;
    case 2: // Sort by symbol
        std::sort(m_playerCards.begin(), m_playerCards.end(),
            [](const Card& a, const Card& b) -> bool
            {
                if (a.getRank() < b.getRank())
                    return true;
                return false;
            });
        break;
    }

    std::cout << loc_cardsSorted;
}

void Player::ai_removeMaxCards()
{
    for (int index{ 0 }, vecEnd{ static_cast<int>(m_playerCards.size()) }; index < vecEnd; ++index)
    {
        if (m_playerCards[index].getRank() == Card::CARD_RANK::MAX_CARD_RANK)
        {
            // Remove the card
            std::swap(m_playerCards[index], m_playerCards.back());
            m_playerCards.pop_back();
            // Reset the loop
            index = 0;
            // vecEnd must be resetted every time the size changes
            vecEnd = static_cast<int>(m_playerCards.size());
        }
    }
}

void Player::setPlayer(int id, const std::string& name, AI_PERSONAS personality)
{
    m_id = id;
    m_name = name;
    m_personality = personality;
}
void Player::setPlayer(int id, const std::string& name, int personality)
{
    m_id = id;
    m_name = name;
    m_personality = static_cast<AI_PERSONAS>(personality);
}

void Player::print() const
{
    std::cout << m_name << " has the id: " << m_id << " and personality: " << static_cast<int>(m_personality) << " and cards: ";
    for (const auto& element : m_playerCards)
        element.print();
}

void Player::printCards() const
{
    for (const auto& element : m_playerCards)
        element.print();
}

//// Overloads

// Card

std::ostream& operator<< (std::ostream& out, const Card& card)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); // Does something that makes the colours work
    switch (card.m_colour)
    {
    case Card::CARD_COLOUR::YELLOW:
        SetConsoleTextAttribute(hConsole, 6);
        break;
    case Card::CARD_COLOUR::RED:
        SetConsoleTextAttribute(hConsole, 12);
        break;
    case Card::CARD_COLOUR::GREEN:
        SetConsoleTextAttribute(hConsole, 10);
        break;
    case Card::CARD_COLOUR::BLUE:
        SetConsoleTextAttribute(hConsole, 3);
        break;
    case Card::CARD_COLOUR::SPECIAL:
        SetConsoleTextAttribute(hConsole, 13);
        break;
    }
    out << card.getColourText() << ' ' << card.getRankText() << '\n';
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Set the colour back to white

    return out;
}

// Player

std::ostream& operator<<(std::ostream& out, const Player& player)
{
    int x{ 1 }; // Shows the card's place, purely cosmetic
    for (const auto& element : player.m_playerCards)
    {
        out << '(' << x << ')' << ' ' << element;
        ++x;
    }

    return out;
}