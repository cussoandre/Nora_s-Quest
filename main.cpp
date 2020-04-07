//g++ -o release/NoraQuest main.cpp -Bstatic olcPixelGameEngine.h -lX11 -lGL -lpthread -lpng -lstdc++fs

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <string>

#define MAXENTITIES 150

#define UP olc::Key::W
#define DOWN olc::Key::S
#define LEFT olc::Key::A
#define RIGHT olc::Key::D
#define INTERACT olc::Key::E
#define MAGIC olc::Key::SPACE
#define START olc::Key::TAB

#define PLAYERSPEED 45.0
#define MAGICRADIUS 2
#define COSTMAGIC 15
#define TILEDAMAGE 25

#define widthInTiles 26
#define heightInTiles 16

#define tileWidth 16
#define tileHeight 16

#define pixSizeW 8
#define pixSizeH 8

#define defaultSpritePath "Assets/Sprites/defaultTile.png"
#define barrenTerrainSprite "Assets/Sprites/BarrenGround.png"
#define grassSpriteA "Assets/Sprites/grass_1.png"
#define cyanTestSprite "Assets/Sprites/Cyan-test-2.png"
#define nextLevelSprite "Assets/Sprites/Cyan-test-2.png"

#define TILESNUM 5

#define defaultTile 0
#define barrenTile 1
#define grassTileA 2
#define cyanTestTile 3
#define nextLevelTile 4

#define MAPSNUM 12

#define characterABack "Assets/Sprites/CharacterA-Back-Border.png" //Nora
#define characterAFront "Assets/Sprites/CharacterA-Front-Border.png"
#define characterARSide "Assets/Sprites/CharacterA-RSide-Border.png"
#define characterALSide "Assets/Sprites/CharacterA-LSide-Border.png"

#define FORWARD 0
#define BACKWARD 1
#define LEFTWARD 2
#define RIGHTWARD 3
//0 is front, 1 is back, 2 is left, 3 is right;

struct map
{
    char tileMap[widthInTiles][heightInTiles];
    int startX;
    int startY;
    int startH = 100;
    int startM = 120;
};

struct directionalSprite
{
    olc::Sprite dir[4];
};

class Entity
{
    directionalSprite entitySprite;
    int sizeH = 1;
    int sizeW = 1;
    double x = 292.0;
    double y = 21.0;
    int direction = FORWARD;
    bool isEmpty = false;
    bool alive = true;

    int mana = 100;
    float health = 100;

public:

    int32_t getX() {return x;}
    int32_t getY() {return y;}
    int32_t getXTile() {return x/tileWidth+0.5;}
    int32_t getYTile() {return y/tileHeight+2;}
    int getDir() {return direction;}
    bool getEmpty() {return isEmpty;}
    int getH() {return sizeH;}
    int getW() {return sizeW;}
    int getMana() {return mana;}
    int getHealthInt() {return health;}
    int getHealthNat() {return std::max(getHealthInt(), 0);}

    float getHealth() {return health;}

    void setX(int32_t newX) {x = newX;}
    void setY(int32_t newY) {y = newY;}
    void setDir(int dir) {direction = dir;}
    void setEmpty(bool val) {isEmpty = val;}
    void setHealth(int h){health = h;}
    void setMana(int m){mana = m;}

    void setH(int h) {sizeH = h;}
    void setW(int w) {sizeW = w;}

    void incX(float amount) {x += amount; x = std::max(x, -8.0); x = std::min(x, (widthInTiles*16.0)-10);}
    void incY(float amount) {y += amount; y = std::max(y, -8.0); y = std::min(y, (heightInTiles*16.0)-10);}

    void incMana(int amount) {mana += amount;}
    void incHealth(float amount) {health += amount;}

    bool checkAlive(){alive = health >= 0;return alive;}

    olc::Sprite* getSpriteP(int direction) {return &entitySprite.dir[direction];}

