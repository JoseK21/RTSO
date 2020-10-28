#include <stdio.h>
#include <string.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <matrix.h>
#include <structs.h>
#include <signal.h>
#include <pthread.h>
// To use time library of C
#include <time.h>

int __algorithm;
int __numberProcess;
int __PPID;
ALLEGRO_BITMAP *__image;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef martian *tpuntero; //Puntero al tipo de dato martian para no utilizar punteros de punteros

int parameterAlgorithm(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Warning!, only one input parameter must be provided\n");
        return 0;
    }

    if (!strcmp(argv[1], "RM"))
    {
        __algorithm = 0;
        return 1;
    }
    else if (!strcmp(argv[1], "EDF"))
    {
        __algorithm = 1;
        return 1;
    }
    else
    {
        printf("Algorithm error\n");
        return 0;
    }
}

/* martian *_martian = malloc(sizeof(martian)); // Reserva el espacio de memoria para el marciano */
/* Lista */
int moveMartian(void *param)
{
    //martian thread_martian = *((martian *)arg); // Casteo de void a martian
    martian *thread_martian = (martian *)param;

    printf("El hilo inicia....");
    printf("\n Energia : %f", thread_martian->energy);

    return 0;
}

void insertarEnLista(tpuntero *node_martian, pthread_t threadID_, int energy_, int period_)
{
    tpuntero new_martian;                      //Creamos un nuevo nodo
    new_martian = malloc(sizeof(martian));     //Utilizamos malloc para reservar memoria para ese nodo
    new_martian->x = 0;                        //Le asignamos el valor ingresado por pantalla a ese nodo
    new_martian->y = 224;                      //Le asignamos el valor ingresado por pantalla a ese nodo
                                               // new_martian->image = image_;               //Le asignamos el valor ingresado por pantalla a ese nodo
    new_martian->energy = energy_;             //Le asignamos el valor ingresado por pantalla a ese nodo
    new_martian->period = period_;             //Le asignamos el valor ingresado por pantalla a ese nodo
    new_martian->id = threadID_;               //Le asignamos el id del thread correspondiente
    new_martian->next_martian = *node_martian; //Le asignamos al siguiente el valor de cabeza
    *node_martian = new_martian;               //Cabeza pasa a ser el ultimo nodo agregado

    // return threadID;
}

void imprimirLista(tpuntero node)
{
    while (node != NULL)
    {                                                                                                         //Mientras node no sea NULL
        printf("\n ImprimirLista : %ld - Energy: %f  -  Period: %f\n", node->id, node->energy, node->period); //Imprimimos el valor del nodo
        node = node->next_martian;                                                                            //Pasamos al siguiente nodo
    }
}

martian *getMartianById(tpuntero node, pthread_t threadID)
{
    martian *temp = malloc(sizeof(martian));
    while (node != NULL)
    {
        if (node->id == threadID)
        {
            temp = node;
            break;
        }
        node = node->next_martian; //Pasamos al siguiente nodo
    }
    return temp;
}

void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;

    // Storing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}

void *move_martian_thread(void *data)
{
    //martian *struct_ptr = (martian *)data;
    //printf("index: %d  |  value: %d | energy: %f  |  period: %f \n", ((martian *)data)->x, struct_ptr->y, struct_ptr->energy, struct_ptr->period); //Now use 'struct_ptr->index', 'struct_ptr->value' as you wish

    printf("!!!!!energy: %f  \n", ((martian *)data)->energy); //Now use 'struct_ptr->index', 'struct_ptr->value' as you wish

    /* for (size_t i = 0; i < 100; i++)
    {
        printf("!!!!!x: %d  \n", ((martian *)data)->x); //Now use 'struct_ptr->index', 'struct_ptr->value' as you wish
        ((martian *)data)->x = ((martian *)data)->x + 1;
    } */

    int i;
    for (i = 0; i < 10; i++)
    {
        // delay of one second
        delay(1);
        printf("%d seconds have passed\n", i + 1);
    }

    /* for (size_t i = 0; i < 100; i++)
    { */

    /*     pthread_mutex_lock(&mutex);
    al_set_target_bitmap(NULL);
    al_draw_bitmap(__image, 200, 224, 0);
    pthread_mutex_unlock(&mutex); */
}

