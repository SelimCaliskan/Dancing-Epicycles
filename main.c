#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include "include/raylib.h"
#include "include/raygui.h"
#define TITLE "The Milky Way"
#define DEFAULT_WIDTH 1200
#define DEFAULT_HEIGHT 800
#define MAX_TRAIL_LENGTH 100000
#define TARGET_FPS 120

struct Circle{
    int X;
    int Y;
    float radius;
    int w;
    Color color;
    float rodLength;
    float phase;
};

struct LList{
    struct Circle data;
    struct LList* prev;
    struct LList* next;
};

typedef struct LList* Node;

struct Complex{
    float re;
    float im;
};

struct Epicycle{
    struct Complex complx;
    int freq;
    float amp;
    float phase;
};

enum State{
    USER = 0,
    FOURIER = 1
};

struct Complex drawing[MAX_TRAIL_LENGTH];
struct Epicycle path[MAX_TRAIL_LENGTH];
int drawing_index = 0;
int trail[MAX_TRAIL_LENGTH][2];
int trail_index = 0;
enum State state = USER;


Node CreateNode();

void AddCircle(Node head, float radius, int w, Color color, float rodLength, float phase);

void RemoveCircle(Node head);

void RemoveAllCircles(Node head);

void SimulateFrame(Node head, float time_taken);

void DrawLines(Node head);

void DrawCircles(Node head, enum State state);

void DrawTrail(Node head, bool toggle);

struct Complex multiply(struct Complex z1, struct Complex z2);

struct Complex add(struct Complex z1, struct Complex z2);

void Fourier_Transform();

int amp_comparator(const void *v1, const void *v2);


