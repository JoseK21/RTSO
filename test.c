#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>

// ---------------------------------------------------------------------------
// Header of DATA module
// ---------------------------------------------------------------------------
typedef struct
{
    ALLEGRO_MUTEX *mutex;
    ALLEGRO_COND *cond;
    int modifyWhich; // which variable to modify, 'X' or 'Y'
    float posiX;
    float posiY;
    bool ready;
} DATA;

#define DATA_NEWINIT (    \
    (DATA){               \
        .mutex = NULL,    \
        .cond = NULL,     \
        .posiX = 96,     \
        .posiY = 224,     \
        .modifyWhich = 0, \
        .ready = false})

DATA *DATA_new(void);
void DATA_delete(DATA *self);
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Header of MAIN module
// ---------------------------------------------------------------------------
const float FPS = 30.0;
const int SCREEN_W = 608;
const int SCREEN_H = 608;
#define BLACK (al_map_rgb(0, 0, 0))
#define MAGENTA 255, 0, 255
#define POINT 0, 0, 0
static void *Func_Thread(ALLEGRO_THREAD *thr, void *arg);
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Header of EVENT HANDLER Module
// ---------------------------------------------------------------------------
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define CODE_CONTINUE (MIN(EXIT_SUCCESS, EXIT_FAILURE) - 1)
#define CODE_SUCCESS EXIT_SUCCESS
#define CODE_FAILURE EXIT_FAILURE
typedef int code;
int new_martian = 0;
int energy = 0;
int period = 0;
code HandleEvent(ALLEGRO_EVENT ev);
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Header of REDRAW HANDLER Module
// ---------------------------------------------------------------------------
bool REDRAW_IS_READY;
//void RedrawSetReady(void);
//void RedrawClearReady(void);
bool RedrawIsReady(void);
void RedrawDo(); // to be defined in Implementation of EVENT HANDLER Module
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Implementation of MAIN Module
// ---------------------------------------------------------------------------
int main()
{
    // Initialize Allegro, mouse, timer, display and bitmap
    // XXX - skimping on Allegro error-checking
    al_init();
    if (!al_install_keyboard())
    {
        printf("couldn't initialize keyboard\n");
        return 1;
    }
    if (!al_init_image_addon())
    {
        printf("couldn't initialize image addon\n");
        return 1;
    }
    ALLEGRO_FONT *font = al_create_builtin_font();
    if (!font)
    {
        printf("couldn't initialize font\n");
        return 1;
    }

    ALLEGRO_TIMER *timer = al_create_timer(1.0 / FPS);
    ALLEGRO_DISPLAY *display = al_create_display(SCREEN_W, SCREEN_H);
    ALLEGRO_BITMAP *bouncer = al_load_bitmap("../src/alien.png");
    ALLEGRO_BITMAP *maze = al_load_bitmap("../src/maze.png");

    al_set_target_bitmap(bouncer);
    al_set_target_bitmap(al_get_backbuffer(display));

    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    /* al_register_event_source(event_queue, al_get_mouse_event_source()); */
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_clear_to_color(BLACK);

    al_flip_display();

    DATA *data = DATA_new();  // Crear nuevo marciano
    DATA *data1 = DATA_new(); // Crear nuevo marciano
    DATA *data2 = DATA_new(); // Crear nuevo marciano
    DATA *data3 = DATA_new(); // Crear nuevo marciano

    al_start_timer(timer); // Start timer

    // Set shared DATA
    //al_lock_mutex(data->mutex);
    data->modifyWhich = 1; // segundos a ejecutar ' energia
    data->ready = false;
    // al_unlock_mutex(data->mutex);

    ALLEGRO_THREAD *thread_1 = al_create_thread(Func_Thread, data); // Initialize and start thread_1

    al_start_thread(thread_1);
    al_lock_mutex(data->mutex);
    while (!data->ready)
    {
        al_wait_cond(data->cond, data->mutex);
    }
    al_unlock_mutex(data->mutex);

    // Set shared DATA
    // al_lock_mutex(data1->mutex);
    data1->modifyWhich = 2;
    data1->ready = false;
    //  al_unlock_mutex(data1->mutex);

    // Initialize and start thread_2
    ALLEGRO_THREAD *thread_2 = al_create_thread(Func_Thread, data1);
    al_start_thread(thread_2);
    al_lock_mutex(data1->mutex);
    while (!data1->ready)
    {
        al_wait_cond(data1->cond, data1->mutex);
    }
    al_unlock_mutex(data1->mutex);

    // Set shared DATA
    // al_lock_mutex(data1->mutex);
    data2->modifyWhich = 3;
    data2->ready = false;
    //  al_unlock_mutex(data1->mutex);
    // Initialize and start thread_3
    ALLEGRO_THREAD *thread_3 = al_create_thread(Func_Thread, data2);
    al_start_thread(thread_3);
    al_lock_mutex(data2->mutex);
    while (!data2->ready)
    {
        al_wait_cond(data2->cond, data2->mutex);
    }
    al_unlock_mutex(data2->mutex);

    // Set shared DATA
    // al_lock_mutex(data1->mutex);
    data3->modifyWhich = 4;
    data3->ready = false;
    //  al_unlock_mutex(data1->mutex);
    // Initialize and start thread_4
    ALLEGRO_THREAD *thread_4 = al_create_thread(Func_Thread, data3);
    al_start_thread(thread_4);
    al_lock_mutex(data3->mutex);
    while (!data3->ready)
    {
        al_wait_cond(data3->cond, data3->mutex);
    }
    al_unlock_mutex(data3->mutex);

    // Event loop
    int code = CODE_CONTINUE;
    char snum[5];
    while (code == CODE_CONTINUE)
    {
        al_draw_bitmap(maze, POINT);
        if (RedrawIsReady() && al_is_event_queue_empty(event_queue))
        {
            RedrawDo(data, bouncer, maze);
            RedrawDo(data1, bouncer, maze);
            RedrawDo(data2, bouncer, maze);
            RedrawDo(data3, bouncer, maze);
        }
        al_draw_text(font, al_map_rgb(178, 178, 178), 460, 10, 0, "ENERGY: ");
        al_draw_text(font, al_map_rgb(178, 178, 178), 520, 10, 0, ">>>>>>>>>>");

        if (new_martian)
        {
            sprintf(snum, "%d", energy);
            al_draw_text(font, al_map_rgb(178, 120, 211), 10, 10, 0, "Energy: ");
            al_draw_text(font, al_map_rgb(255, 255, 255), 70, 10, 0, snum);

            sprintf(snum, "%d", period);
            al_draw_text(font, al_map_rgb(178, 120, 211), 108, 10, 0, "Period: ");
            al_draw_text(font, al_map_rgb(255, 255, 255), 170, 10, 0, snum);
        }

        al_flip_display();

        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);
        code = HandleEvent(ev);
    }

    // Clean up resources and exit with appropriate code
    al_destroy_thread(thread_1);
    al_destroy_thread(thread_2);
    al_destroy_bitmap(bouncer);
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    return code;
}