    void setSprite(int direction, olc::Sprite sprite) {entitySprite.dir[direction] = sprite;}
    void setSprite(int direction, olc::Sprite* sprite) {entitySprite.dir[direction] = *sprite;}

    Entity clone()
    {
        Entity* tmp = new Entity();
        tmp->setSprite(0, this->getSpriteP(0));
        tmp->setSprite(1, this->getSpriteP(1));
        tmp->setSprite(2, this->getSpriteP(2));
        tmp->setSprite(3, this->getSpriteP(3));

        tmp->setX(this->getX());
        tmp->setY(this->getY());

        tmp->setEmpty(this->getEmpty());
        tmp->setH(this->getH());
        tmp->setW(this->getW());
        
        return *tmp;
    }

    Entity* cloneToP()
    {
        Entity* tmp = new Entity();
        tmp->setSprite(0, this->getSpriteP(0));
        tmp->setSprite(1, this->getSpriteP(1));
        tmp->setSprite(2, this->getSpriteP(2));
        tmp->setSprite(3, this->getSpriteP(3));

        tmp->setX(this->getX());
        tmp->setY(this->getY());

        tmp->setEmpty(this->getEmpty());
        tmp->setH(this->getH());
        tmp->setW(this->getW());
        
        return tmp;
    }
};

olc::Sprite* groundTiles[TILESNUM];

struct entityList
{
    Entity* entMap[MAXENTITIES];
    entityList()
    {
        for (int i = 0; i < MAXENTITIES; i++)
        {
            entMap[i] = new Entity();
            entMap[i]->setEmpty(true);
        }
    }
};

Entity gem;

Entity player;

entityList eList;

void changeRadius(Entity* pl, map* mapPoint, char tile)
{
    int x = pl->getXTile();
    int y = pl->getYTile();
    for (int i = -MAGICRADIUS; i < MAGICRADIUS; i++)
    {
        for (int j = -MAGICRADIUS; j < MAGICRADIUS; j++)
        {
            if ((x+i>=0) && (x+i<widthInTiles)&&(y+j>=0) && (y+j<heightInTiles) && (mapPoint->tileMap[x+i][y+j] == barrenTile)) mapPoint->tileMap[x+i][y+j] = tile;
        }
    }
}

map mapFromArray(char arr[widthInTiles][heightInTiles])
{
    map tmp;
    for (int i = 0; i < widthInTiles; i++)
        for (int j = 0; j < heightInTiles; j++)
            tmp.tileMap[i][j] = arr[i][j];
    return tmp;
}

map populateMap(char tile)
{
    map tmp;
    for (int j = 0; j < heightInTiles; j++)
        for (int i = 0; i < widthInTiles; i++)
        {
            tmp.tileMap[i][j] = tile;
        }
    return tmp;
}

map populateRandomMap(uint32_t seed, int diff)
{
    map tmp;
    srand(seed);
    int startX = (rand() % (widthInTiles/4));
    int startY = (rand() % (heightInTiles/4));
    for (int j = 0; j < heightInTiles; j++)
        for (int i = 0; i < widthInTiles; i++)
        {
            tmp.tileMap[i][j] = (rand()%diff == diff-1) +1;
        }
    tmp.tileMap[startX][startY] = grassTileA;
    tmp.tileMap[startX][startY+1] = grassTileA;
    tmp.tileMap[startX+1][startY] = grassTileA;
    tmp.tileMap[startX+1][startY+1] = grassTileA;

    tmp.startX = startX * tileWidth;
    tmp.startY = startY * tileHeight;
    tmp.tileMap[(3*widthInTiles/4) + rand() % widthInTiles/4][(3*heightInTiles/4) + (rand() % (heightInTiles/4))] = nextLevelTile;
    return tmp;
}

entityList randomEntityList(int gems)
{
    entityList tmp;
    int lastX = 0;
    int lastY = 0;

    for (int i = 0; i < gems; i++)
    {
        tmp.entMap[i] = gem.cloneToP();
        lastX += rand()*30;
        tmp.entMap[i]->setX(lastX);
        lastY += rand()*30;
        tmp.entMap[i]->setY(lastY);
    }
    return tmp;
}

