#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <cstdlib>   // for std::size_t
#include <windows.h> // for cool card colours
#include <random>    // used for the mersenne twister
#include <ctime>     // used for std::time
#include <algorithm> // used for std::shuffle
#include <chrono>    // This and
#include <thread>    // this is used to pause code exec for some time
#include "Main.h"    // used mostly for namespace Mersenne and enum declarations
#include "Classes.h" // Contains class declarations
#include "AI.h"      // Contains forward declaration for AITurn()
#include "loc_eng.h" // ENG localization

// These function have been forward declared so that fcnPtr in CounterPlusCard() works
bool isAbleToPlayMoreChosenCard(const Card& chosenCard, const Card& playedCard, Card::CARD_COLOUR);
bool isAbleToCounterPlusWithChosenCard(const Card& chosenCard, const Card& playedCard, Card::CARD_COLOUR chosenColour);

constexpr int g_startingCards{ 7 };
extern const int g_takeCardCount{ 1 };

// These vars specify the time between AI's actions (in microseconds)
const int g_longTime{ 700'000 }; // 0.7s
const int g_mediumTime{ 550'000 }; // 0.55s
const int g_shortTime{ 400'000 }; // 0.4s
extern const int g_aiVictoryAddTime{ 1'000'000 }; // 1s

// Trivial Functions

void pressAnything()
{
    char x{};
    std::cin >> x;

    if (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore(32767, '\n');
    }
    std::cin.ignore(32767, '\n');
}

int getNumOfPlayers()
{
    int x{};
    while (true)
    {
        std::cout << "Please enter the number of players (Min 2, Max 10): ";
        std::cin >> x;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(32767, '\n');
        }
        else if (x >= 2 && x <= 10)
        {
            std::cin.ignore(32767, '\n');
            return x;
        }
        else
        {
            std::cin.ignore(32767, '\n');
            std::cout << "An invalid value was given, please try again.\n";
        }
    }
}

int getChoice() // gets the choice for the player's action
{
    int x{};
    while (true)
    {
        std::cout << "> ";
        std::cin >> x;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(32767, '\n');
        }
        else if (x >= 1 && x <= 5)
        {
            std::cin.ignore(32767, '\n');
            return x;
        }
        else
        {
            std::cin.ignore(32767, '\n');
            std::cout << "An invalid value was given, please try again.\n";
        }
    }
}

int getSortingMethod()
{
    int x{};
    while (true)
    {
        std::cout << "(1) Sort by colour\n(2) Sort by symbol\n> ";
        std::cin >> x;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(32767, '\n');
        }
        else if (x == 1 || x == 2)
        {
            std::cin.ignore(32767, '\n');
            return x;
        }
        else
        {
            std::cin.ignore(32767, '\n');
            std::cout << "An invalid value was given, please try again.\n";
        }
    }
}

char getYN()
{
    char input{};
    while (true)
    {
        std::cout << "> ";
        std::cin >> input;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(32767, '\n');
        }
        else if (input == 'y' || input == 'Y' || input == 'n' || input == 'N')
        {
            std::cin.ignore(32767, '\n');
            return input;
        }
        else
        {
            std::cin.ignore(32767, '\n');
            std::cout << "An invalid value was given, please try again.\n";
        }
    }
}

// The parameters is not used
// They're needed because of the function pointer
Card::CARD_COLOUR getChosenColour(const Player&, const std::vector<Player>&)
{
    int x{};
    while (true)
    {
        std::cout << "Choose the colour for the next player:\n(1) Yellow\n(2) Red\n(3) Green\n(4) Blue\n> ";
        std::cin >> x;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(32767, '\n');
        }
        else if (x >= 1 && x <= 4)
        {
            std::cin.ignore(32767, '\n');
            return static_cast<Card::CARD_COLOUR>((x-1));
        }
        else
        {
            std::cin.ignore(32767, '\n');
            std::cout << "An invalid value was given, please try again.\n";
        }
    }
}

int getCardChoice(const Player &player)
{
    int x{};
    while (true)
    {
        std::cout << "Choose the card to play (0 to cancel): ";
        std::cin >> x;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(32767, '\n');
        }
        else if (x >= 0 && x <= static_cast<int>(player.getVectorSize()))
        {
            std::cin.ignore(32767, '\n');
            return (x - 1);
        }
        else
        {
            std::cin.ignore(32767, '\n');
            std::cout << "An invalid value was given, please try again.\n";
        }
    }
}

int getCounterCardChoice(const std::vector<int> &cardIndexes, bool playedCard)
{
    int input{};
    while (true)
    {
        std::cout << "Choose the card to play" << ((playedCard) ? " (0 to stop playing cards): " : ": ");
        std::cin >> input;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(32767, '\n');
        }
        else if (input >= 0 && input <= static_cast<int>(cardIndexes.size()))
        {
            std::cin.ignore(32767, '\n');
            if (!playedCard && input == 0) // Can only end your turn after you play a card
            {
                std::cout << "An invalid value was given, please try again.\n";
                continue;
            }

            if (input == 0) // Doesn't play anything, ends player's turn
                return -1;  // Because the skip card can be indexed 0 in playerVar

            return cardIndexes[input-1]; // Because the player's numbering starts from 1
        }
        else
        {
            std::cin.ignore(32767, '\n');
            std::cout << "An invalid value was given, please try again.\n";
        }
    }
}

bool doesPlayerWantToCounter(Technical Text = Technical::PLAY_COUNTER_TEXT)
{
    int input{};
    while (true)
    {
        switch (Text)
        {
        case Technical::PLAY_COUNTER_TEXT:
            std::cout << "Do you want to counter the played card?\n(1) Yes\n(2) No\n> ";
            break;
        case Technical::PLAY_TAKEN_TEXT:
            std::cout << "Do you want to play the taken card?\n(1) Yes\n(2) No\n> ";
            break;
        }
        std::cin >> input;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(32767, '\n');
        }
        else if (input == 1 || input == 2)
        {
            std::cin.ignore(32767, '\n');
            switch (input)
            {
            case 1:
                return true;
            case 2:
                return false;
            }
        }
        else
        {
            std::cin.ignore(32767, '\n');
            std::cout << "An invalid value was given, please try again.\n";
        }
    }
}

