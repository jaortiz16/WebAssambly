#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <emscripten.h>
#include <cstdlib>
#include <iostream>

struct context
{
    SDL_Renderer *renderer;
    TTF_Font *font; 
    int ballX, ballY;
    int ballSpeedX, ballSpeedY;
    int paddle1Y, paddle2Y;
    bool movePaddleUp, movePaddleDown;
    int player1Score, player2Score;
    bool gameOver;
};

const int screenWidth = 640;
const int screenHeight = 480;
const int paddleWidth = 10;
const int paddleHeight = 60;
const int ballSize = 10;
const int paddleSpeed = 5;

void updateGame(context *ctx)
{
    if (ctx->gameOver)
        return;


    ctx->ballX += ctx->ballSpeedX;
    ctx->ballY += ctx->ballSpeedY;


    if (ctx->ballY <= 0 || ctx->ballY >= screenHeight - ballSize)
        ctx->ballSpeedY = -ctx->ballSpeedY;


    if (ctx->ballX <= paddleWidth && ctx->ballY >= ctx->paddle1Y && ctx->ballY <= ctx->paddle1Y + paddleHeight)
        ctx->ballSpeedX = -ctx->ballSpeedX;

    if (ctx->ballX >= screenWidth - paddleWidth - ballSize && ctx->ballY >= ctx->paddle2Y && ctx->ballY <= ctx->paddle2Y + paddleHeight)
        ctx->ballSpeedX = -ctx->ballSpeedX;

    if (ctx->movePaddleUp)
        ctx->paddle1Y -= paddleSpeed;
    if (ctx->movePaddleDown)
        ctx->paddle1Y += paddleSpeed;
    ctx->paddle2Y = ctx->ballY - paddleHeight / 2;

    if (ctx->ballX <= 0)
    {
        ctx->player2Score++;
        ctx->gameOver = true;
        std::cout << "Player 2 wins! Press 'R' to restart." << std::endl;
    }

    if (ctx->ballX >= screenWidth - ballSize)
    {
        ctx->player1Score++;
        ctx->gameOver = true;
        std::cout << "Player 1 wins! Press 'R' to restart." << std::endl;
    }
}

void renderGame(context *ctx)
{

    SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
    SDL_RenderClear(ctx->renderer);


    SDL_Rect paddle1 = {0, ctx->paddle1Y, paddleWidth, paddleHeight};
    SDL_Rect paddle2 = {screenWidth - paddleWidth, ctx->paddle2Y, paddleWidth, paddleHeight};
    SDL_SetRenderDrawColor(ctx->renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(ctx->renderer, &paddle1);
    SDL_RenderFillRect(ctx->renderer, &paddle2);


    SDL_Rect ball = {ctx->ballX, ctx->ballY, ballSize, ballSize};
    SDL_RenderFillRect(ctx->renderer, &ball);


    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect scoreRect = {10, 10, 0, 0};
    std::string player1ScoreText = "Player 1: " + std::to_string(ctx->player1Score);
    surface = TTF_RenderText_Solid(ctx->font, player1ScoreText.c_str(), textColor);
    texture = SDL_CreateTextureFromSurface(ctx->renderer, surface);
    SDL_QueryTexture(texture, NULL, NULL, &scoreRect.w, &scoreRect.h);
    SDL_RenderCopy(ctx->renderer, texture, NULL, &scoreRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    std::string player2ScoreText = "Player 2: " + std::to_string(ctx->player2Score);
    scoreRect.x = screenWidth - 150;
    surface = TTF_RenderText_Solid(ctx->font, player2ScoreText.c_str(), textColor);
    texture = SDL_CreateTextureFromSurface(ctx->renderer, surface);
    SDL_QueryTexture(texture, NULL, NULL, &scoreRect.w, &scoreRect.h);
    SDL_RenderCopy(ctx->renderer, texture, NULL, &scoreRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    SDL_RenderPresent(ctx->renderer);
}

void handleInput(SDL_Event event, context *ctx)
{
    switch (event.type)
    {
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym)
        {
        case SDLK_w:
            ctx->movePaddleUp = true;
            break;
        case SDLK_s:
            ctx->movePaddleDown = true;
            break;
        case SDLK_r:
            if (ctx->gameOver)
            {

                ctx->ballX = screenWidth / 2 - ballSize / 2;
                ctx->ballY = screenHeight / 2 - ballSize / 2;
                ctx->ballSpeedX = 5;
                ctx->ballSpeedY = 3;
                ctx->paddle1Y = screenHeight / 2 - paddleHeight / 2;
                ctx->paddle2Y = screenHeight / 2 - paddleHeight / 2;
                ctx->movePaddleUp = false;
                ctx->movePaddleDown = false;
                ctx->gameOver = false;
                std::cout << "Game restarted!" << std::endl;
            }
            break;
        }
        break;
    case SDL_KEYUP:
        switch (event.key.keysym.sym)
        {
        case SDLK_w:
            ctx->movePaddleUp = false;
            break;
        case SDLK_s:
            ctx->movePaddleDown = false;
            break;
        }
        break;
    }
}

void mainloop(void *arg)
{
    context *ctx = static_cast<context *>(arg);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        handleInput(event, ctx);
    }

    updateGame(ctx);
    renderGame(ctx);
}

int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_CreateWindowAndRenderer(screenWidth, screenHeight, 0, &window, &renderer);

    context ctx;
    ctx.renderer = renderer;
    ctx.ballX = screenWidth / 2 - ballSize / 2;
    ctx.ballY = screenHeight / 2 - ballSize / 2;
    ctx.ballSpeedX = 5;
    ctx.ballSpeedY = 3;
    ctx.paddle1Y = screenHeight / 2 - paddleHeight / 2;
    ctx.paddle2Y = screenHeight / 2 - paddleHeight / 2;
    ctx.movePaddleUp = false;
    ctx.movePaddleDown = false;
    ctx.player1Score = 0;
    ctx.player2Score = 0;
    ctx.gameOver = false;

    TTF_Init();
    ctx.font = TTF_OpenFont("arial.ttf", 24);

    const int simulate_infinite_loop = 1; 
    const int fps = -1;                 
    emscripten_set_main_loop_arg(mainloop, &ctx, fps, simulate_infinite_loop);

    TTF_CloseFont(ctx.font);
    TTF_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
