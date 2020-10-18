#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------
// Header of DATA module
// ---------------------------------------------------------------------------
typedef struct
{
    ALLEGRO_MUTEX *mutex;
    ALLEGRO_COND *cond;
    float posiX;
    float posiY;
    char modifyWhich; // which variable to modify, 'X' or 'Y'
    bool ready;
} DATA;

#define DATA_NEWINIT (\  
 (DATA) { \  
     .mutex = NULL,       \
  .cond = NULL, .posiX = 0, .posiY = 0, \  
     .modifyWhich = '\0', \
  .ready = false \  
         } \  
 )

DATA *DATA_new(void);
void DATA_delete(DATA *self);
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Header of MAIN module
// ---------------------------------------------------------------------------
const float FPS = 30.0;
const int SCREEN_W = 608;
const int SCREEN_H = 608;
const int BOUNCER_SIZE = 32;
#define BLACK (al_map_rgb(0, 0, 0))
#define MAGENTA (al_map_rgb(255, 0, 255))
static void *Func_Thread(ALLEGRO_THREAD *thr, void *arg);
// ---------------------------------------------------------------------------
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
code HandleEvent(ALLEGRO_EVENT ev);
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Header of REDRAW HANDLER Module
// ---------------------------------------------------------------------------
bool REDRAW_IS_READY;
void RedrawSetReady(void);
void RedrawClearReady(void);
bool RedrawIsReady(void);
void RedrawDo(); // to be defined in Implementation of EVENT HANDLER Module
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Implementation of MAIN Module
// ---------------------------------------------------------------------------
int main()
{
    // Initialize Allegro, mouse, timer, display and bitmap
    // XXX - skimping on Allegro error-checking
    al_init();
    /* al_install_mouse(); */
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

    DATA *data = DATA_new(); // Crear nuevo marciano
    DATA *data1 = DATA_new(); // Crear nuevo marciano


    // Start timer
    al_start_timer(timer);

    // Set shared DATA
    //al_lock_mutex(data->mutex);
    data->modifyWhich = 'X'; // segundos a ejecutar ' energia
    data->ready = false;
   // al_unlock_mutex(data->mutex);

    // Initialize and start thread_1
    ALLEGRO_THREAD *thread_1 = al_create_thread(Func_Thread, data);

    al_start_thread(thread_1);
    al_lock_mutex(data->mutex);
    while (!data->ready)
    {
        al_wait_cond(data->cond, data->mutex);
    }
    al_unlock_mutex(data->mutex);

    // Set shared DATA
   // al_lock_mutex(data1->mutex);
    data1->modifyWhich = 'Y';
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

    // Event loop
    int code = CODE_CONTINUE;
    while (code == CODE_CONTINUE)
    {
        al_draw_bitmap(maze, 0, 0, 0);
        if (RedrawIsReady() && al_is_event_queue_empty(event_queue))
            RedrawDo(data, bouncer, maze);
            RedrawDo(data1, bouncer, maze);
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
    char mw = data->modifyWhich;
    data->ready = true;
    al_broadcast_cond(data->cond);
    al_unlock_mutex(data->mutex);
    while (!al_get_thread_should_stop(thr))
    {
        al_lock_mutex(data->mutex);
        if (mw == 'X')
            data->posiX += INCVAL;
        else if (mw == 'Y')
            data->posiY += INCVAL;
        else
            assert(UNKNOWN_ERROR);
        al_unlock_mutex(data->mutex);
        al_rest(RESTVAL);
    }
    return NULL;
}
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Implementation of EVENT HANDLER Module
// ---------------------------------------------------------------------------
code HandleEvent(ALLEGRO_EVENT ev)
{
    switch (ev.type)
    {
    case ALLEGRO_EVENT_TIMER:
        RedrawSetReady();
        break;
    case ALLEGRO_EVENT_KEY_DOWN:
        printf("\nALLEGRO_EVENT_KEY_DOWN\n");

        if (ev.keyboard.keycode == ALLEGRO_KEY_X)
        {
            printf("\nXXXXX\n");
        }
        break;
    case ALLEGRO_EVENT_DISPLAY_CLOSE:
        printf("\nALLEGRO_EVENT_DISPLAY_CLOSE\n");
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

// ---------------------------------------------------------------------------
// Implementation of REDRAW HANDLER Module
// ---------------------------------------------------------------------------
bool REDRAW_IS_READY = false;
void RedrawSetReady(void) { REDRAW_IS_READY = true; }
void RedrawClearReady(void) { REDRAW_IS_READY = false; }
bool RedrawIsReady(void)
{
    switch (REDRAW_IS_READY)
    {
    case true:
        RedrawClearReady();
        return true;
    default:
        return false;
    }
}
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
