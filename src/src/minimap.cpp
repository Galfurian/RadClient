#include "../inc/minimap.h"

#include <QGraphicsRectItem>
#include <iostream>
#include <stdio.h>
#include <QFile>

Minimap::Minimap(QWidget * parent):
    QGraphicsView(parent),
    layerNumber(0)
{
    this->setBackgroundBrush(QBrush(QColor(10, 10, 10), Qt::SolidPattern));
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    int tileSize = 32;

    QFile inputFile(":/resources/tileset/config.txt");
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if(!line.isEmpty())
            {
                int tileSetId       = line.mid(0,line.lastIndexOf(":")).toInt();
                QString tileSetName = line.mid(line.lastIndexOf(":") + 1);
                QImage tilesetImage = QImage(":/resources/tileset/"+tileSetName);
                QMap<int, QImage> inputTileset;
                int stopWidth  = tilesetImage.width() - tileSize;
                int stopHeight = tilesetImage.height() - tileSize;
                int tileId = 0;
                for (int row = 0; row <= stopHeight; row += tileSize)
                {
                    for (int column = 0; column <= stopWidth; column += tileSize)
                    {
                        inputTileset.insert(tileId, tilesetImage.copy(column, row, tileSize, tileSize));
                        tileId++;
                    }
                }
                tilemap.addTileset(tileSetId, inputTileset);
            }
        }
        inputFile.close();
    }
}

void Minimap::clearMap()
{
    // Delete previous scene
    delete this->scene();
}

int Minimap::randInt(int low, int high)
{
    // Random number between low and high
    return qrand() % ((high + 1) - low) + low;
}

void Minimap::appendMap(QString & stringMap)
{
    if(!this->scene())
    {
        layerNumber = 0;
        this->setScene(new QGraphicsScene(this));
    }
    else
    {
        layerNumber++;
    }

    // Prepare the matrix which will contain the tiles number.
    QVector<QVector<QPair<int, int> > > tileNumberMap;
    // Prepare matching regular expressions.
    QRegExp commaRx(",");
    QRegExp semicolonRx(";");
    // Retrieve the list of strings which describe each rows.
    QStringList splittedRows = stringMap.split(semicolonRx, QString::SkipEmptyParts);
    for(auto it : splittedRows)
    {
        QStringList splittedCells = it.split(commaRx, QString::SkipEmptyParts);
        QVector<QPair<int, int> > row;
        for(auto cell : splittedCells)
        {
            QString strTileSet = cell.mid(0, cell.lastIndexOf(":"));
            QString strTileId  = cell.mid(cell.lastIndexOf(":") + 1);
            int tileSet = strTileSet.toInt();
            int tileId  = strTileId.toInt();
            row.append(qMakePair(tileSet, tileId));
        }
        tileNumberMap.append(row);
    }

    // Determine the size of tiles.
    int adjustedTileSize = (this->size().width() / tileNumberMap.size());
    for(int numRow = 0; numRow < tileNumberMap.size(); ++numRow)
    {
        QVector<QPair<int, int> > row = tileNumberMap.at(numRow);
        for(int numCell = 0; numCell < row.size(); ++numCell)
        {
            int tileSet = row.at(numCell).first;
            int tileId  = row.at(numCell).second;
            if((tileSet != 0) || (tileId != 0))
            {
                this->addTile(numCell, numRow, adjustedTileSize, tileSet, tileId);
            }
        }
    }
}

void Minimap::addTile(int x, int y, int tileSize, int tileSet, int tileId)
{
    QGraphicsRectItem * tile = new QGraphicsRectItem(x * tileSize, y * tileSize, tileSize, tileSize);
    QImage tileImage = tilemap.getTile(tileSet, tileId).copy();
    tileImage = tileImage.scaled(tileSize, tileSize, Qt::KeepAspectRatio);
    QBrush brush(tileImage);
    tile->setBrush(brush);
    this->scene()->addItem(tile);
}
