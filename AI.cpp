#define NOMINMAX
#include <iostream>
#include <algorithm>
#include <chrono> // This and
#include <thread> // this is used to pause code exec for some time
#include "Main.h"
#include "Classes.h"
#include "Main_Func.h"
#include "loc_eng.h"

// const variable forward declaration
extern const int g_takeCardCount;
extern const int g_aiVictoryAddTime;

// struct declarations
struct AI_info_t
{
    std::vector<int> s_indexesToPlay;
    std::vector<Card> s_correspondingCards;
    int AI_decisionPoints{ 0 };
};

// Functions

bool AI_hasPlus2(const Player& playerVar)
{
    for (int index{ 0 }, vecEnd{ static_cast<int>(playerVar.getVectorSize()) }; index < vecEnd; ++index)
    {
        if (playerVar.getCard(index).getRank() == Card::CARD_RANK::PLUS_2)
            return true;
    }
    return false;
}

bool AI_hasPlus4(const Player& playerVar)
{
    for (int index{ 0 }, vecEnd{ static_cast<int>(playerVar.getVectorSize()) }; index < vecEnd; ++index)
    {
        if (playerVar.getCard(index).getRank() == Card::CARD_RANK::PLUS_4)
            return true;
    }
    return false;
}

int otherPlayersCardSize(const std::vector<Player>& players, int AI_id, bool gameDirection, bool wantsPreviousPlayer=false)
{
    const int maxPlayerIndex{ static_cast<int>(players.size()) - 1 };
    // We know that the current player is playing now and since the Id corresponds to the player's position in the vector, we can use it to find out current player's index
    int currentIndex{ AI_id };

    // If the caller wants the previous player's card size then change the game direction
    if (wantsPreviousPlayer)
        gameDirection = !gameDirection;

    while(true)
    {
        // move to the next player
        nextPlayer(currentIndex, maxPlayerIndex, gameDirection);
        const auto& currentPlayer{ players[currentIndex] };

        // Skips the turn for the ai players who are not playing
        if (currentPlayer.getTurnsBlocked() != 0)
        {
            nextPlayer(currentIndex, maxPlayerIndex, gameDirection);
            continue;
        }

        return currentPlayer.getVectorSize();
    }
}

void addPointsForColors(AI_info_t& currentStrategy, const Player& playerVar, int p_cardsColour)
{
    using colour_t = Card::CARD_COLOUR;

    // This finds the most common colour the player has

    int has_red{};
    int has_blue{};
    int has_yellow{};
    int has_green{};

    colour_t mostCommonColour{};
    colour_t rarestColour{};

    for (int cardIndex{ 0 }; cardIndex < static_cast<int>(playerVar.getVectorSize()); ++cardIndex)
    {
        switch (playerVar.getCard(cardIndex).getColour())
        {
        case colour_t::RED:
            ++has_red;
            break;
        case colour_t::BLUE:
            ++has_blue;
            break;
        case colour_t::YELLOW:
            ++has_yellow;
            break;
        case colour_t::GREEN:
            ++has_green;
            break;
        }
    }

    {
        int greatestAmount{ std::max({ has_red, has_blue, has_yellow, has_green }) };
        int smallestAmount{ std::min({ has_red, has_blue, has_yellow, has_green }) };

        // Because std::max, when there are 2 (or 3) vars with the same highest values, returns the leftmost one, the AI will be biased to playing red cards
        // When "greatestAmount" == "has_red" && "gA" == "amount of other colours", it means that there is the same amount of colours in the set
        // and no points will be added, no need to check for "smallestAmount"
        if (greatestAmount == has_blue && greatestAmount == has_yellow && greatestAmount == has_green)
            return;

        if (greatestAmount == has_red)
            mostCommonColour = colour_t::RED;
        else if (greatestAmount == has_blue)
            mostCommonColour = colour_t::BLUE;
        else if (greatestAmount == has_yellow)
            mostCommonColour = colour_t::YELLOW;
        else if (greatestAmount == has_green)
            mostCommonColour = colour_t::GREEN;

        if (smallestAmount == has_red)
            rarestColour = colour_t::RED;
        else if (smallestAmount == has_blue)
            rarestColour = colour_t::BLUE;
        else if (smallestAmount == has_yellow)
            rarestColour = colour_t::YELLOW;
        else if (smallestAmount == has_green)
            rarestColour = colour_t::GREEN;
    }

    // This finds the most used colour in the strategy

    int strategy_has_red{};
    int strategy_has_blue{};
    int strategy_has_yellow{};
    int strategy_has_green{};

    colour_t strategy_mostUsedColour{};

    for (int cardIndex{ 0 }; cardIndex < static_cast<int>(currentStrategy.s_correspondingCards.size()); ++cardIndex)
    {
        switch (currentStrategy.s_correspondingCards[cardIndex].getColour())
        {
        case colour_t::RED:
            ++strategy_has_red;
            break;
        case colour_t::BLUE:
            ++strategy_has_blue;
            break;
        case colour_t::YELLOW:
            ++strategy_has_yellow;
            break;
        case colour_t::GREEN:
            ++strategy_has_green;
            break;
        }
    }

    {
        int greatestAmountStrategy{ std::max({ strategy_has_red, strategy_has_blue, strategy_has_yellow, strategy_has_green }) };

        // Because std::max, when there are 2 (or 3) vars with the same highest values, returns the leftmost one, the AI will be biased to playing red cards
        // When "greatestAmount" == "has_red" && "gA" == "amount of other colours", it means that there is the same amount of colours in the set
        // and no points will be added
        if (greatestAmountStrategy == strategy_has_red && greatestAmountStrategy == strategy_has_yellow && greatestAmountStrategy == strategy_has_green)
            return;

        if (greatestAmountStrategy == strategy_has_red)
            strategy_mostUsedColour = colour_t::RED;
        else if (greatestAmountStrategy == strategy_has_blue)
            strategy_mostUsedColour = colour_t::BLUE;
        else if (greatestAmountStrategy == strategy_has_yellow)
            strategy_mostUsedColour = colour_t::YELLOW;
        else if (greatestAmountStrategy == strategy_has_green)
            strategy_mostUsedColour = colour_t::GREEN;
    }

    if (strategy_mostUsedColour == mostCommonColour)
        currentStrategy.AI_decisionPoints += p_cardsColour;
    else if (strategy_mostUsedColour == rarestColour)
        currentStrategy.AI_decisionPoints -= p_cardsColour;
}