std::string getPlayerName()
{
    std::cout << "Please enter the player's name: ";
    std::string x{};
    std::getline(std::cin, x);

    return x;
}

void printColourName(Card::CARD_COLOUR colour)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); // Does something that makes the colours work
    std::string text{};
    switch (colour)
    {
    case Card::CARD_COLOUR::YELLOW:
        SetConsoleTextAttribute(hConsole, 6);
        text = "Yellow";
        break;
    case Card::CARD_COLOUR::RED:
        SetConsoleTextAttribute(hConsole, 12);
        text = "Red";
        break;
    case Card::CARD_COLOUR::GREEN:
        SetConsoleTextAttribute(hConsole, 10);
        text = "Green";
        break;
    case Card::CARD_COLOUR::BLUE:
        SetConsoleTextAttribute(hConsole, 3);
        text = "Blue";
        break;
    }

    // Prints the colour
    std::cout << text;

    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Set the colour back to white
}

void printHumanTurnInfo(const Card &playedCard, const Player &player, Card::CARD_COLOUR chosenColour)
{
    std::cout << "\n###################################\n\n";
    std::cout << "Card played: " << playedCard;
    if (playedCard.getRank() >= Card::CARD_RANK::PLUS_4)
    {
        std::cout << "The chosen colour is: ";
        printColourName(chosenColour);
    }  
    std::cout << "\nYou have the following cards:\n" << player << '\n';
    std::cout << "Your moves:\n";
    std::cout << "(1) Pick a card\n";
    std::cout << "(2) Play a card\n";
    std::cout << "(3) Sort your cards\n";
    std::cout << "(4) End your turn\n";
    std::cout << "(5) Check other players' cards" << std::endl;
    // Hopefully std::endl will stop std::cout form lagging the console
}

//// Important Functions

// effect functions

void createDeck(std::vector<Card> &deck)
{
    int cardIndex{ 0 };

    for (int rowCount{0}; rowCount < 8; ++rowCount) // Loop through all the rows, first 4 have ZEROs && Wild cards, the other half has
    {                                               // no ZEROs and +4 instead of Wild cards
        if (rowCount < 4)
        {
            for (int columnCount{0}; columnCount < 15; ++columnCount) // 14 cards in this "pack"
            {
                if (columnCount == 13) // Doesn't add +4
                    continue;
                if (columnCount == 14) // Adds the SPECIAL card type
                {
                    deck[cardIndex].setCard(4, columnCount);
                    ++cardIndex;
                    continue;
                }
                deck[cardIndex].setCard(rowCount , columnCount);
                ++cardIndex;
            }
        }
        else
        {
            for (int columnCount{ 1 }; columnCount < 15; ++columnCount) // 13 cards in this "pack" (has no ZEROs). 0 == ZERO, that's why it starts from 1
            {
                if (columnCount == 14) // Doesn't add wild cards
                    continue;
                if (columnCount == 13) // Adds the SPECIAL card type
                {
                    deck[cardIndex].setCard(4, columnCount);
                    ++cardIndex;
                    continue;
                }
                deck[cardIndex].setCard((rowCount - 4), columnCount);
                ++cardIndex;
            }
        }
    }

    std::shuffle(deck.begin(), deck.end(), Mersenne::mersenne);
}

void createPlayers(std::vector<Player> &players)
{
    // Creates the player
    std::string playerName{ getPlayerName() };
    players[0].setPlayer(0, playerName, AI_PERSONAS::HUMAN);

    // Prepares the names for ai players
    std::array<std::string, 20> aiNames{ "John", "William", "James", "Jack", "Harry", "Jacob", "Thomas", "George", "Jake", "Connor",
                                         "Kyle", "Joe", "Ethan", "Alexander", "Daniel", "David", "Richard", "Robert", "Joseph", "Michael" };

    std::shuffle(aiNames.begin(), aiNames.end(), Mersenne::mersenne);

    // Prepares the personalities for ai players
    std::array<AI_PERSONAS, 12> aiPersonas{ AI_PERSONAS::CAUTIOUS, AI_PERSONAS::ADVANCED, AI_PERSONAS::BOLD, AI_PERSONAS::CASUAL, AI_PERSONAS::UNPREDICTABLE,
                                            AI_PERSONAS::NORMAL, AI_PERSONAS::NORMAL, AI_PERSONAS::NORMAL, AI_PERSONAS::NORMAL, AI_PERSONAS::NORMAL, 
                                            AI_PERSONAS::NORMAL, AI_PERSONAS::NORMAL };

    std::shuffle(aiPersonas.begin(), aiPersonas.end(), Mersenne::mersenne);

    // Creates AI
    for (std::size_t count{1}; count < players.size(); ++count)
    {
        players[count].setPlayer(count, aiNames[count], aiPersonas[count]);
    }
}

void giveOutCardsToPlayers(std::vector<Player> &players, std::vector<Card> &deck)
{
    int deckCardCounter{ 107 };
    for (int count{ 0 }; count < g_startingCards; ++count) // Gives out 7 cards per start
    {
        for (std::size_t playerCount{ 0 }; playerCount < players.size() ; ++playerCount) // Loops thru every player
        {
            players[playerCount].push_card(deck[deckCardCounter--]);
            deck.pop_back();
        }
    }
}

void initializePlayedCards(std::vector<Card> &playedCards, std::vector<Card>& deck)
{
    playedCards.resize(0); // set size to 0
    playedCards.push_back(deck.back()); // add a card from the top of the deck
    deck.pop_back();
}

void checkAvailableAndAddCardsInDeck(std::vector<Card>& playedCards, std::vector<Card>& deck, int numberOfCardsTaken=1)
{
    if (static_cast<int>(deck.size()) <= (9 + numberOfCardsTaken))
    {
        std::vector<Card> tempShuffle{};

        // Stores the last played card
        Card tempLastPlayed{ playedCards.back() };
        playedCards.pop_back();

        for (int count{ static_cast<int>(playedCards.size() - 1) }; count >= 0 ; --count) // Moves all cards (beside the last one) to the tempShuffle vector
        {
            tempShuffle.push_back(playedCards[count]);
            playedCards.pop_back();
        }
        playedCards.push_back(tempLastPlayed);
        std::shuffle(tempShuffle.begin(), tempShuffle.end(), Mersenne::mersenne);

        // Inserts all of the cards in "tempShuffle" to the beginning of "deck"
        // Deck can store all of the cards so the refrence won't be invalidated
        for (std::size_t count{0u}; count < tempShuffle.size(); ++count)
        {
            deck.insert(deck.begin(), tempShuffle.back());
            tempShuffle.pop_back();
        }
    }
}