map titleScreen;
map firstLevel, secondLevel, thirdLevel, fourthLevel, fifthLevel, sixthLevel, seventhLevel, eightLevel, ninthLevel, tenthLevel;

map winningLevel;

char tileLevelTwo[widthInTiles][heightInTiles]=
{
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 2, 1, 1, 1, 1, 2, 1, 2, 2, 1, 1, 1, 1, 1},
    {1, 2, 1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1},
    {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 1, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1},
    {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 2, 2, 2, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};


olc::Sprite* grassSpriteALoaded;

map* playingMap;
map* allMaps[MAPSNUM];
int currentMapNum = 0;

class RPG : public olc::PixelGameEngine
{
    //map playingMap;
    bool isTitleScreenDisplayed = true;

    void updatePlayingMap(Entity* pl, map* newMap)
    {
        playingMap = newMap;
        pl->setX(newMap->startX);
        pl->setY(newMap->startY);
        pl->setHealth(newMap->startH);
        pl->setMana(newMap->startM);
    }

    void drawMap(map* currentMap)
    {
        for (int x = 0; x < widthInTiles; x++)
			for (int y = 0; y < heightInTiles; y++)
                DrawSprite(x*tileWidth, y*tileHeight, groundTiles[currentMap->tileMap[x][y]]);
    }

    void addEntity(Entity* ent)
    {
        SetPixelMode(olc::Pixel::Mode::ALPHA);
        DrawSprite(ent->getX(), ent->getY(), ent->getSpriteP(ent->getDir()));
        SetPixelMode(olc::Pixel::Mode::NORMAL);
    }

    void drawEnts(entityList ents)
    {
        for (int i = 0; i < MAXENTITIES; i++)
        {
            addEntity(ents.entMap[i]->cloneToP());
        }
    }