std::vector<int> AI_limitEffectCards(AI_info_t& chosenStrategy, size_t playerCardsAmount)
{
    // Checks if the strategy has effect cards in it and if so appropriately limits the amount of them played
    // (without this the AI would play all of its special cards at once)

    using rank_t = Card::CARD_RANK;

    switch (chosenStrategy.s_correspondingCards[0].getRank())
    {
    case rank_t::PLUS_4:
    case rank_t::PLUS_2:
    case rank_t::REVERSE:
    case rank_t::SKIP:
    case rank_t::WILD:

        // If the AI is left with 3 or less cards then play every card
        if (3u >= (playerCardsAmount - chosenStrategy.s_correspondingCards.size()))
            break;
        else
        {
            // If the AI is left with more cards, save 1 or 2 cards for later
            if (chosenStrategy.s_indexesToPlay.size() > 1u)
                // It just pops_back so this won't invalidate the Ref
                chosenStrategy.s_indexesToPlay.pop_back();
        }

        break;
    }

    return chosenStrategy.s_indexesToPlay;
}

std::vector<int> AI_decideIndexes(std::vector<AI_info_t>& AI_possibleStrategies, AI_PERSONAS AI_personality, 
                                  const Player& playerVar, const std::vector<Player>& players, bool gameDirection)
{
    // vvvvvv Explanation of how this function works
    /*
    // It grades the strategies depending on:
    // 1. The amount of cards to be played,
    // 2. Whether the strategy plays cards of colour which the AI has the most/least
    // 3. If the card is a +4 (of greatest importance) (also checks if the AI has any +4 or +2 cards, separately)
    // 4. If the card is a WILD (important)
    // 5. If the card is a +2 or block (important)
    // 6. If the card is a reverse card (of least importance)

    // When a strategy contains +4s or +2s the AI checks if the next player has less than let's say 4 cards

    // if the strategy uses effect cards, then if the AI is left with more than 3 cards after playing, save 1 effect card for later.

    // Available AI personalities

    //NORMAL,         // Normal player, acts normally (==1) const
    //CAUTIOUS,       // Especially cautious
    //BOLD,           // Is more daring than others (reasonably)
    //UNPREDICTABLE,  // His moves are mostly random
    //CASUAL,         // Takes some chances, doesn't really plan ahead
    //ADVANCED,       // Acts as reasonable as the coder can (probably shouldn't have named it advanced)
    */

    using rank_t = Card::CARD_RANK;

    // Opt variables
    auto AI_ps_Size{ AI_possibleStrategies.size() };
    auto AI_ID{ playerVar.getID() };

    // Other variables
    int previousPlayer_CardSize{ otherPlayersCardSize(players, AI_ID, gameDirection, true) };
    int nextPlayer_CardSize{ otherPlayersCardSize(players, AI_ID, gameDirection) };

    // Point variables set up
    // Sets up the weight for NORMAL persona
    int p_percards{ 10 };
    int p_cardsColour{ 10 };
    int p_plus4{ -4 };
    int p_hasOtherPlusCards{ 6 }; // Its third part is used
    int p_wild{ -2 };
    int p_plus2{ -2 };
    int p_block{ -2 };
    int p_reverseCard{ -2 };
    int p_nextPlayerLess4Cards{ 12 }; // Its 1.5x is used

    // Variables used by UNPREDICTABLE persona
    std::uniform_int_distribution AI_unpredictable{ 0, 4 };
    std::uniform_int_distribution AI_unpredictable_divisible{ 2, 4 };
    std::uniform_int_distribution AI_unpredictable_actionCards{ 0, 3 };

    // Sets up the weight for other personas
    switch (AI_personality)
    {
    case AI_PERSONAS::CAUTIOUS:
        p_cardsColour = 11;
        p_plus4 = -6;
        p_hasOtherPlusCards = 9;
        p_wild = -3;
        p_plus2 = -3;
        p_block = -3;
        p_reverseCard = -3;
        p_nextPlayerLess4Cards = 14;

        break;
    case AI_PERSONAS::BOLD:
        p_plus4 = -2;
        p_hasOtherPlusCards = 9;
        p_wild = -1;
        p_plus2 = -1;
        p_block = -1;
        p_reverseCard = -1;
        p_nextPlayerLess4Cards = 14;

        break;
    case AI_PERSONAS::UNPREDICTABLE:
        p_percards = (AI_unpredictable(Mersenne::mersenne) + 8);
        p_cardsColour = (AI_unpredictable(Mersenne::mersenne) + 8);
        p_plus4 = (AI_unpredictable(Mersenne::mersenne) - 5);
        p_hasOtherPlusCards = (AI_unpredictable_divisible(Mersenne::mersenne) * 3); // Makes sure it's divisible by 3
        p_wild = (AI_unpredictable_actionCards(Mersenne::mersenne) - 3);
        p_plus2 = (AI_unpredictable_actionCards(Mersenne::mersenne) - 3);
        p_block = (AI_unpredictable_actionCards(Mersenne::mersenne) - 3);
        p_reverseCard = (AI_unpredictable_actionCards(Mersenne::mersenne) - 3);
        p_nextPlayerLess4Cards = ((AI_unpredictable_divisible(Mersenne::mersenne) * 2) + 6); // Makes sure it can by multiplied by 1.5

        break;
    case AI_PERSONAS::CASUAL:
        p_plus4 = -3;
        p_wild = -1;
        p_hasOtherPlusCards = 9;

        break;
    case AI_PERSONAS::ADVANCED:
        p_cardsColour = 11;
        p_hasOtherPlusCards = 9;
        p_wild = -1;
        p_plus2 = -1;
        p_block = -1;
        p_reverseCard = -1;
        p_nextPlayerLess4Cards = 14;

        break;
    }

    // Goes thru every strategy and rates it
    for (int index{0}; index < static_cast<int>(AI_ps_Size); ++index)
    {
        auto& currentStrategy{ AI_possibleStrategies[index] };
        auto& cS_points{ currentStrategy.AI_decisionPoints };
        const auto& firstCardInStrategy{ currentStrategy.s_correspondingCards[0] };

        // Adds points for the amount of cards
        cS_points += p_percards * currentStrategy.s_indexesToPlay.size();

        // Adds points for the second check
        if (firstCardInStrategy.getColour() != Card::CARD_COLOUR::SPECIAL)
            addPointsForColors(currentStrategy, playerVar, p_cardsColour);

        // Keeps track of how many of these cards does the player have 
        int has_plus4{ 0 };
        int has_plus2{ 0 };

        // Adds the points for individual cards
        switch (firstCardInStrategy.getRank())
        {
        case rank_t::PLUS_4:
            // checks how many +4 cards does the player have
            // If the first card is a +4 card then the others must be +4 cards
            has_plus4 += static_cast<int>(currentStrategy.s_correspondingCards.size());

            // Adds points for a +4 card
            cS_points += p_plus4;

            // Adds points for having additional +4 cards (the other most likely won't be used,
            // as one of the following algorithms limits the effect cards played,
            // which means that these cards will be a safety measure in case the others too play +4 cards)
            if (has_plus4 > 1)
                cS_points += p_hasOtherPlusCards;

            // Adds points for having +2 cards (This doesn't guarantee safety but it may save the AI from picking cards)
            if (AI_hasPlus2(playerVar))
                cS_points += (p_hasOtherPlusCards / 3);

            // Adds points for the next player having less than 4 cards
            if (nextPlayer_CardSize < 4)
                cS_points += p_nextPlayerLess4Cards;

            break;
        case rank_t::WILD:
            // A WILD card can only be played when the first played card is a WILD too
            cS_points += p_wild;

            break;
        case rank_t::PLUS_2:
            // checks how many +2 cards does the player have
            // If the first card is a +2 card then the others must be +2 cards
            has_plus2 += static_cast<int>(currentStrategy.s_correspondingCards.size());

            // Adds points for a +2 card
            cS_points += p_plus2;

            // Adds points for having additional +2 cards (the other most likely won't be used,
            // as one of the following algorithms limits the effect cards played,
            // which means that these cards will be a safety measure in case the others too play +2 cards)
            if (has_plus2 > 1)
                cS_points += p_hasOtherPlusCards;

            // Adds points for having +4 cards (This guarantees safety)
            if (AI_hasPlus4(playerVar))
                cS_points += p_hasOtherPlusCards;

            // Adds points for the next player having less than 4 cards
            if (nextPlayer_CardSize < 4)
                cS_points += p_nextPlayerLess4Cards;

            break;
        case rank_t::SKIP:
            cS_points += p_block;

            if (nextPlayer_CardSize < 4)
                cS_points += (p_nextPlayerLess4Cards * 1.5);

            break;
        case rank_t::REVERSE:
            cS_points += p_reverseCard;

            // If the next player has more cards than the previous one, be less likely to play this card
            if (previousPlayer_CardSize < nextPlayer_CardSize)
                cS_points -= (p_nextPlayerLess4Cards * 1.5);

            // If the next player has less cards than the previous one, be more likely to play this card
            if (previousPlayer_CardSize > nextPlayer_CardSize)
                cS_points += (p_nextPlayerLess4Cards * 1.5);

            break;
        }  
    }

    // Choses the strategy with the most points
    int topPointsIndex{ 0 };
    int largestScore{ AI_possibleStrategies[0].AI_decisionPoints };

    // This function assumes that is more than one strategy
    // If there are 2 with the same amount of points the first is chosen
    for (int index{ 1 }; index < static_cast<int>(AI_ps_Size); ++index)
    {
        const auto& currentStrategy_p{ AI_possibleStrategies[index].AI_decisionPoints };
        if (currentStrategy_p > largestScore)
        {
            largestScore = currentStrategy_p;
            topPointsIndex = index;
        }
    }
            // Checks if the strategy has effect cards in it and if so appropriately limits the amount of them played
    return AI_limitEffectCards(AI_possibleStrategies[topPointsIndex], playerVar.getVectorSize());
}

