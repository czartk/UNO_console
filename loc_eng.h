#ifndef H_LOC_ENG
#define H_LOC_ENG

#include <string_view>

// Used to store lengthy strings

using Text = std::string_view;

constexpr Text loc_cardsSorted{ "\n#########################################################################################################\n\n    Cards have been sorted\n" };
constexpr Text loc_case1alert{ "\n#########################################################################################################\n\n    You can't do this, you've already played a card\n" };
constexpr Text loc_case1CardTaken{ "\n#########################################################################################################\n\n    You can't do this, you've already taken a card\n" };
constexpr Text loc_firstCardPlayed{ "\n#########################################################################################################\n\n    First card played. Now you can only play cards with the same symbol\n" };
constexpr Text loc_cantPlayThisCard{ "\n#########################################################################################################\n\n    You can't play this card\n" };
constexpr Text loc_turnEnded{ "\n#########################################################################################################\n\n    Your turn has ended.\n\n#########################################################################################################\n\n" };
constexpr Text loc_case4alert{ "\n#########################################################################################################\n\n    You can't do this, you haven't done anything yet!\n" };

#endif