void switchGameDirection(bool &gameDirection)
{
    gameDirection = !gameDirection;
}

void assignCards(Player &player, std::vector<Card>& playedCards, std::vector<Card>& deck, int cardsToTake=1)
{
    checkAvailableAndAddCardsInDeck(playedCards, deck, cardsToTake); // Checks if there are enough cards in the deck to pick "cardsToTake"
                                                                     // at start and at every action which influences the deck vector
    for (int count{0}; count < cardsToTake ;++count)
    {
        player.push_card(deck.back());
        deck.pop_back();
    } 
}

void nextPlayer(int& playerCount, int maxPlayerIndex, bool gameDirection)
{
    if (gameDirection)
    {
        if (playerCount < maxPlayerIndex)
            ++playerCount;
        else
            playerCount = 0;
    }
    else
    {
        if (playerCount != 0)
            --playerCount;
        else
            playerCount = maxPlayerIndex;
    }
}

void CounterSkipCard(Player& playerVar, std::vector<Card>& playedCards, int& turnsBlocked)
{
    std::vector<int> skipIndexes(8); // There are only 8 skip cards
    bool playedSkip{ false };

    for (;;)
    {
        skipIndexes.resize(0); // reset the vector

        for (int count{ 0 }, loc_number{ 0 }; count < static_cast<int>(playerVar.getVectorSize()); ++count) // Prints skip cards, collects their indexes
        {
            const auto& currentCard{ playerVar.getCard(count) };
            if (currentCard.getRank() == Card::CARD_RANK::SKIP)
            {
                std::cout << '(' << ++loc_number << ") " << currentCard; // Prints the card, \n is added by the overload
                skipIndexes.push_back(count);
            }
        }

        if (skipIndexes.size() == 0) // If there are no skip cards break the loop (the player has to have skip cards at start, so this won't break anything)
            break;

        int choice{ getCounterCardChoice(skipIndexes, playedSkip) };

        if (choice == -1)   // Available when player playes a card and choses to not play anymore cards
            break;          // (can only equal -1 when a card was played, this is guaranteed by the above function)
        else
        {
            ++turnsBlocked;

            // Adds the card to playedCards vector, removes the card from player's hand
            playedCards.push_back(playerVar.getCard(choice));
            std::swap(playerVar.getCardRef(choice), playerVar.getLastCardRef());
            playerVar.pop_card();

            playedSkip = true;
        }
    }
}

void CounterPlusCard(Player& playerVar, std::vector<Card>& playedCards, int& takeCards, Card::CARD_COLOUR& chosenColour)
{
    std::vector<int> plusIndexes(12); // There are only 12 plus cards
    bool playedPlus{ false };

    bool (*fcnPtr)(const Card&, const Card&, Card::CARD_COLOUR) { &isAbleToCounterPlusWithChosenCard };

    // Prints all of the player's card colours
    {
        int vectorSize{ static_cast<int>(playerVar.getVectorSize()) };

        int green{ 0 };
        int blue{ 0 };
        int red{ 0 };
        int yellow{ 0 };

        for (int count{ 0 }; count < vectorSize; ++count)
        {
            switch (playerVar.getCard(count).getColour())
            {
            case Card::CARD_COLOUR::GREEN:
                ++green;
                break;
            case Card::CARD_COLOUR::BLUE:
                ++blue;
                break;
            case Card::CARD_COLOUR::RED:
                ++red;
                break;
            case Card::CARD_COLOUR::YELLOW:
                ++yellow;
                break;
            }
        }

        std::cout << "\nYou have " << green << ' ';
        printColourName(Card::CARD_COLOUR::GREEN);
        std::cout << " cards, " << blue << ' ';
        printColourName(Card::CARD_COLOUR::BLUE);
        std::cout << " cards, " << red << ' ';
        printColourName(Card::CARD_COLOUR::RED);
        std::cout << " cards, " << yellow << ' ';
        printColourName(Card::CARD_COLOUR::YELLOW);
        std::cout << " cards.\n";
    }

    for (;;)
    {
        plusIndexes.resize(0); // reset the vector

        // At first you can play any card you wish, but then you can only play cards of the same rank
        if (playedPlus)
            fcnPtr = &isAbleToPlayMoreChosenCard;

        int vectorSize{ static_cast<int>(playerVar.getVectorSize()) };

        for (int count{ 0 }, loc_number{ 0 }; count < vectorSize; ++count) // Prints plus cards, collects their indexes
        {
            const auto& currentCard{ playerVar.getCard(count) };
            auto currentCardRank{ currentCard.getRank() };
            if ((currentCardRank == Card::CARD_RANK::PLUS_2 || currentCardRank == Card::CARD_RANK::PLUS_4) && 
                    fcnPtr(currentCard, playedCards.back(), chosenColour))
            {
                std::cout << '(' << ++loc_number << ") " << currentCard; // Prints the card, \n is added by the overload
                plusIndexes.push_back(count);
            }
        }

        if (plusIndexes.size() == 0) // If there are no plus cards break the loop (the player has to have plus cards at start, so this won't break anything)
            break;

        int choice{ getCounterCardChoice(plusIndexes, playedPlus) };

        if (choice == -1) // Available when player playes a card and choses to not play anymore cards
            break;
        else
        {
            const auto& chosenCard{ playerVar.getCard(choice) };

            switch (chosenCard.getRank())
            {
            case Card::CARD_RANK::PLUS_2:
                takeCards += 2;
                break;
            case Card::CARD_RANK::PLUS_4:
                takeCards += 4;
                chosenColour = getChosenColour(Player(), std::vector<Player>());
                std::cout << '\n' << playerVar.getName() << " has chosen the next colour to be: ";
                printColourName(chosenColour);
                std::cout << "\n\n";
                break;
            }
            // Adds the card to playedCards vector, removes the card from player's hand
            playedCards.push_back(chosenCard);
            std::swap(playerVar.getCardRef(choice), playerVar.getLastCardRef());
            playerVar.pop_card();

            playedPlus = true;
        }
    }
}

