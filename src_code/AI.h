#ifndef AI_H
#define AI_H

#include "Classes.h"

void AITurn(Player& playerVar, const std::vector<Player>& players, std::vector<Card>& playedCards, std::vector<Card>& deck, Card& playedCardVar, Card::CARD_COLOUR& chosenColour, int& playerCount,
    const int maxPlayerIndex, bool& gameDirection, int& takeCards, int& turnsBlocked, int& aiPlayersWon, int& time_AIActions);

struct AI_info_t
{
    std::vector<int> s_indexesToPlay;
    std::vector<Card> s_correspondingCards;
    int AI_decisionPoints{ 0 };
};

Card::CARD_COLOUR AI_decideChosenColour(const Player& playerVar, const std::vector<Player>& players);
void AI_CounterSkipCard(Player& playerVar, std::vector<Card>& playedCards, int& turnsBlocked, int time_AIActions);
void AI_CounterPlusCard(const std::vector<Player>& players, Player& playerVar, std::vector<Card>& playedCards, int& takeCards, Card::CARD_COLOUR& chosenColour, int time_AIActions);

#endif