bool index_isEqualToChosenIndexes(const AI_info_t& ai_info, int nextCardIndex)
{
    for (int vectorIndex{ 0 }, vecSize{ static_cast<int>(ai_info.s_indexesToPlay.size()) }; vectorIndex < vecSize; ++vectorIndex)
    {
        if (nextCardIndex == ai_info.s_indexesToPlay[vectorIndex])
            return true;
    }
    return false;
}

bool index_isEqualToChosenIndexes(const std::vector<int>& vector, int nextCardIndex)
{
    for (int vectorIndex{ 0 }, vecSize{ static_cast<int>(vector.size()) }; vectorIndex < vecSize; ++vectorIndex)
    {
        if (nextCardIndex == vector[vectorIndex])
            return true;
    }
    return false;
}

// AI behavior

// At first I look at what cards I can play, and then I decide which of the possible card sets should I play
// - This function in one sentence
std::vector<int> AI_strategy(const Player& playerVar, const std::vector<Player>& players, const Card& playedCard, Card::CARD_COLOUR chosenColour, bool gameDirection)
{
    auto AI_personality{ playerVar.getPersona() };  // Opt variable, keeps the player's persona

    std::vector<AI_info_t> AI_possibleStrategies(10); // Keeps all of the possible ways to play cards in this vector, 10 should be enough most of the time
    AI_possibleStrategies.resize(0);

    auto playerVectorSize{ playerVar.getVectorSize() };

    // This checks for the possible card combinations
    for (std::size_t cardIndex{ 0 }; cardIndex < playerVectorSize; ++cardIndex)
    {
        if (auto previousCard{ playerVar.getCard(cardIndex) }; isAbleToPlayChosenCard(previousCard, playedCard, chosenColour))
        {
            AI_info_t temp; // Keeps the details about a possible strategy

            auto& s_itp{ temp.s_indexesToPlay }; // Gotta optimize it
            auto& s_cC{ temp.s_correspondingCards };

            s_itp.reserve(20);
            s_cC.reserve(20);

            s_itp.push_back(cardIndex); // Adds the first index to the vector
            s_cC.push_back(previousCard); // Adds the corresponding card

            for (std::size_t nextCardIndex{ 0 }; nextCardIndex < playerVectorSize; ++nextCardIndex)
            {
                if (index_isEqualToChosenIndexes(temp, nextCardIndex))
                    continue; // If the current index is equal to one of the already chosen indexes, don't check it
                
                auto currentCard{ playerVar.getCard(nextCardIndex) };

                if (isAbleToPlayMoreChosenCard(currentCard, previousCard, chosenColour))
                {
                    s_itp.push_back(nextCardIndex);
                    s_cC.push_back(currentCard);

                    // resets this loop so that it can find all of the possible cards to play
                    // because the added card is now the last card played, we check for cards that are possible to play with the last added card
                    // "nextCardIndex" is reset so that it can search the whole array again
                    previousCard = currentCard;
                    nextCardIndex = 0;
                }
            }

            AI_possibleStrategies.push_back(temp);
        }
    }

    /*
    { // DEBUG
        std::cout << "\nCards' order is reverted";
        for (const auto& element : AI_possibleStrategies)
        {
            auto arraySize{ element.s_correspondingCards.size() };
            std::cout << "\n Strategy found: (DEBUG: CC size: " << arraySize << " ITP size: " << element.s_indexesToPlay.size() << ")\n";
            for (int index{0}; index < arraySize; ++index)
            {
                std::cout << '(' << element.s_indexesToPlay[index] << ") " << element.s_correspondingCards[index] << '\n';
            }
            std::cout << '\n';
        }

        return std::vector<int>(0);
    }
    */

    // Checks if the cards are worth playing, if not return an empty std::vector<int>
        // Will most likely be never done ^^^^^^^

    // If a strategy wasn't found then return an empty vector
    if (AI_possibleStrategies.size() == 0)
        return std::vector<int>(0);

    // If there is only one option, there's no need to test which one is the best
    if (AI_possibleStrategies.size() == 1)
    {
        std::vector<int> returnValue{ AI_limitEffectCards(AI_possibleStrategies[0], playerVar.getVectorSize()) };
        // Reorder the vector so that it's easier to play the cards later (first to play == last on the stack)
        if (returnValue.size() > 1) // if there is only one element in the vector there is no need to reorder it
            std::reverse(returnValue.begin(), returnValue.end());

        return returnValue;
    }

    // This decides the strategy to use
    std::vector<int> returnValue{ AI_decideIndexes(AI_possibleStrategies, AI_personality, playerVar, players, gameDirection) };
    if (returnValue.size() > 1) // if there is only one element in the vector there is no need to reorder it
        std::reverse(returnValue.begin(), returnValue.end());
    
    return returnValue;
}