void playCard(Player& playerVar, std::vector<Card>& playedCards, const Card& chosenCard, int chosenCardIndex,
                bool& gameDirection, Card::CARD_COLOUR& chosenColour, int& takeCards, int& turnsBlocked, int time_AIActions)
{
    using Rank = Card::CARD_RANK;
    auto chosenCardRank{ chosenCard.getRank() };
    bool isPlayer{ !playerVar.getID() };

    // Function pointer to not make a duplicate function for AI
    // At start assigns the AI version to fcnPtr
    Card::CARD_COLOUR(*fcnPtr)(const Player&, const std::vector<Player>&) { &AI_decideChosenColour };

    // If a human plays, fcnPtr is set to getChosenColour()
    if (isPlayer)
        fcnPtr = &getChosenColour;

    if (chosenCardRank >= Rank::SKIP) // won't execute the switch if the card is lower than skip
    {
        switch (chosenCardRank)
        {
        case Rank::SKIP:
            ++turnsBlocked;
            break;
        case Rank::PLUS_2:
            takeCards += 2;
            break;
        case Rank::REVERSE:
            switchGameDirection(gameDirection);
            break;
        case Rank::PLUS_4:
            // There's no break; here beacuse the player has to choose the next colour, and this is the only thing that
            // case Rank::WILD does, so less duplicate code is the result
            takeCards += 4;
        case Rank::WILD:
            chosenColour = fcnPtr(Player(), std::vector<Player>());
            std::cout << '\n' << playerVar.getName() << " has chosen the next colour to be: ";
            printColourName(chosenColour);
            std::cout << "\n\n";
            break;
        }
    }

    // Adds the card to playedCards vector, removes the card from player's hand
    playedCards.push_back(chosenCard);
    if (isPlayer)
    {
        // This removes the card immediately
        std::swap(playerVar.getCardRef(chosenCardIndex), playerVar.getLastCardRef());
        playerVar.pop_card();
    }
    else
    {
        // Sleep for time_AIActions microseconds
        std::this_thread::sleep_for(std::chrono::microseconds(time_AIActions));
        // If we do the same for AI, it'll mess up the indexes to play
        // This changes the card's rank & colour to MAX_TYPE so that these can be removed later
        playerVar.getCardRef(chosenCardIndex).setCard(Card::CARD_COLOUR::MAX_CARD_TYPE, Card::CARD_RANK::MAX_CARD_RANK);
    }
    
}

void showOtherPlayersInfo(const std::vector<Player>& players, int currentPlayerIndex, int maxPlayerIndex, bool gameDirection)
{
    int nextPlayerIndex{ currentPlayerIndex };

    while (true)
    {
        nextPlayer(nextPlayerIndex, maxPlayerIndex, gameDirection);

        // Skips the turn for the ai players who are not playing
        if (players[nextPlayerIndex].getTurnsBlocked() != 0)
            continue;

        // Once it found the next player stop the loop
        break;
    }

    const auto& nextplayer{ players[nextPlayerIndex] };

    // "UI" begins here
    // Prints who's the next player
    std::cout << "\n\nNext player: " << nextplayer.getName() << " has " << nextplayer.getVectorSize() << " cards.\n";

    // Prints all the other players (don't include the player himself)
    for (int iii{1}; iii < static_cast<int>(players.size());iii++)
    {
        const auto& currentPlayer{ players[iii] };
        const auto currentPlayerTurnsBlocked{ currentPlayer.getTurnsBlocked() };

        // Doesn't print players who are not playing
        if (currentPlayerTurnsBlocked < 0)
            continue;

        // Print info
        std::cout << currentPlayer.getName() << " has " << currentPlayer.getVectorSize() << " cards.";
        if (currentPlayerTurnsBlocked > 0)
            std::cout << " Is blocked for " << currentPlayerTurnsBlocked << " turns\n";
        else
            std::cout << '\n';
    }
    
    std::cout << "\nInput anything to continue...\n";
    pressAnything();
}

// This goes from up down (ex. amount of players goes from 8 to 7, then it sets a longer time, going from 7 to 8 is impossible)
void changeTime_AIActions(const std::vector<Player>& players, int& time_AIActions)
{
    const size_t playersSize{ players.size() };

    // early exit to save time
    if (playersSize < 5)
        return;

    size_t playingPlayers{ playersSize };

    for (size_t iii{ 0 }; iii < playersSize; ++iii)
    {
        if (players[iii].getTurnsBlocked() == -1)
            --playingPlayers;
    }

    if (playingPlayers < 5)
    {
        time_AIActions = g_longTime;
    }
    else if (playingPlayers < 8)
    {
        time_AIActions = g_mediumTime;
    }
}

// Boolean functions

bool canCounterSkipCard(const Player& player)
{
    for (std::size_t count{ 0 }; count < player.getVectorSize(); ++count)
    {
        // No need for opt variables, as there is the same amount of function calls either way
        using Rank = Card::CARD_RANK;

        // If the player has a SKIP card, he can counter the block
        if (player.getCard(count).getRank() == Rank::SKIP)
            return true;
    }
    return false;
}

bool canCounterPlusCards(const Player& player, const Card& playedCard, Card::CARD_COLOUR chosenColour) // This function assumes that playedCard is +4 or +2
{
    for (std::size_t count{ 0 }; count < player.getVectorSize(); ++count)
    {
        using Rank = Card::CARD_RANK;
        // Don't repeat yourself
        const auto& currentCard{ player.getCard(count) };
        auto currentCardRank{ currentCard.getRank() };
        auto playedCardRank{ playedCard.getRank() };

        // This checks if the rank is the same      // If +4 is placed, +2 of chosen colour can be placed
        if (currentCardRank == playedCardRank || (currentCard.getColour() == chosenColour && currentCardRank == Rank::PLUS_2)
            || (currentCardRank == Rank::PLUS_4 && playedCardRank == Rank::PLUS_2)) // If +2 is placed, +4 can be placed
            return true;
    }
    return false;
}

