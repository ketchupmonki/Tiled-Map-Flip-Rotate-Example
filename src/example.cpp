/*
MIT License

Copyright (c) 2019 Alexander Theulings

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <SDL2/SDL.h>

//Settings for our SDL window
int windowWidth = 640;
int windowHeight = 480;
const char windowName[] = "SDL2 Tiled Map Rotation Example";

//The tile flip flags, taken from the example at https://docs.mapeditor.org/en/stable/reference/tmx-map-format/#tile-flipping
const unsigned tiledHorizontal = 0x80000000;
const unsigned tiledVertical   = 0x40000000;
const unsigned tiledDiagonal   = 0x20000000;

//Our map
const int mapWidth = 9;
const uint32_t tiledMap[] = {
        0,0,0,0,0,0,0,0,0,
        0,1,0,2684354561,0,3221225473,0,1610612737,0,
        0,0,0,0,0,0,0,0,0,
        0,1073741825,0,536870913,0,2147483649,0,3758096385,0,
        0,0,0,0,0,0,0,0,0,
        0,2,0,2684354562,0,3221225474,0,1610612738,0,
        0,0,0,0,0,0,0,0,0,
        0,1073741826,0,536870914,0,2147483650,0,3758096386,0,
        0,0,0,0,0,0,0,0,0,
        0,3,0,2684354563,0,3221225475,0,1610612739,0,
        0,0,0,0,0,0,0,0,0,
        0,1073741827,0,536870915,0,2147483651,0,3758096387,0,
        0,0,0,0,0,0,0,0,0,
        0,536870913,2684354561,3,2147483650,2147483651,1610612737,3758096385,0,
        0,0,0,0,0,0,0,0,0
};

//Our tile sheet
SDL_Texture* sheet;
#define TILE_WIDTH 32
#define TILE_HEIGHT 32

//When rotating we need to set a point to pivot around.
SDL_Point centre = {TILE_WIDTH / 2, TILE_HEIGHT / 2};

//Our image source and destination rectangles both adopt the width and height of the tiles as we don't want to scale our images.
SDL_Rect srcRect = {0, 0, TILE_WIDTH, TILE_HEIGHT};
SDL_Rect destRect = {0, 0, TILE_WIDTH, TILE_HEIGHT};

//SDL 
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Event event;

int main(int argc, char* argv[]){
    //Initialize SDL2 and create a window
    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        SDL_Log("SDL init error: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow(windowName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, 0);
    if(window == NULL){
        SDL_Log("SDL create renderer error %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if(renderer == NULL){
        SDL_Log("SDL create renderer error %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    //Load our tile sheet
    SDL_Surface* tmpSurf;
    tmpSurf = SDL_LoadBMP("tileset.bmp");
    if(tmpSurf == NULL){
        SDL_Log("SDL load image error %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    sheet = SDL_CreateTextureFromSurface(renderer, tmpSurf);
    SDL_FreeSurface(tmpSurf);

    //Cycle through a loop that renders our 'map' and checks for a request to close the window
    bool run = true;
    while(run){
        //Check if we need to close the window
        while(SDL_PollEvent(&event) != 0){
            if (event.window.event == SDL_WINDOWEVENT_CLOSE){
                run = false;
            }
        }

        //Go through the map and render tiles
        //On a proper project we would make this loop only go though tiles visible on the screen, rather than every single tile on the map
        for(int i = 0; i < sizeof(tiledMap); i++){
            //Only proccess the position if there is a tile placed there
            if (tiledMap[i] == 0){
                continue;
            }

            //Get the status of the bit address assigned for flipping
            bool H = tiledMap[i] & tiledHorizontal;
            bool V = tiledMap[i] & tiledVertical;
            bool D = tiledMap[i] & tiledDiagonal;

            //We then use the bitwise operator to resolve the tile ID without the flipping bits
            //This ID tells us the position of our tile in the tile tile sheet
            uint32_t tileID = tiledMap[i];
            tileID &= ~(tiledHorizontal | tiledVertical | tiledDiagonal);
            srcRect.x = (tileID - 1) * TILE_WIDTH;

            //Now we run though our 'TMX Map Rotation' table
            //https://ketchupcomputing.com/tmxRot
            double rotate = 0;
            SDL_RendererFlip flip = SDL_FLIP_NONE;
 
            if(!H && V && D){
                rotate = 90;
            }

            if(H && V && !D){
                rotate = 180;
            }

            if(H && !V && D){
                rotate = 270;
            }

            if(!H && V && !D){
                flip = SDL_FLIP_VERTICAL;
            }

            if(H && V && D){
                flip = SDL_FLIP_VERTICAL;
                rotate = 90;
            }

            if(H && !V && !D){
                flip = SDL_FLIP_HORIZONTAL;
            }

            if(!H && !V && D){
                flip = SDL_FLIP_HORIZONTAL;
                rotate = 90;
            }

            //Our rotation on SDL is going counterclockwise so lets adjust our rotation to match
            rotate *= -1;
            
            //Set the destination rectangle to our tiles position on the map 
            destRect.y = (i / mapWidth) * TILE_HEIGHT;
            destRect.x = (i - ((i / mapWidth) * mapWidth)) * TILE_WIDTH;

            //Copy the image to the renderer
            SDL_RenderCopyEx(renderer, sheet, &srcRect, &destRect, rotate, &centre, flip);
        }

        //Render everything to the screen
        SDL_RenderPresent(renderer);

        //Delay so the program doesn't eat our CPU. 
        //Nothing is moving so it need not have a high fps...
        SDL_Delay(100);
    }
    
    //Clean up after ourselves and terminate the program
    SDL_DestroyTexture(sheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    exit(EXIT_SUCCESS);
}