// When AI picks a card and it matches the previous one, it can only play strategies with that card as the first one
std::vector<int> AI_strategy_when_picked_card(const Player& playerVar, const Card& pickedCard, Card::CARD_COLOUR chosenColour)
{
    auto playerVectorSize{ playerVar.getVectorSize() };

    // This checks for the possible card combinations
    auto previousCard{ pickedCard };
    AI_info_t temp; // Keeps the details about a possible strategy

    auto& s_itp{ temp.s_indexesToPlay }; // Gotta optimize it
    auto& s_cC{ temp.s_correspondingCards };

    s_itp.reserve(20);
    s_cC.reserve(20);

    // The card will be played before this strategy is made, so there's no need to add it to the strategy

    for (std::size_t nextCardIndex{ 0 }; nextCardIndex < playerVectorSize; ++nextCardIndex)
    {
        if (index_isEqualToChosenIndexes(temp, nextCardIndex))
            continue; // If the current index is equal to one of the already chosen indexes, don't check it

        auto currentCard{ playerVar.getCard(nextCardIndex) };

        if (isAbleToPlayMoreChosenCard(currentCard, previousCard, chosenColour))
        {
            s_itp.push_back(nextCardIndex);
            s_cC.push_back(currentCard);

            // resets this loop so that it can find all of the possible cards to play
            // because the added card is now the last card played, we check for cards that are possible to play with the last added card
            // "nextCardIndex" is reset so that it can search the whole array again
            previousCard = currentCard;
            nextCardIndex = 0;
        }
    }

    // If this one strategy has no iTP then return an empty vector
    if (s_itp.empty())
        return std::vector<int>(0);

    std::vector<int> returnValue{ AI_limitEffectCards(temp, playerVar.getVectorSize()) };

    // Reorder the vector so that it's easier to play the cards later (first to play == last on the stack)
    if (s_itp.size() > 1) // if there is only one (or 0) element in the vector there is no need to reorder it
        std::reverse(returnValue.begin(), returnValue.end());

    // There will only be 1 strategy
    return returnValue;

}