bool isAbleToPlayACard(const Player &player,const Card &playedCard, const Card::CARD_COLOUR chosenColour) // Looks thru all of the player's cards
{
    for (std::size_t count{ 0 }; count < player.getVectorSize(); ++count)
    {
        using Colour = Card::CARD_COLOUR;
        using Rank = Card::CARD_RANK;
        // Don't repeat yourself
        const auto& currentCard{ player.getCard(count) };
        auto currentCardColour{ currentCard.getColour() };
        auto playedCardRank{ playedCard.getRank() };
        Colour playedCardColour{};

        // +4 and WILDs must have a chosen colour, this way I can easily check for the colour need without any additional complexity
        if (playedCardRank >= Rank::PLUS_4)
            playedCardColour = chosenColour;
        else
            playedCardColour = playedCard.getColour();

              // This checks if the colours are the same                        // This checks if the rank is the same
        if (currentCardColour == playedCardColour || currentCard.getRank() == playedCardRank
            || currentCardColour == Colour::SPECIAL)  // This checks if the card is +4 or wild (using the SPECIAL colour)
            return true;
    }
    return false;
}

bool isAbleToPlayMoreCards(const Player& player, const Card& playedCard) // Looks thru all of the player's cards
{
    for (std::size_t count{ 0 }; count < player.getVectorSize(); ++count)
    {
        // This checks if the rank is the same (Only then can you more cards. eg. placed a blue 5 on blue, and then 2 different fives on it)
        if (player.getCard(count).getRank() == playedCard.getRank())
            return true;
    }
    return false;
}

bool isAbleToPlayChosenCard(const Card& chosenCard, const Card& playedCard, Card::CARD_COLOUR chosenColour)
{
    using Colour = Card::CARD_COLOUR;
    using Rank = Card::CARD_RANK;

    // Don't repeat yourself
    auto chosenCardColour{ chosenCard.getColour() };
    auto playedCardRank{ playedCard.getRank() };
    Colour playedCardColour{};

    // +4 and WILDs must have a chosen colour, this way I can easily check for the colour needed without any additional complexity
    if (playedCardRank >= Rank::PLUS_4)
        playedCardColour = chosenColour;
    else
        playedCardColour = playedCard.getColour();

    // This checks if the colours are the same  // This checks if the rank is the same    // This checks if the card is +4 or wild (using the SPECIAL colour)
    return (chosenCardColour == playedCardColour || chosenCard.getRank() == playedCardRank || chosenCardColour == Colour::SPECIAL);
}

bool isAbleToPlayMoreChosenCard(const Card& chosenCard, const Card& playedCard, Card::CARD_COLOUR)
{
    // This checks if the rank is the same (Only then can you more cards. eg. placed a blue 5 on blue, and then 2 different fives on it)
    return (chosenCard.getRank() == playedCard.getRank());
}

bool isAbleToCounterPlusWithChosenCard(const Card& chosenCard, const Card& playedCard, Card::CARD_COLOUR chosenColour)
{
    // Here only +2 or +4 can be played
    using Rank = Card::CARD_RANK;

    // Don't repeat yourself
    auto chosenCardRank{ chosenCard.getRank() };
    auto playedCardRank{ playedCard.getRank() };

    // This checks if the colours are the same  // This checks if the rank is the same      // If +2 is placed, +4 can be placed
    // (will only work for +2)
    return (chosenCard.getColour() == chosenColour || chosenCardRank == playedCardRank || (chosenCardRank == Rank::PLUS_4 && playedCardRank == Rank::PLUS_2));
}

// UNO

