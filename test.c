#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <matrix.h>
#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>

// ---------------------------------------------------------------------------
// Header of DATA module
// ---------------------------------------------------------------------------
typedef struct mnodo
{
    ALLEGRO_MUTEX *mutex;
    ALLEGRO_COND *cond;
    int modifyWhich; // which variable to modify, 'X' or 'Y'
    float posiX;
    float posiY;
    float period; // Weight
    float energy; // Speed
    bool ready;
    struct mnodo *next_martian; //El puntero siguiente para recorrer la lista enlazada
} DATA;

#define DATA_NEWINIT (    \
    (DATA){               \
        .mutex = NULL,    \
        .cond = NULL,     \
        .posiX = 96,      \
        .posiY = 224,     \
        .period = 0,      \
        .energy = 0,      \
        .modifyWhich = 0, \
        .ready = false,   \
        .next_martian = NULL})

DATA *DATA_new(int energy_, int period_);
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
typedef DATA *tpuntero; //Puntero al tipo de dato DATA para no utilizar punteros de punteros

int new_martian = 0;
int energy = 0;
int period = 0;
code HandleEvent(ALLEGRO_EVENT ev, tpuntero node_martian);

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Header of REDRAW HANDLER Module
// ---------------------------------------------------------------------------
bool REDRAW_IS_READY;
bool RedrawIsReady(void);
void RedrawDo(); // to be defined in Implementation of EVENT HANDLER Module
// ---------------------------------------------------------------------------

void insertarEnLista(tpuntero *node_martian, DATA *data)
{
    tpuntero new_martian;                         //Creamos un nuevo nodo
    new_martian = malloc(sizeof(DATA));           //Utilizamos malloc para reservar memoria para ese nodo
    new_martian->cond = data->cond;               //Le asignamos el valor ingresado por pantalla a ese nodo
    new_martian->mutex = data->mutex;             //Le asignamos el valor ingresado por pantalla a ese nodo
    new_martian->period = data->period;           //Le asignamos el valor ingresado por pantalla a ese nodo
    new_martian->energy = data->energy;           //Le asignamos el valor ingresado por pantalla a ese nodo
    new_martian->modifyWhich = data->modifyWhich; //Le asignamos el valor ingresado por pantalla a ese nodo
    new_martian->posiX = data->posiX;             //Le asignamos el valor ingresado por pantalla a ese nodo
    new_martian->posiY = data->posiY;             //Le asignamos el valor ingresado por pantalla a ese nodo
    new_martian->ready = data->ready;             //Le asignamos el valor ingresado por pantalla a ese nodo
    new_martian->next_martian = *node_martian;    //Le asignamos al siguiente el valor de cabeza
    *node_martian = new_martian;                  //Cabeza pasa a ser el ultimo nodo agregado
}

void imprimirLista(tpuntero node)
{
    while (node != NULL)
    {                                                                                       //Mientras node no sea NULL
        printf("\n ImprimirLista Energy: %f  -  Period: %f\n", node->energy, node->period); //Imprimimos el valor del nodo
        node = node->next_martian;                                                          //Pasamos al siguiente nodo
    }
}

void renderLista(tpuntero node, ALLEGRO_BITMAP *bouncer, ALLEGRO_BITMAP *maze)
{
    while (node != NULL)
    {
        RedrawDo(node, bouncer, maze);
        //Mientras node no sea NULL
        printf("\n Pintando..");   //Imprimimos el valor del nodo
        node = node->next_martian; //Pasamos al siguiente nodo
    }
}

DATA *getMartianById(tpuntero node, float e)
{
    DATA *temp = malloc(sizeof(DATA));
    while (node != NULL)
    {
        if (node->energy == e)
        {
            temp = node;
            break;
        }
        node = node->next_martian; //Pasamos al siguiente nodo
    }
    return temp;
}

void borrarLista(tpuntero *node_martian)
{
    tpuntero node_temp; //Puntero auxiliar para eliminar correctamente la lista

    printf("Borrando lista ....\n");
    while (*node_martian != NULL)
    {                                                  //Mientras node_martian no sea NULL
        node_temp = *node_martian;                     //Actual toma el valor de node_martian
        *node_martian = (*node_martian)->next_martian; //Cabeza avanza 1 posicion en la lista
        free(node_temp);                               //Se libera la memoria de la posicion de Actual (el primer nodo), y node_martian queda apuntando al que ahora es el primero
    }
    printf("Lista Borrada!\n");
}

int tamanoLista(tpuntero node)
{
    int r = 0;
    while (node != NULL)
    { //Mientras node no sea NULL
        r++;
        node = node->next_martian; //Pasamos al siguiente nodo
    }
    return r;
}