int AI_menuChoice(const std::vector<int>& indexes, const int AI_movesMade)
{
    auto AI_isVectorEmpty{ indexes.empty() };

    if (AI_movesMade && AI_isVectorEmpty) // Is placed here to avoid unnecessary operations, happens when the AI has finished its strategy
        return 3; // Ends the AI's turn

    if (AI_movesMade) // AI's following moves (after it plays a card)
    {
        return 2; // This will only execute once "AI_movesMade" is positive, this means that at this point "AI_isVectorEmpty" must be false, 
                  // otherwise the above if statement'd have executed
    }
    else    // AI's first move (after it had decided for an strategy)
    {
        if (AI_isVectorEmpty)
            return 1; // Pick a card
        else
            return 2; // Play a card
    }
}

void AITurn(Player& playerVar, const std::vector<Player>& players, std::vector<Card>& playedCards, std::vector<Card>& deck, Card& playedCardVar, Card::CARD_COLOUR& chosenColour, int& playerCount,
            const int maxPlayerIndex, bool& gameDirection, int& takeCards, int& turnsBlocked, int& aiPlayersWon, int& time_AIActions)
{
    bool keepTurn{ true };                          // Deremines whether the player can make a move
    int AI_movesMade{ 0 };                          // How many times has the AI done something, used in AI_menuChoice

    const auto& playerVarName{ playerVar.getName() };

    // This allows the AI to calculate it moves only once at start. This contains the whole AI strategy!
    // If this doesn't have any indexes, means no cards will be played == take card && end turn
    std::vector<int> cardIndexesToPlay{ AI_strategy(playerVar, players, playedCardVar, chosenColour, gameDirection) };

    while (keepTurn)
    {
        playedCardVar = playedCards.back(); // IMPORTANT // Needs to be here so that it updates the var every time a player does something

        /*
        // DEBUG //
        if (!AI_movesMade)
            std::cout << playerVar;
        ///////////
        */

        int choice{ AI_menuChoice(cardIndexesToPlay, AI_movesMade) };

        switch (choice)
        {
        case 1:
            if (auto deckBack{ deck.back() }; isAbleToPlayChosenCard(deckBack, playedCardVar, chosenColour))
            {
                // AI will always play the card
                std::cout << '\n' << playerVarName << " has picked a card!\n\n";
                std::cout << '\n' << playerVarName << " has played the picked card: " << deckBack << '\n';
                assignCards(playerVar, playedCards, deck, g_takeCardCount);

                /*
                // True when the player wants to play the card
                // If I remembered how to play UNO, I'd probably put something here
                // Because it's always true there is no need to run this code

                bool play{ true };

                if (!play) // Available when player choses to not play the found card
                {
                    keepTurn = false;  // Player takes the card and ends his turn

                    nextPlayer(playerCount, maxPlayerIndex, gameDirection);
                    break;
                }
                */
                                                    //     vvvvvvvvvvv          is now the "deck.back()", because it's in the player's hand
                playCard(playerVar, playedCards, playerVar.getLastCard(), playerVar.getVectorSize() - 1,
                    gameDirection, chosenColour, takeCards, turnsBlocked, time_AIActions);

                // AI is able to play other matching cards, therefore a new strategy must be found with this card as the first one
                cardIndexesToPlay = AI_strategy_when_picked_card(playerVar, deckBack, chosenColour);
                
                break;
            }

            std::cout << '\n' << playerVarName << " has picked a card!\n\n";
            // basically else, as the if the above executes the code below won't
            assignCards(playerVar, playedCards, deck, g_takeCardCount); // assigns cards to the player
            keepTurn = false;

            nextPlayer(playerCount, maxPlayerIndex, gameDirection);
            break;
        case 2:
        { // so chosenCard variables can be initialized
            int chosenCardIndex{ cardIndexesToPlay.back() }; // Chooses the card
            cardIndexesToPlay.pop_back();    // Remove the card's index from the strategy
            auto chosenCard{ playerVar.getCard(chosenCardIndex) };

            std::cout << '\n' << playerVarName << " has played: " << chosenCard << '\n';

            playCard(playerVar, playedCards, chosenCard, chosenCardIndex, gameDirection, chosenColour, takeCards, turnsBlocked, time_AIActions);

            break;
        }
        case 3:
            keepTurn = false;
            nextPlayer(playerCount, maxPlayerIndex, gameDirection);
            std::cout << '\n' << playerVarName << " has finished his turn\n\n";

            break;
        }

        ++AI_movesMade;
        // Stops the execution for time_AIActions microseconds
        std::this_thread::sleep_for(std::chrono::microseconds(time_AIActions));
    }

    // Removes the played cards
    playerVar.ai_removeMaxCards();

    if (playerVar.getVectorSize() == 0) // If the AI has no cards left, it wins the game
    {
        std::cout << '\n' << playerVarName << " has played all of his cards and won!\n\n";

        // Stops the execution for time_AIActions microseconds
        std::this_thread::sleep_for(std::chrono::microseconds(time_AIActions + g_aiVictoryAddTime));

        ++aiPlayersWon;
        playerVar.endGame();  // Sets the ai's turnsBlocked to -1, which doesn't allow it to play at all

        changeTime_AIActions(players, time_AIActions);
    }
}