bool playUNO()
{
    // Initializes the deck
    std::vector<Card> deck(108);
    createDeck(deck);

    /*
    {   //DEBUG
        for (const auto& element : deck)
            std::cout << element;
    }
    */

    // Initializes the players
    std::vector<Player> players(getNumOfPlayers());
    createPlayers(players);
    giveOutCardsToPlayers(players, deck);

    // Sets up the time between AI's actions
    int time_AIActions{ g_longTime };
    switch (players.size())
    {
    case 5:
    case 6:
    case 7:
        time_AIActions = g_mediumTime;
        break;
    case 8:
    case 9:
    case 10:
        time_AIActions = g_shortTime;
        break;
    }

    /*
    {   // DEBUG
        for (const auto& element : players)
            std::cout << element;

        std::cout << '\n' << players[1].getVectorSize() << '\n';

        for (const auto& element : deck)
            std::cout << element;

        std::cout << deck.size();
    }
    */

    // True: +1, False: -1
    bool gameDirection{ true };                                     // The player to the dealer's left plays first - official rule, whatever this means, here it's +1 at start
    Card::CARD_COLOUR chosenColour{};                               // When a +4 or WILD card is chosen, the player must choose a colour
    int takeCards{ 0 };                                             // accumulated cards to take from +4 and +2
    int turnsBlocked{ 0 };                                          // Keeps track of all the skip (block) cards used (1 skip card = 1 turn blocked)
    int aiPlayersWon{ 0 };                                          // Keeps track of how many AI players won the game (used as a fail condition for the player)
    const int maxPlayerIndex{ static_cast<int>(players.size()) - 1 };

    // Creates the used cards' stack
    std::vector<Card> playedCards(108); // get the memory for 108 cards
    initializePlayedCards(playedCards, deck);

    // If one of these cards is the first one played, run these mechanics
    std::uniform_int_distribution randColour{ 0, 3 };
    switch (playedCards.back().getRank())
    {
    case Card::CARD_RANK::REVERSE:  // If the first played card is REVERSE, then change the game direction
        switchGameDirection(gameDirection);
        break;
    case Card::CARD_RANK::PLUS_2:   // If the first played card is +2 and the player can't defend himself, then add the amount of cards to "takeCards"
        takeCards += 2;
        std::cout << "\nThe following card is the first one on the stack: " << playedCards.back();
        break;
    case Card::CARD_RANK::PLUS_4:   // If the first played card is +4 and the player can't defend himself, then add the amount of cards to "takeCards" & choose a random colour
        takeCards += 4;
        chosenColour = static_cast<Card::CARD_COLOUR>(randColour(Mersenne::mersenne));
        std::cout << "\nThe following card is the first one on the stack: " << playedCards.back();
        break;
    case Card::CARD_RANK::SKIP:     // If the first played card is SKIP and the player can't defend himself, then add the amount of turns blocked to "turnsBlocked"
        ++turnsBlocked;
        std::cout << "\nThe following card is the first one on the stack: " << playedCards.back();
        break;
    case Card::CARD_RANK::WILD:     // If the first played card is WILD, then choose a random colour
        chosenColour = static_cast<Card::CARD_COLOUR>(randColour(Mersenne::mersenne));
        break;
    }

    /*
    {   // DEBUG
        for (const auto& element : playedCards)
            std::cout << element;

        std::cout << deck.size() << '\n';
    }
    */

    // Game mechanism
    std::uniform_int_distribution playerRand{ 0, static_cast<int>(players.size()) };

    for (int playerCount{ 0 /*playerRand(Mersenne::mersenne)*/ };;)
    {
        // If all AIs play all of their cards, the player loses
        // -1 here because it doesn't count the player
        if (aiPlayersWon == maxPlayerIndex)
            return false;

        // Instead of doing the same thing a few times, do it once
        auto& playerVar{ players[playerCount] };    // has to be a reference
        auto playedCardVar{ playedCards.back() };

        // Player has id == 0, AI has >= 1
        bool isAI{ static_cast<bool>(playerVar.getID()) };

        {
            auto playerVarTurnsBlocked{ playerVar.getTurnsBlocked() }; // This variable is not needed anywhere else
            if (playerVarTurnsBlocked < 0) // Skips the turn for the ai players who've won
            {
                nextPlayer(playerCount, maxPlayerIndex, gameDirection);
                continue;
            }

            // If the player is blocked, let the next player play
            if (playerVarTurnsBlocked > 0)
            {
                playerVar.addTurnsBlocked(-1); // Takes one blocked turn off
                if (isAI)
                    std::cout << '\n' << playerVar.getName() << " is blocked for this and " << (playerVarTurnsBlocked - 1) << " more turn(s).\n\n";
                else
                {
                    std::cout << "\nYou're blocked for this and " << (playerVarTurnsBlocked - 1) << " more turn(s).\n\n";
                    // Stops the execution for 1.5 seconds
                    std::this_thread::sleep_for(std::chrono::microseconds(1'500'000));
                } 

                nextPlayer(playerCount, maxPlayerIndex, gameDirection);
                continue; // no need to do the rest since this player won't do anything this turn
            }

            if (turnsBlocked > 0)
            {
                bool counterSkipVar{ canCounterSkipCard(playerVar) }; // First checks if the player has any cards than can counter skips

                if (isAI)
                { // AI will always play so there's no need for a loop here
                    if (counterSkipVar)
                    {
                        AI_CounterSkipCard(playerVar, playedCards, turnsBlocked, time_AIActions);

                        if (playerVar.getVectorSize() == 0) // If the AI has no cards left, it wins the game
                        {
                            std::cout << '\n' << playerVar.getName() << " has played all of his cards and won!\n\n";

                            // Stops the execution for time_AIActions microseconds
                            std::this_thread::sleep_for(std::chrono::microseconds(time_AIActions + g_aiVictoryAddTime));

                            ++aiPlayersWon;
                            playerVar.endGame();  // Sets the ai's turnsBlocked to -1, which doesn't allow it to play at all

                            changeTime_AIActions(players, time_AIActions);
                        }
                    }
                    else // If SKIP has been played and the player can't defend himself, then block him for "turnsBlocked" turns
                    {
                        std::cout << '\n' << playerVar.getName() << " has been blocked for this and " << (turnsBlocked - 1) << " next turn(s).\n\n";

                        // This turn also counts as blocked
                        playerVar.addTurnsBlocked(turnsBlocked - 1);
                        turnsBlocked = 0;

                        // Stops the execution for time_AIActions microseconds
                        std::this_thread::sleep_for(std::chrono::microseconds(time_AIActions));
                    }
                }
                else
                { // Human
                    for (;;)
                    {
                        if (counterSkipVar)
                        {
                            std::cout << "\nYou can counter a skip card!\n";

                            counterSkipVar = doesPlayerWantToCounter(); // Then he is able to choose whether he wants to counter them
                            if (!counterSkipVar) // If he doesn't want to counter them, he'll have to be blocked by the else option
                                continue;

                            CounterSkipCard(playerVar, playedCards, turnsBlocked); // Mechanics are here

                            if (playerVar.getVectorSize() == 0) // If the player has no cards left, he wins the game
                                return true;

                            break;
                        }
                        else // If SKIP has been played and the player can't defend himself, then block him for "turnsBlocked" turns
                        {
                            std::cout << "\nYou've been blocked for this and " << (turnsBlocked - 1) << " next turn(s).\n\n";

                            // This turn also counts as blocked
                            playerVar.addTurnsBlocked(turnsBlocked - 1);
                            turnsBlocked = 0;

                            // Stops the execution for 1.5 seconds
                            std::this_thread::sleep_for(std::chrono::microseconds(1'500'000));

                            break;
                        }
                    }
                }

                nextPlayer(playerCount, maxPlayerIndex, gameDirection);
                continue; // no need to do the rest since this player won't do anything this turn
            }
        }

        // This doesn't change playedCardVar
        if (takeCards > 0) // if takeCards is greater than 0 then playedCard has to be +2 or +4
        {
            bool counterPlusVar{ canCounterPlusCards(playerVar, playedCardVar, chosenColour) }; // First checks if the player has any cards than can counter plus cards

            if (isAI)
            { // AI
                for (;;)
                {
                    // If +4 and +2 have been played and the player can defend himself
                    if (counterPlusVar)
                    {
                        AI_CounterPlusCard(players, playerVar, playedCards, takeCards, chosenColour, time_AIActions); // Mechanics are here

                        if (playerVar.getVectorSize() == 0) // If the AI has no cards left, it wins the game
                        {
                            std::cout << '\n' << playerVar.getName() << " has played all of his cards and won!\n\n";

                            // Stops the execution for time_AIActions microseconds
                            std::this_thread::sleep_for(std::chrono::microseconds(time_AIActions + g_aiVictoryAddTime));

                            ++aiPlayersWon;
                            playerVar.endGame();  // Sets the ai's turnsBlocked to -1, which doesn't allow it to play at all

                            changeTime_AIActions(players, time_AIActions);
                        }

                        break;
                    }
                    else // If +4 and +2 have been played and the player can't defend himself, then add the cards
                    {
                        std::cout << '\n' << playerVar.getName() << " has taken " << takeCards << " card(s).\n\n";

                        if (auto deckBackRank{ deck.back().getRank() }; (deckBackRank == Card::CARD_RANK::PLUS_2 || deckBackRank == Card::CARD_RANK::PLUS_4) && 
                                isAbleToCounterPlusWithChosenCard(deck.back(), playedCards.back(), chosenColour))
                        {
                            counterPlusVar = true;
                            assignCards(playerVar, playedCards, deck, 1);
                            continue;
                        }

                        // Stops the execution for time_AIActions microseconds
                        std::this_thread::sleep_for(std::chrono::microseconds(time_AIActions));

                        assignCards(playerVar, playedCards, deck, takeCards);
                        takeCards = 0;

                        break;
                    }
                }
            }
            else
            { // Human
                for (;;)
                {
                    // If +4 and +2 have been played and the player can defend himself
                    if (counterPlusVar)
                    {
                        std::cout << "\nYou can counter a plus card!\n";

                        counterPlusVar = doesPlayerWantToCounter(); // Then he is able to choose whether he wants to counter them
                        if (!counterPlusVar) // If he doesn't want to counter them, he'll have to be take new cards, by the else option
                            continue;

                        CounterPlusCard(playerVar, playedCards, takeCards, chosenColour); // Mechanics are here

                        if (playerVar.getVectorSize() == 0) // If the player has no cards left, he wins the game
                            return true;

                        break;
                    }
                    else // If +4 and +2 have been played and the player can't defend himself, then add the cards
                    {
                        std::cout << "\nYou've taken " << takeCards << " card(s).\n";

                        if (auto deckBackRank{ deck.back().getRank() }; (deckBackRank == Card::CARD_RANK::PLUS_2 || deckBackRank == Card::CARD_RANK::PLUS_4) &&
                                isAbleToCounterPlusWithChosenCard(deck.back(), playedCards.back(), chosenColour))
                        {
                            std::cout << "\nYou've picked a plus card!\n";
                            counterPlusVar = true;
                            assignCards(playerVar, playedCards, deck, 1);
                            continue;
                        }

                        assignCards(playerVar, playedCards, deck, takeCards);
                        takeCards = 0;

                        break;
                    }
                }
            }

            nextPlayer(playerCount, maxPlayerIndex, gameDirection);
            continue; // no need to do the rest since this player won't do anything this turn
        }


        // The menu screen
        if (isAI) // AI
            AITurn(playerVar, players, playedCards, deck, playedCardVar, chosenColour, playerCount, maxPlayerIndex, gameDirection, takeCards, turnsBlocked, aiPlayersWon, time_AIActions);
        else // HUMAN
        {
            bool keepTurn{ true };                      // Deremines whether the player can make a move
            bool hasPlacedFirstCard{ false };           // After the first card is placed, only the cards of the same rank can be played

            while (keepTurn)
            {
                playedCardVar = playedCards.back(); // IMPORTANT // Needs to be here so that it updates the var every time a player does something

                if (playerVar.getVectorSize() == 0) // If the player has no cards left, he wins the game
                    return true;

                printHumanTurnInfo(playedCardVar, playerVar, chosenColour); // Prints your cards and moves

                /*
                {   // DEBUG
                    std::cout << deck.back() << deck.size() << '\n';
                }
                */

                int choice{ getChoice() };

                switch (choice)
                {
                case 1:
                    if (!hasPlacedFirstCard) // If the player makes a move, he can't use this option anymore (only usable as the first move)
                    {
                        if (const auto& deckBack{ deck.back() }; isAbleToPlayChosenCard(deckBack, playedCardVar, chosenColour))
                        {
                            std::cout << "\nYou've found a matching card!\n" << deckBack;
                            assignCards(playerVar, playedCards, deck, g_takeCardCount);

                            bool play{ doesPlayerWantToCounter(Technical::PLAY_TAKEN_TEXT) }; // returns true when the player wants to play the card

                            if (!play) // Available when player choses to not play the found card
                            {
                                keepTurn = false;  // Player takes the card and ends his turn

                                nextPlayer(playerCount, maxPlayerIndex, gameDirection);
                                break;
                            }
                                                           //     vvvvvvvvvvv          is now the "deck.back()", because it's in the player's hand
                            playCard(playerVar, playedCards, playerVar.getLastCard(), playerVar.getVectorSize() - 1,
                                        gameDirection, chosenColour, takeCards, turnsBlocked, time_AIActions);

                            hasPlacedFirstCard = true; // player is able to play other matching cards
                            break;
                        }
                        // basically else, as the if the above executes the code below won't

                        std::cout << "\nYou've picked the following card: " << deck.back() << '\n';

                        assignCards(playerVar, playedCards, deck, g_takeCardCount); // assigns cards to the player
                        keepTurn = false;

                        nextPlayer(playerCount, maxPlayerIndex, gameDirection);
                        break;
                    }

                    // This executes when the player can't pick a card
                    std::cout << loc_case1alert;
                    break;
                case 2:
                { // so chosenCard variables can be initialized
                    int chosenCardIndex{ getCardChoice(playerVar) }; // Chooses the card (numbering normal to arrays)

                    // Cancel move
                    if (chosenCardIndex == -1)
                        continue;

                    // This has to be moved here, otherwise when the player cancels his move this will getCard() for -1
                    const auto& chosenCard{ playerVar.getCard(chosenCardIndex) };

                    switch (hasPlacedFirstCard)
                    {
                    case false: // Used when you place your first card
                        if (isAbleToPlayChosenCard(chosenCard, playedCardVar, chosenColour))
                        {
                            playCard(playerVar, playedCards, chosenCard, chosenCardIndex, gameDirection, chosenColour, takeCards, turnsBlocked, time_AIActions);

                            hasPlacedFirstCard = true;
                            std::cout << loc_firstCardPlayed;
                        }
                        else
                            std::cout << loc_cantPlayThisCard;
 
                        break;
                    case true: // Used after you place your first card
                        if (isAbleToPlayMoreChosenCard(chosenCard, playedCardVar, chosenColour))
                            playCard(playerVar, playedCards, chosenCard, chosenCardIndex, gameDirection, chosenColour, takeCards, turnsBlocked, time_AIActions);
                        else
                            std::cout << loc_cantPlayThisCard;

                        break;
                    }
                    break;
                }
                case 3:
                    playerVar.sortCards(); // Sorts player's cards either by colour or rank
                    break;
                case 4:
                    if (hasPlacedFirstCard) // If the player makes a move, he can use this option (only usable after the first move)
                    {
                        keepTurn = false;
                        nextPlayer(playerCount, maxPlayerIndex, gameDirection);
                        std::cout << loc_turnEnded;
                        break;
                    }
                    std::cout << loc_case4alert;
                    break;
                case 5:
                    showOtherPlayersInfo(players, playerCount, maxPlayerIndex, gameDirection);
                    break;
                }
            }
        }
    }
}

