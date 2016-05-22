#ifndef MINIMAP_H
#define MINIMAP_H

#include <QGraphicsView>
#include <QDebug>
#include <QMap>

class Tilemap
{
    public:
        Tilemap():
            tilemap()
        {
        }
        void addTileset(int id, QMap<int, QImage> tileset)
        {
            tilemap.insert(id, tileset);
        }
        QImage getTile(int set, int tile)
        {
            return tilemap[set][tile];
        }

    private:
        QMap<int, QMap<int, QImage> > tilemap;
};

class Minimap : public QGraphicsView
{
    public:
        Minimap(QWidget * parent = 0);

        void clearMap();
        int randInt(int low, int high);
        void appendMap(QString & stringMap);
        void addTile(int x, int y, int tileSize, int tileSet, int tileNumber);

    private:
        unsigned int layerNumber;

        Tilemap tilemap;
};

namespace MapTile
{
/// It's an empty tile.
const char MapVoid = ' ';
/// It's a wall tile.
const char MapWall = '#';
/// It's a walkable tile.
const char MapWalk = '.';
/// It's the player.
const char MapPlay = '@';
/// It's a mobile.
const char MapMobile = 'M';
/// It's a player.
const char MapPlayer = 'P';
/// It's an UpDown staris.
const char MapUpDownStairs = 'x';
/// It's an Up staris.
const char MapUpStairs = '>';
/// It's a Down staris.
const char MapDownStairs = '<';
/// It's a Pit.
const char MapPit = 'p';
/// It's a piece of equipment.
const char MapEquipment = 'e';
/// It's a generic item.
const char MapItem = 'i';
/// It's a door tile.
const char MapDoor = 'D';
/// It's a door tile.
const char MapDoorOpen = 'd';
}

#endif // MINIMAP_H