Card::CARD_COLOUR mostCommonColour(const Player& playerVar)
{
    using colour_t = Card::CARD_COLOUR;

    // This finds the most common colour the player has

    int has_red{};
    int has_blue{};
    int has_yellow{};
    int has_green{};

    for (int cardIndex{ 0 }, vecEnd{ static_cast<int>(playerVar.getVectorSize()) }; cardIndex < vecEnd; ++cardIndex)
    {
        switch (playerVar.getCard(cardIndex).getColour())
        {
        case colour_t::RED:
            ++has_red;
            break;
        case colour_t::BLUE:
            ++has_blue;
            break;
        case colour_t::YELLOW:
            ++has_yellow;
            break;
        case colour_t::GREEN:
            ++has_green;
            break;
        }
    }

    int greatestAmount{ std::max({ has_red, has_blue, has_yellow, has_green }) };

    // When the AI has the same amount of all card colours, it chooses a random one
    if (greatestAmount == has_blue && greatestAmount == has_yellow && greatestAmount == has_green)
    {
        std::uniform_int_distribution AI_colourChance{ 0, 3 };
        return static_cast<Card::CARD_COLOUR>(AI_colourChance(Mersenne::mersenne));
    }

    if (greatestAmount == has_red)
        return colour_t::RED;
    else if (greatestAmount == has_blue)
        return colour_t::BLUE;
    else if (greatestAmount == has_yellow)
        return colour_t::YELLOW;
    else if (greatestAmount == has_green)
        return colour_t::GREEN;

}

