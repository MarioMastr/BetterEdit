#include <objects/ObjectType.hpp>
#include <unordered_set>

std::unordered_set ROTATING_OBJ_IDS {
    1705, 1706, 1707,
    1708, 1709, 1710,
    1734, 1735, 1736,
    678, 679, 680,
    186, 187, 188,
    740, 741, 742,
    1619, 1620,
    183, 184, 185,
    85, 86, 87, 97,
    1831, 1832, 1833, 1834,
    1058, 1059, 1060, 1061,
    1752,
    1055, 1056, 1057,
    1019, 1020, 1021,
    997, 998, 999, 1000,
    1521, 1522,
    180, 181, 182,
    154, 155, 156,
    137, 138, 139,
    222, 223, 224,
    1523, 1524, 1525, 1526, 1527, 1528,
    394, 395, 396,
    375, 376, 377, 378,
    // old black 1.9 saws
    675, 676, 677,
    // orbs
    1022, 1330,
};

bool ObjectProps::isRotating(GameObject* obj) {
    return ROTATING_OBJ_IDS.count(obj->m_objectID);
}