#define INCVAL (0.1f)
#define RESTVAL (0.01f)
#define UNKNOWN_ERROR (0)
static void *Func_Thread(ALLEGRO_THREAD *thr, void *arg)
{
    DATA *data = arg;
    al_lock_mutex(data->mutex);
    int mw = data->modifyWhich;
    data->ready = true;
    al_broadcast_cond(data->cond);
    al_unlock_mutex(data->mutex);
    while (!al_get_thread_should_stop(thr))
    {
        al_lock_mutex(data->mutex);
        if (mw == 1)
            data->posiX += INCVAL;
        else if (mw == 2)
            data->posiY += INCVAL;
        else if (mw == 3)
            data->posiX -= INCVAL;
        else if (mw == 4)
            data->posiY -= INCVAL;
        else
            assert(UNKNOWN_ERROR);
        al_unlock_mutex(data->mutex);
        al_rest(RESTVAL);
    }
    return NULL;
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Implementation of EVENT HANDLER Module
// ---------------------------------------------------------------------------
code HandleEvent(ALLEGRO_EVENT ev)
{
    switch (ev.type)
    {
    case ALLEGRO_EVENT_TIMER:
        // RedrawSetReady();
        REDRAW_IS_READY = true;
        break;
    case ALLEGRO_EVENT_KEY_DOWN:
        if (ev.keyboard.keycode == ALLEGRO_KEY_X)
            return EXIT_SUCCESS;
        if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
        {
            new_martian = 0;
            energy = 0;
            period = 0;
        }
        if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER)
        {
            if (new_martian == 1) // Deshabilita la creacion de marcianos
            {

                if (energy != 0 && period != 0)
                {
                    printf("Create Hilo con data...\n");

                    //int t = tamanoLista(node_martian);
                    // pthread_t threadID = t + 1;

                    // insertarEnLista(&node_martian, threadID, energy, period);
                    // martian *get_m = getMartianById(node_martian, threadID);
                    //printf("\nDATOS OBTENIDO DEL GET %ld - Energy: %f  -  Period: %f\n", get_m->id, get_m->energy, get_m->period); //Imprimimos el valor del nodo
                }
                else
                {
                    /* No hacer nada, */
                    printf("Sin data...\n");
                }

                new_martian = 0;
                energy = 0;
                period = 0;
            }
            else // Habilita la creacion de marcianos
            {
                new_martian = 1;
            }
        }
        if (new_martian == 1)
        {
            if (ev.keyboard.keycode == ALLEGRO_KEY_E)
            {
                energy++;
            }
            else if (ev.keyboard.keycode == ALLEGRO_KEY_P)
            {
                period++;
            }
        }
        break;
    case ALLEGRO_EVENT_DISPLAY_CLOSE:
        return EXIT_SUCCESS;
    default:
        break;
    }
    return CODE_CONTINUE;
}

void RedrawDo(DATA *data, ALLEGRO_BITMAP *bouncer, ALLEGRO_BITMAP *maze)
{
    //al_clear_to_color(al_map_rgb(0, 0, 0));

    al_lock_mutex(data->mutex);
    float X = data->posiX;
    float Y = data->posiY;
    al_unlock_mutex(data->mutex);
    al_draw_bitmap(bouncer, X, Y, 0);
    //al_flip_display();
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Implementation of DATA module
// ---------------------------------------------------------------------------
DATA *DATA_new(void)
{
    DATA *self = NULL;
    self = malloc(sizeof(*self));
    assert(self);
    *self = DATA_NEWINIT;
    self->mutex = al_create_mutex();
    self->cond = al_create_cond();
    assert(self->mutex);
    assert(self->cond);
    return self;
}

void DATA_delete(DATA *self)
{
    al_destroy_mutex(self->mutex);
    al_destroy_cond(self->cond);
    free(self);
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Implementation of REDRAW HANDLER Module
// ---------------------------------------------------------------------------
bool REDRAW_IS_READY = false;
//void RedrawSetReady(void) { REDRAW_IS_READY = true; }
//void RedrawClearReady(void) { REDRAW_IS_READY = false; }
bool RedrawIsReady(void)
{
    switch (REDRAW_IS_READY)
    {
    case true:
        //RedrawClearReady();
        REDRAW_IS_READY = false;
        return true;
    default:
        return false;
    }
}
// ---------------------------------------------------------------------------