Card::CARD_COLOUR mostCommonColourAC(const Player& playerVar)
{
    using colour_t = Card::CARD_COLOUR;
    using rank_t = Card::CARD_RANK;

    // This finds the most common colour the player has

    int has_red{};
    int has_blue{};
    int has_yellow{};
    int has_green{};

    for (int cardIndex{ 0 }, vecEnd{ static_cast<int>(playerVar.getVectorSize()) }; cardIndex < vecEnd; ++cardIndex)
    {
        const auto& currentCard{ playerVar.getCard(cardIndex) };
        const auto& currentCardRank{ currentCard.getRank() };

        // Only counts action card's colours
        if (currentCardRank == rank_t::SKIP || currentCardRank == rank_t::PLUS_2 || currentCardRank == rank_t::REVERSE)
        {
            switch (currentCard.getColour())
            {
            case colour_t::RED:
                ++has_red;
                break;
            case colour_t::BLUE:
                ++has_blue;
                break;
            case colour_t::YELLOW:
                ++has_yellow;
                break;
            case colour_t::GREEN:
                ++has_green;
                break;
            }
        }
    }

    int greatestAmount{ std::max({ has_red, has_blue, has_yellow, has_green }) };

    // When the AI has the same amount of all card colours, it chooses a random one
    if (greatestAmount == has_blue && greatestAmount == has_yellow && greatestAmount == has_green)
    {
        std::uniform_int_distribution AI_colourChance{ 0, 3 };
        return static_cast<Card::CARD_COLOUR>(AI_colourChance(Mersenne::mersenne));
    }

    if (greatestAmount == has_red)
        return colour_t::RED;
    else if (greatestAmount == has_blue)
        return colour_t::BLUE;
    else if (greatestAmount == has_yellow)
        return colour_t::YELLOW;
    else if (greatestAmount == has_green)
        return colour_t::GREEN;

}

Card::CARD_COLOUR AI_decideChosenColour(const Player& playerVar, const std::vector<Player>& players)
{
    // This checks how many players are playing
    const size_t playersSize{ players.size() };
    size_t playingPlayers{ playersSize };

    for (size_t iii{ 0 }; iii < playersSize; ++iii)
    {
        if (players[iii].getTurnsBlocked() == -1)
            --playingPlayers;
    }

    // 50% for choosing the colour the AI has the most
    // 25% for choosing the most common colour of the AI's action cards (skip, reverse, +2)
    // 2 of the above - If the colours are in a 1:1:1:1 proportion then it returns a random colour
    // 25% for choosing a random colour
    // If there are less than 4 active players, make it less likely for the AI to choose randomly
    // If there are less than 3 active players, disable choosing random colours
    std::uniform_int_distribution AI_colourChance{ 0, 11 };
    int chance{ AI_colourChance(Mersenne::mersenne) };
    
    if (playingPlayers < 4)
        chance -= 2;
    else if (playingPlayers < 3)
        chance -= 4;

    if (chance < 6)
        return mostCommonColour(playerVar);
    else if (chance < 9)
        return mostCommonColourAC(playerVar);
    else
    {
        std::uniform_int_distribution AI_randColour{ 0, 3 };
        return static_cast<Card::CARD_COLOUR>(AI_randColour(Mersenne::mersenne));
    }
}

void AI_limitCounterEffectCards(std::vector<int>& chosenStrategy, size_t playerCardsAmount)
{
    // Appropriately limits the amount of them played
    // (without this the AI would play all of its special cards at once)

    using rank_t = Card::CARD_RANK;

    // If the AI is left with 1 or less cards then play every card
    if (1u >= (playerCardsAmount - chosenStrategy.size()))
        return;
    // If the AI is left with more cards, save 1 or 2 cards for later
    else if (chosenStrategy.size() > 1u)
    {
        std::uniform_int_distribution AI_limit_effect_rand{ 1, (static_cast<int>(chosenStrategy.size()) - 1) };

        // Removes "times" cards from the strategy
        for (int times{ AI_limit_effect_rand(Mersenne::mersenne) }; times != 0; --times)
        {
            // It just pops_back so this won't invalidate the Ref
            chosenStrategy.pop_back();
        }
    }
}

void AI_CounterSkipCard(Player& playerVar, std::vector<Card>& playedCards, int& turnsBlocked, int time_AIActions)
{
    std::vector<int> skipIndexes(8); // There are only 8 skip cards
    skipIndexes.resize(0);

    // Finds and collects skip card indexes
    for (int count{ 0 }, vectorSize{ static_cast<int>(playerVar.getVectorSize()) }; count < vectorSize; ++count)
    {
        if (playerVar.getCard(count).getRank() == Card::CARD_RANK::SKIP)
            skipIndexes.push_back(count);
    }

    AI_limitCounterEffectCards(skipIndexes, playerVar.getVectorSize());

    // In this loop all of the chosen cards are played, loop ends when the vector is empty
    for (bool isFirstLoop{ true }; !(skipIndexes.empty());)
    {
        // Sets the card to play
        int choice{ skipIndexes.back() };
        const auto& chosenCard{ playerVar.getCard(choice) };
        // Removes it from the strategy
        skipIndexes.pop_back();

        if (isFirstLoop)
        {
            std::cout << '\n' << playerVar.getName() << " has countered the skip card with: " << chosenCard << '\n';
            isFirstLoop = false;
        }
        else
            std::cout << '\n' << playerVar.getName() << " has played another skip card: " << chosenCard << '\n';

        // Stops the execution for time_AIActions microseconds
        std::this_thread::sleep_for(std::chrono::microseconds(time_AIActions));

        // AI will always play
        ++turnsBlocked;

        // Adds the card to playedCards vector, removes the card from player's hand
        playedCards.push_back(chosenCard);
        // This changes the card's rank & colour to MAX_TYPE so that these can be removed later
        playerVar.getCardRef(choice).setCard(Card::CARD_COLOUR::MAX_CARD_TYPE, Card::CARD_RANK::MAX_CARD_RANK);
    }

    // Remove the played cards
    playerVar.ai_removeMaxCards();
}