int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------

    int WIDTH = 0;
    int HEIGHT = 0;
    InitWindow(WIDTH, HEIGHT, TITLE);

    ToggleFullscreen();
    const int MONITOR_WIDTH = GetScreenWidth();
    const int MONITOR_HEIGHT = GetScreenHeight();
    
    ToggleFullscreen();
    WIDTH = DEFAULT_WIDTH;
    HEIGHT = DEFAULT_HEIGHT;
    SetWindowSize(WIDTH, HEIGHT);
    SetWindowPosition(MONITOR_WIDTH/2 - DEFAULT_WIDTH/2, MONITOR_HEIGHT/2 - DEFAULT_HEIGHT/2);

    SetTargetFPS(TARGET_FPS);               // Set our pendulum to run at 120 frames-per-second

    Node head = CreateNode(); //initialize the first (rigid) node of the pendulum
    head->data = (struct Circle){WIDTH/2, HEIGHT/2, 5.f, 5, GRAY, 50.f, 0.f};

    float time_taken = 0;
    bool toggleTrail = true;

    Color colors[4] = {RED, BLUE, GREEN, VIOLET};

    //--------------------------------------------------------------------------------------
    
    // Main loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here

        srand(((int)(time_taken * 1000)) % INT_MAX);

        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            toggleTrail = true;
            trail_index = 0;
            drawing_index = 0;
            state = USER;
        }

        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
            
            Fourier_Transform(head);

            RemoveAllCircles(head);

            head->data.radius = path[0].amp;
            head->data.w = path[0].freq;
            head->data.phase = path[0].phase;
            head->data.rodLength = path[0].amp;
            head->data.color = colors[rand()%4];
            
            for(int i = 1; i < drawing_index; i++){

                float radius = path[i].amp;
                int w = path[i].freq;
                float rodLength = path[i].amp;
                float phase = path[i].phase;
                AddCircle(head, radius, w, colors[rand()%4], rodLength, phase);

            }

            AddCircle(head, 0, 0, WHITE, 0.f, 0.f);

            trail_index = 0;
            time_taken = 0;
            state = FOURIER;
        }
        
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){

            int x = GetMouseX();
            int y = GetMouseY();

            if(trail_index == 0){

                trail[trail_index][0] = x;
                trail[trail_index][1] = y;
                drawing[drawing_index].re = (float)(x - head->data.X);
                drawing[drawing_index].im = (float)(head->data.Y - y);
                trail_index++;
                drawing_index++;

            }
            else if( !((trail[trail_index - 1][0] == x) && (trail[trail_index - 1][1] == y)) ){

                trail[trail_index][0] = x;
                trail[trail_index][1] = y;
                drawing[drawing_index].re = (float)(x - head->data.X);
                drawing[drawing_index].im = (float)(head->data.Y - y);
                trail_index++;
                drawing_index++;

            }
            
        }
        
        else if(IsKeyPressed(KEY_ENTER)){

            int w = rand() % 13 - 6;
            Color color = colors[rand() % 4];
            float rodLength = (float)(rand() % 50 + 40);
            float radius = (float)(rand() % 15) + 8.f;
            float phase = (float)(rand() % 2 * PI);
            AddCircle(head, radius, w, color, rodLength, phase);
            trail_index = 0;
            state = USER;

        }
        else if(IsKeyPressed(KEY_BACKSPACE)){
            RemoveCircle(head);
            trail_index = 0;
            state = USER;
        }
        else if(IsKeyPressed(KEY_R)){
            RemoveAllCircles(head);
            trail_index = 0;
        }
        else if(IsKeyPressed(KEY_SPACE)){
            toggleTrail ^= 1;
            trail_index = 0;
        }
        else if(IsKeyPressed(KEY_F11)){

            if(IsWindowFullscreen()){
                ToggleFullscreen();
                WIDTH = DEFAULT_WIDTH;
                HEIGHT = DEFAULT_HEIGHT;
                SetWindowSize(WIDTH, HEIGHT);
                SetWindowPosition(MONITOR_WIDTH/2 - WIDTH/2, MONITOR_HEIGHT/2 - HEIGHT/2);
            }
            else{
                WIDTH = MONITOR_WIDTH;
                HEIGHT = MONITOR_HEIGHT;
                SetWindowSize(WIDTH, HEIGHT);
                ToggleFullscreen();
            }

            head->data.X = WIDTH/2;
            head->data.Y = HEIGHT/2;
            trail_index = 0;
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();


            ClearBackground(BLACK);

            DrawText("[LEFT MOUSE BUTTON] - Hold & drag to draw your own!", 10, 10, 20, WHITE);
            DrawText("[ENTER] - Add a random circle", 10, 10 + 25, 20, WHITE);
            DrawText("[BACKSPACE] - Remove the last circle", 10, 10 + 25*2, 20, WHITE);
            DrawText("[R] - Remove all circles", 10, 10 + 25*3, 20, WHITE);
            DrawText("[SPACE] - Toggle trail", 10, 10 + 25*4, 20, WHITE);
            DrawText("[F11] - Toggle fullscreen", 10, 10 + 25*5, 20, WHITE);
            DrawText("[ESC] - Exit the program", 10, 10 + 25*6, 20, WHITE);

            if(!IsMouseButtonDown(MOUSE_BUTTON_LEFT)){

                SimulateFrame(head, time_taken);
            
                DrawLines(head);
                DrawCircles(head, state);

            }
			
			DrawTrail(head, toggleTrail);
			

        EndDrawing();
        //----------------------------------------------------------------------------------

        float dt;

        if(state == FOURIER)
            dt = 2 * PI / drawing_index;

        if(state == USER)
            dt = (1000.f / TARGET_FPS) / 1000.f;

        if(time_taken >= 2*PI){
            
            if(state == FOURIER)
                trail_index = 0;
            
            time_taken = 0;

        }

        time_taken += dt;

    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

Node CreateNode(){
    Node temp; // declare a node
    temp = (Node)malloc(sizeof(struct LList)); // allocate memory using malloc()
    temp->next = NULL;// make next point to NULL
    temp->prev = NULL;// make previous point to NULL
    return temp;//return the new node
}

void AddCircle(Node head, float radius, int w, Color color, float rodLength, float phase){
    Node p = head;
    Node temp = CreateNode();
    temp->data = (struct Circle){0, 0, radius, w, color, rodLength, phase};

    while(p->next != NULL) p = p->next;
    temp->prev = p;
    p->next = temp;
}

void RemoveCircle(Node head){
    if(head->next == NULL){ RemoveAllCircles(head); return;}

    Node p = head;
    Node temp;

    while(p->next != NULL) p = p->next;
    temp = p;
    p = p->prev;
    p->next = NULL;
    free(temp);
}

void RemoveAllCircles(Node head){

    head->data.radius = 5.f;
    head->data.w = 5;
    head->data.color = GRAY;
    head->data.rodLength = 50.f;
    head->data.phase = 0.f;

    while(head->next != NULL) RemoveCircle(head);

}

void SimulateFrame(Node head, float time_taken){
    Node p = head->next;
    while(p != NULL){
        float angle = (float)(p->prev->data.w * time_taken + p->prev->data.phase);
        p->data.X = p->prev->data.X + (int)(p->prev->data.rodLength * sin(angle));
        p->data.Y = p->prev->data.Y + (int)(p->prev->data.rodLength * cos(angle));

        if(p->next == NULL){
            trail[trail_index][0] = p->data.X;
            trail[trail_index][1] = p->data.Y;
            trail_index = (trail_index + 1) % MAX_TRAIL_LENGTH; 
        }
        
        p = p->next;
    }
}

void DrawLines(Node head){
    Node p = head;
    while(p->next != NULL){
        DrawLine(p->data.X, p->data.Y, p->next->data.X, p->next->data.Y, WHITE);
        p = p->next;
    }
}

void DrawCircles(Node head, enum State state){
    Node p = head;
    while(p != NULL){
        DrawCircleLines(p->data.X, p->data.Y, p->data.radius, p->data.color);
        p = p->next;
    }
}

void DrawTrail(Node head, bool toggle){
    if(toggle != true) return;

    for(int i = 1; i < trail_index; i++){
        DrawLine(trail[i-1][0], trail[i-1][1], trail[i][0], trail[i][1], YELLOW);
    }
    
}

struct Complex multiply(struct Complex z1, struct Complex z2){
    float real = (z1.re * z2.re) - (z1.im * z2.im);
    float imag = (z1.re * z2.im) + (z1.im * z2.re);
    
    struct Complex z = (struct Complex){real, imag};

    return z;
}

struct Complex add(struct Complex z1, struct Complex z2){
    float real = z1.re + z2.re;
    float imag = z1.im + z2.im;
    
    struct Complex z = (struct Complex){real, imag};

    return z;
}

void Fourier_Transform(){

    int N = drawing_index;

    for(int k = 0; k < N; k++){

        struct Complex z = (struct Complex){0.f, 0.f};

        for(int n = 0; n < N; n++){

            float phi = (2 * PI * k * n) / N;
            struct Complex t = (struct Complex){(float)cos(phi), (float)-sin(phi)};

            z = add(z, multiply(drawing[n], t));

        }

        z.re /= (float)N;
        z.im /= (float)N;
        path[k].complx = z;
        path[k].freq = k;
        path[k].amp = (float)sqrt(z.re * z.re + z.im * z.im);
        path[k].phase = (float)(atan2(z.im, z.re) + PI/2) ;

        printf("x: %f y: %f, re: %f im: %f freq: %d amp: %f phase: %f\n", drawing[k].re, drawing[k].im, path[k].complx.re, path[k].complx.im, path[k].freq, path[k].amp, path[k].phase);

    }

    qsort(path, drawing_index, sizeof(struct Epicycle), amp_comparator);

}

int amp_comparator(const void *v1, const void *v2)
{
    const struct Epicycle *p1 = (struct Epicycle *)v1;
    const struct Epicycle *p2 = (struct Epicycle *)v2;
    if (p1->amp > p2->amp)
        return -1;
    else if (p1->amp < p2->amp)
        return +1;
    else
        return 0;
}
