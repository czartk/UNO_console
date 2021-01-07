#ifndef CLASSES_H
#define CLASSES_H

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>   // for std::size_t
#include <windows.h> // for cool card colours
#include "Main.h"

// Class declarations

class Card
{
public:
    enum class CARD_COLOUR
    {
        YELLOW, // Starts from 0
        RED,
        GREEN,
        BLUE,
        SPECIAL,

        MAX_CARD_TYPE // 6 Values total
    };
    enum class CARD_RANK
    {
        ZERO, // only one per colour
        ONE,
        TWO,
        THREE,
        FOUR,
        FIVE,
        SIX,
        SEVEN,
        EIGHT,
        NINE,
        SKIP, // blocks the next player for a turn
        REVERSE, // switches the direction of the game
        PLUS_2,
        PLUS_4,
        WILD,

        MAX_CARD_RANK
    };
private:
    CARD_COLOUR m_colour;
    CARD_RANK m_rank;

    std::string getColourText() const;
    std::string getRankText() const;
public:
    // Constructors
    Card() = default;
    Card(CARD_COLOUR colour, CARD_RANK rank);
    Card(int colour, int rank);

    // Overloads
    friend std::ostream& operator<< (std::ostream& out, const Card& card);

    // Other
    void print() const;

    // Setters
    void setCard(CARD_COLOUR colour, CARD_RANK rank);
    void setCard(int colour, int rank);

    // Getters
    CARD_COLOUR getColour() const { return m_colour; }
    CARD_RANK getRank() const { return m_rank; }
};

class Player
{
private:
    int m_id;
    std::string m_name;
    AI_PERSONAS m_personality;
    std::vector<Card> m_playerCards;
    int m_score{ 0 };
    int m_turnsBlocked{ 0 }; // If == -1 (or any other number < 0) the ai player will always be skipped

public:
    // Constructors
    Player() : m_id{ 0 }, m_name{ "Player" }, m_personality{ AI_PERSONAS::HUMAN }, m_playerCards(30), m_score{ 0 }, m_turnsBlocked{ 0 } // Default constructor
    { m_playerCards.resize(0); /* Capacity stays 30, size changed to 0 */ }

    Player(int id, const std::string& name, AI_PERSONAS personality, int vectorLength, int score, int turnsBlocked);
    Player(int id, const std::string& name, int personality, int vectorLength, int score, int turnsBlocked);

    // Overloads
    friend std::ostream& operator<< (std::ostream& out, const Player& player);

    // Other
    void sortCards();
    void ai_removeMaxCards();

    // Getters
    AI_PERSONAS getPersona() const { return m_personality; }
    const std::string& getName() const { return m_name; }
    int getID() const { return m_id; }
    int getTurnsBlocked() const { return m_turnsBlocked; }
    std::size_t getVectorSize() const { return m_playerCards.size(); }
    Card getCard(std::size_t x) const { return m_playerCards[x]; }
    Card getCard(int x) const { return m_playerCards[x]; }
    Card getLastCard() const { return m_playerCards.back(); }
    Card& getCardRef(int x) { return m_playerCards[x]; }
    Card& getLastCardRef() { return m_playerCards.back(); }

    // Setters
    void setPlayer(int id, const std::string& name, AI_PERSONAS personality);
    void setPlayer(int id, const std::string& name, int personality);
    void push_card(Card x) { m_playerCards.push_back(x); }
    void pop_card() { m_playerCards.pop_back(); }
    void addTurnsBlocked(int x) { m_turnsBlocked += x; }
    void endGame() { m_turnsBlocked = -1; }

    // Other Functions
    void print() const;
    void printCards() const;
};

#endif
