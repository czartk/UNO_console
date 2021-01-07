#ifndef MAIN_FUNC_H
#define MAIN_FUNC_H

#include "Classes.h"
#include "Main.h"

// Trivial Functions
int getNumOfPlayers();
int getChoice(); // gets the choice for the player's action
int getSortingMethod();
Card::CARD_COLOUR getChosenColour(const Player&, const std::vector<Player>&);
int getCardChoice(const Player& player);
int getCounterCardChoice(const std::vector<int>& cardIndexes, bool playedCard);
bool doesPlayerWantToCounter(Technical Text = Technical::PLAY_COUNTER_TEXT);
std::string getPlayerName();
void printColourName(Card::CARD_COLOUR colour);
void printHumanTurnInfo(const Card& playedCard, const Player& player, Card::CARD_COLOUR chosenColour);

// Effect Functions
void createDeck(std::vector<Card>& deck);
void createPlayers(std::vector<Player>& players);
void giveOutCardsToPlayers(std::vector<Player>& players, std::vector<Card>& deck);
void initializePlayedCards(std::vector<Card>& playedCards, std::vector<Card>& deck);
void checkAvailableAndAddCardsInDeck(std::vector<Card>& playedCards, std::vector<Card>& deck, int numberOfCardsTaken = 1);
void switchGameDirection(bool& gameDirection);
void assignCards(Player& player, std::vector<Card>& playedCards, std::vector<Card>& deck, int cardsToTake = 1);
void nextPlayer(int& playerCount, int maxPlayerIndex, bool gameDirection);
void CounterSkipCard(Player& playerVar, std::vector<Card>& playedCards, int& turnsBlocked);
void CounterPlusCard(Player& playerVar, std::vector<Card>& playedCards, int& takeCards, Card::CARD_COLOUR& chosenColour);
void playCard(Player& playerVar, std::vector<Card>& playedCards, const Card& chosenCard, int chosenCardIndex,
    bool& gameDirection, Card::CARD_COLOUR& chosenColour, int& takeCards, int& turnsBlocked, int time_AIActions);
void changeTime_AIActions(const std::vector<Player>& players, int& time_AIActions);

// Boolean functions
bool canCounterSkipCard(const Player& player);
bool canCounterPlusCards(const Player& player, const Card& playedCard, Card::CARD_COLOUR chosenColour); // This function assumes that playedCard is +4 or +2
bool isAbleToPlayACard(const Player& player, const Card& playedCard, Card::CARD_COLOUR chosenColour); // Looks thru all of the player's cards
bool isAbleToPlayMoreCards(const Player& player, const Card& playedCard); // Looks thru all of the player's cards
bool isAbleToPlayChosenCard(const Card& chosenCard, const Card& playedCard, Card::CARD_COLOUR chosenColour);
bool isAbleToPlayMoreChosenCard(const Card& chosenCard, const Card& playedCard, Card::CARD_COLOUR);
bool isAbleToCounterPlusWithChosenCard(const Card& chosenCard, const Card& playedCard, Card::CARD_COLOUR chosenColour);

// Game function
bool playUNO();

#endif