void move(tpuntero node)
{

    while (node != NULL)
    { //Mientras tem no sea NULL
        al_draw_bitmap(__image, node->x + 100, 224, 0);

        printf("\n WHILE : %ld - Energy: %f  -  Period: %f\n", node->id, node->energy, node->period); //Imprimimos el valor del nodo
        node = node->next_martian;                                                                    //Pasamos al siguiente nodo
    }
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

void borrarLista(tpuntero *node_martian)
{
    tpuntero node_temp; //Puntero auxiliar para eliminar correctamente la lista

    while (*node_martian != NULL)
    {                                                  //Mientras node_martian no sea NULL
        node_temp = *node_martian;                     //Actual toma el valor de node_martian
        *node_martian = (*node_martian)->next_martian; //Cabeza avanza 1 posicion en la lista
        free(node_temp);                               //Se libera la memoria de la posicion de Actual (el primer nodo), y node_martian queda apuntando al que ahora es el primero
    }
}

/* -------------- MAIN ------------- */
int main(int argc, char *argv[])
{

    tpuntero node_martian; //Indica la node_martian de la lista enlazada, si la perdemos no podremos acceder a la lista
    node_martian = NULL;   //Se inicializa la node_martian como NULL ya que no hay ningun nodo cargado en la lista

    if (!parameterAlgorithm(argc, argv))
    {
        return 0;
    }

    if (!al_init())
    {
        printf("couldn't initialize allegro\n");
        return 1;
    }

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

    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 30.0);
    if (!timer)
    {
        printf("couldn't initialize timer\n");
        return 1;
    }

    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    if (!queue)
    {
        printf("couldn't initialize queue\n");
        return 1;
    }

    ALLEGRO_DISPLAY *disp = al_create_display(608, 608);
    if (!disp)
    {
        printf("couldn't initialize display\n");
        return 1;
    }

    ALLEGRO_FONT *font = al_create_builtin_font();
    if (!font)
    {
        printf("couldn't initialize font\n");
        return 1;
    }

    __image = al_load_bitmap("../src/alien.png");
    ALLEGRO_BITMAP *maze = al_load_bitmap("../src/maze.png");

    if (!__image || !maze)
    {
        puts("couldn't load alien\n");
        return 1;
    }

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    bool done = false;
    bool redraw = true;
    ALLEGRO_EVENT event;

    int x_alien = 0;

    int new_martian = 0;
    int energy = 0;
    int period = 0;
    char snum[5];

    /*     int c = al_show_native_message_box(
        disp,
        "Fin de Partida",
        "Quieres reiniciar",
        "Si as'i lo dIf you click yes then you are confirming that \"Yes\""
        "is your response to the query which you have"
        "generated by the action you took to open this"
        "message box.",
        NULL,
        ALLEGRO_MESSAGEBOX_OK_CANCEL);
    printf("%d\n", c); */

    al_start_timer(timer);
    while (1)
    {
        al_wait_for_event(queue, &event);

        switch (event.type)
        {
        case ALLEGRO_EVENT_TIMER:

            // game logic goes here.

            /* if (0 != pthread_create(&threadID, NULL, move_martian_thread, &get_m))
            {
                printf("ERROR CREATE THREAD\n");
                return -1;
            }
            pthread_join(threadID, NULL); */

            /* move(node_martian); */
            redraw = true;
            break;

        case ALLEGRO_EVENT_KEY_DOWN:
            if (event.keyboard.keycode == ALLEGRO_KEY_X)
            {
                done = true;
            }
            if (event.keyboard.keycode == ALLEGRO_KEY_L)
            {
                printf("\nSe imprime la lista cargada: ");
                imprimirLista(node_martian);
            }
            if (event.keyboard.keycode == ALLEGRO_KEY_J)
            {
                printf("\nLeer por ID ");
                martian *get = getMartianById(node_martian, 1);
                printf("\n]]]]]] %ld - Energy: %f  -  Period: %f - x: %d - y: %d\n", get->id, get->energy, get->period, get->x, get->y); //Imprimimos el valor del nodo
            }
            if (event.keyboard.keycode == ALLEGRO_KEY_ENTER)
            {
                if (new_martian == 1) // Deshabilita la creacion de marcianos
                {

                    if (energy != 0 && period != 0)
                    {
                        printf("Create Hilo con data...\n");

                        int t = tamanoLista(node_martian);
                        pthread_t threadID = t + 1;

                        insertarEnLista(&node_martian, threadID, energy, period);
                        martian *get_m = getMartianById(node_martian, threadID);
                        printf("\nDATOS OBTENIDO DEL GET %ld - Energy: %f  -  Period: %f\n", get_m->id, get_m->energy, get_m->period); //Imprimimos el valor del nodo

                        pthread_create(&threadID, NULL, move_martian_thread, (void *)get_m);
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
                if (event.keyboard.keycode == ALLEGRO_KEY_E)
                {
                    energy++;
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_P)
                {
                    period++;
                }
            }

            break;
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            done = true;
            break;
        }

        if (done)
        {
            printf("\nSe borra la lista cargada\n");
            borrarLista(&node_martian);
            break;
        }

        if (redraw && al_is_event_queue_empty(queue))
        {

            al_clear_to_color(al_map_rgb(255, 255, 255));

            //pthread_mutex_lock(&mutex);

            al_draw_bitmap(maze, 0, 0, 0);

            // al_draw_bitmap(__image, 100, 224, 0);
            /*            
            al_draw_bitmap(alien, x_alien + 150, 224, 0);
            al_draw_bitmap(alien, x_alien + 200, 224, 0);
            al_draw_bitmap(alien, x_alien + 250, 224, 0);
             */

            while (node_martian != NULL)
            {
                al_draw_bitmap(__image, node_martian->x, node_martian->y, 0); //Mientras node no sea NULL
                sleep(3);
                node_martian = node_martian->next_martian; //Pasamos al siguiente nodo
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
            //pthread_mutex_unlock(&mutex);
            redraw = false;
        }
    }

    al_destroy_bitmap(__image);
    al_destroy_bitmap(maze);

    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}