// ---------------------------------------------------------------------------
// Implementation of MAIN Module
// ---------------------------------------------------------------------------
int main()
{
    tpuntero node_martian; //Indica la node_martian de la lista enlazada, si la perdemos no podremos acceder a la lista
    node_martian = NULL;   //Se inicializa la node_martian como NULL ya que no hay ningun nodo cargado en la lista

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
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_clear_to_color(BLACK);

    al_flip_display();

    /*     DATA *data = DATA_new(1, 3); // Crear nuevo marciano
    insertarEnLista(&node_martian, data);

    DATA *data1 = DATA_new(3, 5); // Crear nuevo marciano
    insertarEnLista(&node_martian, data1);

    DATA *data2 = DATA_new(6, 2); // Crear nuevo marciano
    insertarEnLista(&node_martian, data2);

    DATA *data3 = DATA_new(4, 4); // Crear nuevo marciano
    insertarEnLista(&node_martian, data3);

    DATA *get_m = getMartianById(node_martian, 6);
    printf("\nDATOS OBTENIDO DEL GET - Energy: %f  -  Period: %f\n", get_m->energy, get_m->period); //Imprimimos el valor del nodo

    printf("Tamano: %d\n", tamanoLista(node_martian)); */

    al_start_timer(timer); // Start timer

    DATA *dataX = DATA_new(1, 3);
    // Set shared DATA
    al_lock_mutex(dataX->mutex);
    dataX->modifyWhich = 1; // segundos a ejecutar ' energia
    dataX->ready = false;
    al_unlock_mutex(dataX->mutex);

    printf("%f\n", dataX->energy);

    ALLEGRO_THREAD *thread_11 = al_create_thread(Func_Thread, dataX); // Initialize and start thread_1

    al_start_thread(thread_11);
    al_lock_mutex(dataX->mutex);
    while (!dataX->ready)
    {
        al_wait_cond(dataX->cond, dataX->mutex);
    }
    al_unlock_mutex(dataX->mutex);

    // Set shared DATA
    // al_lock_mutex(data1->mutex);
    /*  data1->modifyWhich = 2;
    data1->ready = false; */
    //  al_unlock_mutex(data1->mutex);

    // Initialize and start thread_2
    /* ALLEGRO_THREAD *thread_2 = al_create_thread(Func_Thread, data1);
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
    */

    // Event loop
    int code = CODE_CONTINUE;
    char snum[5];
    while (code == CODE_CONTINUE)
    {
        al_draw_bitmap(maze, POINT);
        if (RedrawIsReady() && al_is_event_queue_empty(event_queue))
        {
            //RedrawDo(dataX, bouncer, maze);
            /*  RedrawDo(data, bouncer, maze);
            RedrawDo(data2, bouncer, maze);
            RedrawDo(data3, bouncer, maze); */
            //renderLista(node_martian, bouncer, maze);
            //printf("T: %d\n", tamanoLista(node_martian));
            if (tamanoLista(node_martian) >= 1)
            {
                printf("\treder...\n");
                DATA *get_m = getMartianById(node_martian, 1);
                RedrawDo(get_m, bouncer, maze);
            }

            // NO LO RENDERICE
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
        code = HandleEvent(ev, node_martian);
    }

    // Clean up resources and exit with appropriate code
    /*     
    al_destroy_thread(thread_1);
    al_destroy_thread(thread_2); 
    */

    // NO LO DESTRUI
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
code HandleEvent(ALLEGRO_EVENT ev, tpuntero node_martian)
{
    switch (ev.type)
    {
    case ALLEGRO_EVENT_TIMER:
        // RedrawSetReady();
        REDRAW_IS_READY = true;
        break;
    case ALLEGRO_EVENT_KEY_DOWN:
        if (ev.keyboard.keycode == ALLEGRO_KEY_X)
        {
            borrarLista(&node_martian);
            return EXIT_SUCCESS;
        }
        if (ev.keyboard.keycode == ALLEGRO_KEY_L)
        {
            printf("\n Lista cargada \n");
            imprimirLista(node_martian);
        }
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

                    DATA *data = DATA_new(1, 3); // Crear nuevo marciano
                    insertarEnLista(&node_martian, data);
                    al_lock_mutex(data->mutex);
                    data->modifyWhich = 2;
                    data->ready = false;
                    al_unlock_mutex(data->mutex);

                    printf("Energia nueva : %f\n", data->energy);
                    printf("Tamano: %d\n", tamanoLista(node_martian));


                    // crean hilo
                    ALLEGRO_THREAD *thread_1 = al_create_thread(Func_Thread, data); // Initialize and start thread_1

                    al_start_thread(thread_1);
                    al_lock_mutex(data->mutex);
                    while (!data->ready)
                    {
                        al_wait_cond(data->cond, data->mutex);
                    }
                    al_unlock_mutex(data->mutex);
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
        borrarLista(&node_martian);
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
DATA *DATA_new(int energy_, int period_)
{
    DATA *self = NULL;
    self = malloc(sizeof(*self));
    assert(self);
    *self = DATA_NEWINIT;
    self->mutex = al_create_mutex();
    self->cond = al_create_cond();
    self->energy = energy_;
    self->period = period_;
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
