#include "levels.h"

// Returns the level with this id, or nullptr when the id is out of range.
const Level* getLevel(int id) {
    static const Level levels[] = {
        {1,
         "Pliska",
         "First Bulgarian State",
         "Pliska was Bulgaria's first capital, from 681 until 893.",
         681,
         1,
         4.0f,
         3.0f,
         0.0f,
         0,
         false,
         false},
        {2,
         "Varbitsa Pass",
         "First Bulgarian State",
         "In 811 Khan Krum trapped the Byzantine army here. Legend says he turned the emperor's skull "
         "into a silver cup.",
         811,
         2,
         3.5f,
         2.0f,
         2.0f,
         0,
         false,
         false},
        {3,
         "Preslav",
         "First Bulgarian State",
         "Under Simeon, Preslav became the heart of Bulgaria's Golden Age of books and learning.",
         893,
         3,
         3.0f,
         1.5f,
         1.0f,
         3,
         false,
         false},
        {4,
         "Tarnovo",
         "Second Bulgarian State",
         "In 1185 the brothers Asen and Peter rose up in Tarnovo and brought Bulgaria back to life.",
         1185,
         1,
         3.75f,
         3.0f,
         0.0f,
         2,
         false,
         false},
        {5,
         "Cherven",
         "Second Bulgarian State",
         "Cherven sat on a cliff above a river canyon. It had more medieval church inscriptions than "
         "the capital Tarnovo.",
         0,
         2,
         3.25f,
         2.0f,
         0.0f,
         0,
         true,
         false},
        {6,
         "Klokotnitsa",
         "Second Bulgarian State",
         "At Klokotnitsa in 1230, Ivan Asen II rode into battle with the broken peace treaty on his "
         "spear.",
         1230,
         3,
         2.75f,
         1.5f,
         2.0f,
         3,
         true,
         false},
        {7,
         "Koprivshtitsa",
         "Road to Liberation",
         "The first shot of the April Uprising was fired at a bridge in Koprivshtitsa on 20 April 1876.",
         1876,
         1,
         3.5f,
         2.5f,
         1.0f,
         2,
         false,
         false},
        {8,
         "Plovdiv",
         "Road to Liberation",
         "On 6 September 1885 Bulgaria was united again, starting in Plovdiv.",
         1885,
         2,
         3.0f,
         2.0f,
         0.0f,
         2,
         true,
         false},
        {9,
         "Shipka",
         "Road to Liberation",
         "In 1877 Bulgarian volunteers held Shipka against a far bigger army, then survived a frozen "
         "winter on the peak.",
         1877,
         3,
         2.5f,
         1.5f,
         2.0f,
         3,
         true,
         true}};

    if (id < 1 || id > LEVEL_COUNT) {
        return nullptr;
    }

    return &levels[id - 1];
}

const Arena* getArena(int) { return nullptr; }

const Character* getCharacter(CharacterId) { return nullptr; }

bool isLevelUnlocked(int, bool, bool) { return false; }

bool isCharacterUnlocked(CharacterId, int, bool) { return false; }

bool isArenaUnlocked(int, int, bool) { return false; }