int main()
{
    std::cout << "Uno, the card game\n\n";
    std::cout << "\nInput anything to play...\n";
    pressAnything();

    while (true)
    {
        std::cout << (playUNO() ? "\n\nYou've won!" : "\n\nYou've lost");
        std::cout << "\n\nDo you want to play again? (Y/N)\n";
        char input{ getYN() };
        if (input == 'y' || input == 'Y')
            continue;
        else
            break;
    }

    return 0;
}

// BUGS:

// COOL STUFF TO ADD:

// TO DO:
// it's done

// THINGS DONE:
// Added auto playedCard (and other) variables  (28.05.2020)
// Added case 3, allows the player to sort his cards + a few bugfixes                                               (30.05.2020)
// Added canCounterPlusCards() +4/+2 functionality (check if playing +2 on +4 works correctly)                      (30.05.2020)
// Added cardEffect() (Unfinished), removed shuffleDeck(), added a mechanism that allows you to play cards (case 2) (30.05.2020)
// Added Messages (like "You can't play this card")                                 (31.05.2020)
// Cards are numbered in printHumanTurnInfo()                                       (31.05.2020)
// You can now cancel playing a card                                                (31.05.2020)
// Added case 4, which ends the player's turn (possible after having made a move)   (31.05.2020)
// Make sure that skip cards work (block players)                                           (04.06.2020)
// Debug info has been hidden, bugfixes, changes to loc                                     (04.06.2020)
// Implemented chosenColour handling                                                        (04.06.2020)
// Added effects for +4 and WILD cards when one of them is the first card in playedCards    (04.06.2020)
// You can only take a card once using case 1   (07.06.2020)
// Game can end and skips AI players that have won  (10.06.2020)
// You can counter SKIP cards   (12.06.2020)
// You can choose not to counter SKIP cards (13.06.2020)
// You can counter +4 & +2 cards (the first picked card can also counter them)  (13.06.2020)
// Added card effects when countering   (14.06.2020)
// When you pick a card (using case 1) and it's the right one you can play it (and other cards that can be played on it)    (16.06.2020)
// Made auto variables references (16.06.2020)
// Work on AI can start (code has been nicely split into different files)   (22.06.2020)
// AI_menuChoice has been made  (09.07.2020)
// AI can now find possible card sets to play   (10.07.2020)          
// changed some variables to const auto& to reduce copies   (15.07.2020)
// Added case 5 that informs the player how many other players there are and how many cards do they have    (24.08.2020)
// Added a scoring system for AI and fixed a bug where AI would prefer red cards if the cards' colour ratio was 1:1:1:1 (25.08.2020)
// Finished AI behavior in AI.cpp, fixed a bug   (26.08.2020)
// Added a start menu and fixed a potential bug  (26.08.2020)
// Fixed a bug in a function that looked for the most common card colour & AI can now decide which colour to choose when playing +4 or WILD cards   (27.08.2020)
// Fixed a bug that made iTP useless for AI, the game says which colour has been chosen in playCard()   (28.08.2020)
// AI can counter SKIP and PLUS cards, fixed a bug in counterPlusCard and std::reverse in AI_limit works correctly now  (29.08.2020)
// AI has been finished                                                                                                 (29.08.2020)
// Game has been finished   (30.08.2020)
// Time between moves changes depending on the amount of active players (27.10.2020)
// Added a few spaces so the game looks nicer                           (27.10.2020)
// Game shows how many colors you have when you counter PLUS cards  (28.10.2020)
// Upgraded AI_decideChosenColour                                   (28.10.2020)