// This function is guaranteed to have atleast one card able to be played, otherwise it will crash
void AI_CounterPlusCard(const std::vector<Player>& players, Player& playerVar, std::vector<Card>& playedCards, int& takeCards, Card::CARD_COLOUR& chosenColour, int time_AIActions)
{
    std::vector<int> plusIndexes(12); // There are only 12 plus cards
    int vectorSize{ static_cast<int>(playerVar.getVectorSize()) };

    plusIndexes.resize(0); // reset the vector

    // This collects plus card indexes that can be played as first
    for (int count{ 0 }; count < vectorSize; ++count)
    {
        const auto& currentCard{ playerVar.getCard(count) };
        auto currentCardRank{ currentCard.getRank() };
        if ((currentCardRank == Card::CARD_RANK::PLUS_2 || currentCardRank == Card::CARD_RANK::PLUS_4) &&
            isAbleToCounterPlusWithChosenCard(currentCard, playedCards.back(), chosenColour))
            plusIndexes.push_back(count);
    }

    // This randomly picks one of the starting indexes and then finds all of the possible cards to play with it.
    std::uniform_int_distribution AI_rand_plusCounter{ 0, (static_cast<int>(plusIndexes.size()) - 1) };
    int firstIndex{ plusIndexes[AI_rand_plusCounter(Mersenne::mersenne)] };
    auto previousCard{ playerVar.getCard(firstIndex) };

    plusIndexes.resize(1); // reset the vector, it will now hold the strategy for AI
    plusIndexes[0] = firstIndex; // Adds the first index to the vector

    for (int nextCardIndex{ 0 }; nextCardIndex < vectorSize; ++nextCardIndex)
    {
        if (index_isEqualToChosenIndexes(plusIndexes, nextCardIndex))
            continue; // If the current index is equal to one of the already chosen indexes, don't check it

        const auto& currentCard{ playerVar.getCard(nextCardIndex) };

        if (isAbleToPlayMoreChosenCard(currentCard, previousCard, chosenColour))
        {
            plusIndexes.push_back(nextCardIndex);

            // resets this loop so that it can find all of the possible cards to play
            // because the added card is now the last card played, we check for cards that are possible to play with the last added card
            // "nextCardIndex" is reset so that it can search the whole array again
            previousCard = currentCard;
            nextCardIndex = 0;
        }
    }

    // Limit the effect cards played
    AI_limitCounterEffectCards(plusIndexes, playerVar.getVectorSize());

    // Reorder the vector so that it's easier to play the cards later (first to play == last on the stack)
    if (plusIndexes.size() != 1) // if there is only one element in the vector there is no need to reorder it
        std::reverse(plusIndexes.begin(), plusIndexes.end());

    // In this loop all of the chosen cards are played, loop ends when the vector is empty
    for (bool isFirstLoop{ true }; !(plusIndexes.empty());)
    {
        // Sets the card to play
        int choice{ plusIndexes.back() };
        const auto& chosenCard{ playerVar.getCard(choice) };
        // Removes it from the strategy
        plusIndexes.pop_back();

        if (isFirstLoop)
        {
            std::cout << '\n' << playerVar.getName() << " has countered the plus card with: " << chosenCard << '\n';
            isFirstLoop = false;
        }
        else
            std::cout << '\n' << playerVar.getName() << " has played another plus card: " << chosenCard << '\n';

        // Stops the execution for time_AIActions seconds
        std::this_thread::sleep_for(std::chrono::microseconds(time_AIActions));

        // AI will always play
        switch (chosenCard.getRank())
        {
        case Card::CARD_RANK::PLUS_2:
            takeCards += 2;
            break;
        case Card::CARD_RANK::PLUS_4:
            takeCards += 4;
            chosenColour = AI_decideChosenColour(playerVar, players);
            std::cout << '\n' << playerVar.getName() << " has chosen the next colour to be: ";
            printColourName(chosenColour);
            std::cout << "\n\n";
            break;
        }

        // Adds the card to playedCards vector, removes the card from player's hand
        playedCards.push_back(chosenCard);
        // This changes the card's rank & colour to MAX_TYPE so that these can be removed later
        playerVar.getCardRef(choice).setCard(Card::CARD_COLOUR::MAX_CARD_TYPE, Card::CARD_RANK::MAX_CARD_RANK);
    }

    // Remove the played cards
    playerVar.ai_removeMaxCards();
}