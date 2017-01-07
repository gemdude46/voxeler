#define blk_count 6
#define bt_count 2

#define BTtype int
#define AIR_TYPE 0
#define REG_TYPE 1

STRING B_name[blk_count]     = {"Air",              "Rock",             "Dirt",             "Grass",            "Sand",             "Water"};
BTtype B_type[blk_count]     = {AIR_TYPE,           REG_TYPE,           REG_TYPE,           REG_TYPE,           REG_TYPE,           REG_TYPE};
SColor B_clr1[blk_count]     = {C_NONE,             GREY(130),          RGB(174,166,61),    RGB(83,203,41),     RGB(239,219,155),   RGB(0,0,170)};
SColor B_clr2[blk_count]     = {C_NONE,             GREY(150),          RGB(155,188,83),    RGB(73,223,20),     RGB(253,227,141),   RGB(0,0,140)};

bool BT_opaque[bt_count]     = {false,              true};
bool BT_noclip[bt_count]     = {true,               false};
bool BT_select[bt_count]     = {false,              true};