// BUGFIXES vvvvvv

// Changed 1 to 2 in AI_menuChoice and now AI works as intended         (29.08.2020)
// Fixed ai_removeMaxCards and other vecEnd variables                                                   (30.08.2020)
// Fixed chosenCard causing a crash when a move was canceled                                            (30.08.2020)
// Game now informs the player what card he picked                                                      (30.08.2020)
// Fixed end game message not showing up                                                                (30.08.2020)
// printHumanInfo() shows chosenColour, when an AI player wins a message pops up                        (30.08.2020)
// First turn now counts when calculating playerTurnsBlockedVar                                         (30.08.2020)
// Fixed a bug in counterPlusCard(), rand chose a number and not an index                               (30.08.2020)
// Added (s) to loc, moved if(hasWon) to the start of the loop, fixed a bug                             (30.08.2020)
// Changed <= to >= in AI_limitCards()                                                                  (30.08.2020)
// AI_hasPlusX() doesn't return false when it can't find a card - fixed                                 (30.08.2020)
// It's possible to enter counterPlusCard() and have no counter cards, which crashes the game - fixed   (30.08.2020)
// Changed the message that pops up when blocking a player and removed a superfluous nextPlayer() in AI.cpp     (31.08.2020)
// Playing all cards in counterCard() now ends the game                                                         (31.08.2020)
// Countering plus cards with +4 shows the chosen color                         (01.09.2020)
// Test and added something that might help console stop lagging                    (02.09.2020)
// Removed a superfluous nextplayer() in showotherPlayerInfo()                  (20.09.2020)
// Moving cards from playedCards to deck now keeps the same order of cards          (28.10.2020)