    void movePlayer(Entity* pl, float time)
    {
        int direction = 0; //0 is front, 1 is back, 2 is left, 3 is right;

        //int currentX = pl->getX();
        //int currentY = pl->getY();;

        if (GetKey(UP).bHeld)
        {
            pl->incY (-PLAYERSPEED * time);
            pl->setDir (BACKWARD);
        }
        if (GetKey(DOWN).bHeld)
        {
            pl->incY (PLAYERSPEED * time);
            pl->setDir (FORWARD);
        }
        if (GetKey(RIGHT).bHeld)
        {
            pl->incX (PLAYERSPEED * time);
            pl->setDir (RIGHTWARD);
        }
        if (GetKey(LEFT).bHeld)
        {
            pl->incX (-PLAYERSPEED * time);
            pl->setDir (LEFTWARD);
        }


        addEntity(pl);
        //pl->setX (currentX);
        //pl->setY (currentY);
    }
    
public:
	RPG()
	{
		sAppName = "Nora's Quest";
	}

public:
	bool OnUserCreate() override
	{
        playingMap = &titleScreen;
        allMaps[0] = &titleScreen;
        allMaps[1] = &firstLevel;
        allMaps[2] = &secondLevel;
        allMaps[3] = &thirdLevel;
        allMaps[4] = &fourthLevel;
        allMaps[5] = &fifthLevel;
        allMaps[6] = &sixthLevel;
        allMaps[7] = &seventhLevel;
        allMaps[8] = &eightLevel;
        allMaps[9] = &ninthLevel;
        allMaps[10] = &tenthLevel;

        allMaps[MAPSNUM-1] = &winningLevel;

		// Called once at the start, so create things here
        groundTiles[grassTileA] = new olc::Sprite(grassSpriteA);
        groundTiles[barrenTile] = new olc::Sprite(barrenTerrainSprite);
        groundTiles[cyanTestTile] = new olc::Sprite(cyanTestSprite);
        groundTiles[nextLevelTile] = new olc::Sprite (nextLevelSprite);

        titleScreen = populateMap(grassTileA);
        titleScreen.startX = 292;
        titleScreen.startY = 21;

        /*
        firstLevel = populateMap(barrenTile);
        firstLevel.startX = 32;
        firstLevel.startY = 32;
        firstLevel.tileMap[22][12] = nextLevelTile;
        */

        firstLevel = populateRandomMap(1011980, 4);

        secondLevel = mapFromArray(tileLevelTwo);

        thirdLevel = populateRandomMap(15031999, 6);
        
        fourthLevel = populateRandomMap(17032002, 7);

        fifthLevel = populateRandomMap(25051998, 8);

        sixthLevel = populateRandomMap(4031943, 9);
        
        seventhLevel = populateRandomMap(4051930, 10);

        eightLevel = populateRandomMap(12345678, 15);

        ninthLevel = populateRandomMap(88888888, 15);
        
        tenthLevel = populateRandomMap(22233311, 20);

        winningLevel = populateMap(cyanTestTile);

        player.setH(2);

        player.setSprite (RIGHTWARD, new olc::Sprite(characterARSide));
        player.setSprite (LEFTWARD, new olc::Sprite(characterALSide));
        player.setSprite (BACKWARD, new olc::Sprite(characterABack));
        player.setSprite (FORWARD, new olc::Sprite(characterAFront));

        gem.setSprite (FORWARD, new olc::Sprite("Assets/Sprites/Gem-A.png"));

        eList = randomEntityList(32);


        return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
        playingMap = allMaps[currentMapNum];
		drawMap (playingMap);
        if (player.checkAlive()) movePlayer(&player, fElapsedTime);
        if (isTitleScreenDisplayed)
        {
            DrawString(30, 30, "NORA'S QUEST", olc::VERY_DARK_MAGENTA, 4);
            DrawString(50,70, "a game by aki", olc::YELLOW, 2);
            DrawString(75, 90, "created using the \"Pixel Game Engine\" \n by OneLoneCoder (NOT AFFILIATED)");
            DrawString(0, 140, ">>> PRESS TAB TO START <<<", olc::WHITE, 2);
        }
        else
        {
            DrawString(0, 0, "Mana  : \nHealth:", olc::VERY_DARK_MAGENTA, 1);
            DrawString(64, 0, std::to_string(player.getMana()) + "\n" + std::to_string(player.getHealthNat()), olc::WHITE, 1);
        }
        drawEnts(eList);
        

        if (GetKey(START).bReleased && isTitleScreenDisplayed)
        {
            isTitleScreenDisplayed = false;
            updatePlayingMap(&player, allMaps[++currentMapNum]);
        }

        if (player.checkAlive() && GetKey(MAGIC).bPressed && player.getMana() >= COSTMAGIC)
        {
            player.incMana(-COSTMAGIC);
            changeRadius(&player, playingMap, grassTileA);
        }

        if (playingMap->tileMap[player.getXTile()][player.getYTile()] == barrenTile) player.incHealth(-TILEDAMAGE * fElapsedTime);
        if (playingMap->tileMap[player.getXTile()][player.getYTile()] == nextLevelTile) updatePlayingMap(&player, allMaps[++currentMapNum]);
        if (currentMapNum == MAPSNUM -1)
        {
            DrawString(30, 30, "HEY! YOU WON!!!", olc::DARK_MAGENTA, 4);
            DrawString(90, 130, "Press Tab to exit", olc::WHITE, 1);
            if (GetKey(START).bPressed) return false;;
        }
        if (!player.checkAlive())
        {
            DrawString(30, 30, "SORRY: \nYOU LOST", olc::VERY_DARK_MAGENTA, 4);
            DrawString(90, 130, "Press Tab to exit", olc::WHITE, 1);
            if (GetKey(START).bPressed) return false;;
        }
            
		return true;
	}
};


int main()
{
	RPG NoraQuest;
	if (NoraQuest.Construct(widthInTiles*tileWidth, heightInTiles*tileHeight, pixSizeW, pixSizeH))
		NoraQuest.Start();

	return 0;
}
